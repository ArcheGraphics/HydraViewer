//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "windows.h"
#include "editor/framerate.h"
#include "panels/stage_tree.h"
#include "panels/render_settings.h"
#include "panels/view_settings_view.h"
#include "editor/node/graph_model.h"
#include <QMenuBar>
#include <QActionGroup>
#include <fmt/format.h>
#include <pxr/usd/usd/prim.h>
#include <QStatusBar>
#include <QDockWidget>
#include <QApplication>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/StyleCollection>
#include <fstream>
#include <regex>
#include <QMessageBox>
#include <QDesktopServices>

namespace vox {
namespace {
std::string read_text_file(const std::string &filename) {
    std::ifstream file;

    file.open(filename, std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    return std::string{(std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>())};
}
}// namespace

Windows::Windows(int width, int height)
    : QMainWindow() {
    resize(width, height);
    setWindowTitle("Editor");
    setAutoFillBackground(true);

    _initUI();
    _initMenuBar();
    _loadStylesheet();

    model.setStage(pxr::UsdStage::Open(fmt::format("{}/{}", PROJECT_PATH, "assets/Kitchen_set/Kitchen_set.usd")));
}

void Windows::run() {
    show();
    while (isVisible()) {
        viewport->draw();
        QApplication::processEvents();
    }
}

void Windows::_initUI() {
    setObjectName("MainWindow");
    l_status = new QLabel();
    l_status->setStyleSheet("border: 0px;");
    statusBar()->setHidden(false);
    statusBar()->addWidget(l_status);

    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
    setDockNestingEnabled(true);

    viewport = new Viewport(this, model);
    setCentralWidget(viewport);
    viewport->setFocus();

    // region Tree
    {
        auto stage_tree_widget = new StageTreeWidget(model, this);
        stage_tree_dock_widget = new QDockWidget();
        stage_tree_dock_widget->setWindowTitle("Scenegraph");
        stage_tree_dock_widget->setWidget(stage_tree_widget);
        stage_tree_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea);
        stage_tree_dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        addDockWidget(Qt::LeftDockWidgetArea, stage_tree_dock_widget);
    }

    // region Properties
    {
        auto render_settings_widget = new ViewSettingsWidget(model);
        auto properties_dock_widget = new QDockWidget();
        properties_dock_widget->setWindowTitle("Properties");
        properties_dock_widget->setWidget(render_settings_widget);
        properties_dock_widget->setAllowedAreas(Qt::RightDockWidgetArea);
        properties_dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        addDockWidget(Qt::RightDockWidgetArea, properties_dock_widget);
        properties_dock_widget->setMaximumWidth(300);
    }

    // region node
    {
        auto node_view = _create_node_graph();
        auto node_graph_dock_widget = new QDockWidget();
        node_graph_dock_widget->setWindowTitle("Nodegraph");
        node_graph_dock_widget->setWidget(node_view);
        node_graph_dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea);
        node_graph_dock_widget->setFeatures(QDockWidget::DockWidgetFloatable);
        addDockWidget(Qt::BottomDockWidgetArea, node_graph_dock_widget);
    }
}

void Windows::_initMenuBar() {
    auto help_menu = menuBar()->addMenu("&Help");

    {
        auto homepage_action = new QAction("HydraViewer Homepage...", this);
        connect(homepage_action, &QAction::triggered, this, []() {
            auto homepage_url = "https://github.com/ArcheGraphics/HydraViewer/";
            QDesktopServices::openUrl(QUrl(homepage_url));
        });
        help_menu->addAction(homepage_action);

        auto issues_action = new QAction("HydraViewer issues...", this);
        connect(issues_action, &QAction::triggered, this, []() {
            auto issues_url = "https://github.com/ArcheGraphics/HydraViewer/issues";
            QDesktopServices::openUrl(QUrl(issues_url));
        });
        help_menu->addAction(issues_action);

        auto versions_to_be_displayed = {
            fmt::format("USD version: {}", PXR_VERSION)};
        auto versions_submenu = new QMenu("Loaded Versions", this);
        for (const auto &version : versions_to_be_displayed) {
            auto version_action = new QAction(QString(version.c_str()), this);
            version_action->setEnabled(false);
            versions_submenu->addAction(version_action);
        }
        help_menu->addMenu(versions_submenu);

        auto about_action = new QAction("About HydraViewer", this);
        connect(about_action, &QAction::triggered, this, &Windows::_showAboutTriggered);
        help_menu->addAction(about_action);
    }
}

void Windows::_showAboutTriggered() {
    QString info = fmt::format("<p>Version: {}<p><p>&nbsp;</p>", _version).c_str();
    info += "<p><a href='https://github.com/ArcheGraphics/HydraViewer' style='color:#ffffff;'>Homepage...</a></p>";
    info += "<p><a href='https://github.com/ArcheGraphics/HydraViewer/blob/main/LICENSE' style='color:#ffffff;'>License...</a></p>";
    info += "<p>© Feng Yang</p>";
    QMessageBox::about(this, "About HydraViewer", info);
}

void Windows::_loadStylesheet() {
    auto qss = read_text_file(fmt::format("{}/{}", PROJECT_PATH, "editor/style.qss"));
    std::regex regexPattern("resource_path");
    qss = std::regex_replace(qss, regexPattern, fmt::format("{}/{}", PROJECT_PATH, "editor"));
    setStyleSheet(QString(qss.c_str()));
}

QtNodes::GraphicsView *Windows::_create_node_graph() {
    auto scene = new QtNodes::BasicGraphicsScene(model.graphModel());
    auto view = new QtNodes::GraphicsView(scene);
    view->setSceneRect(QRect(0, 0, 20, 200));

    // Setup context menu for creating new nodes.
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto createNodeAction = new QAction(QStringLiteral("Create Node"), view);
    QObject::connect(createNodeAction, &QAction::triggered, [=]() {
        // Mouse position in scene coordinates.
        QPointF posView = view->mapToScene(view->mapFromGlobal(QCursor::pos()));

        NodeId const newId = model.graphModel().addNode();
        model.graphModel().setNodeData(newId, NodeRole::Position, posView);
    });
    view->insertAction(view->actions().front(), createNodeAction);
    return view;
}

}// namespace vox