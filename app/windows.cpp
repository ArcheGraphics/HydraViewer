//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "windows.h"
#include "editor/framerate.h"
#include <QMenuBar>
#include <QActionGroup>
#include <fmt/format.h>
#include <pxr/usd/usd/prim.h>
#include <QStatusBar>
#include <QDockWidget>
#include <QApplication>

namespace vox {
Windows::Windows(int width, int height)
    : QMainWindow(),
      viewport{this} {
    resize(width, height);
    setWindowTitle("Editor");
    setAutoFillBackground(true);

    viewport.move(contentsRect().topLeft());
    viewport.resize(contentsRect().size());
    viewport.recreateSwapChain(contentsRect().size());
    viewport.setupScene(pxr::UsdStage::Open("assets/Kitchen_set/Kitchen_set.usd"));

    // initUI();
    initMenuBar();
}

void Windows::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    viewport.resizeEvent(event);
}

void Windows::run() {
    show();
    vox::Framerate framerate;
    while (isVisible()) {
        viewport.draw();
        QApplication::processEvents();

        framerate.record();
        auto title = fmt::format("Display - {:.2f} fps", framerate.report());
        setWindowTitle(title.c_str());
    }
}

void Windows::initUI() {
    resize(1600, 900);
    setObjectName("MainWindow");
    l_status = new QLabel();
    l_status->setStyleSheet("border: 0px;");
    statusBar()->setHidden(false);
    statusBar()->addWidget(l_status);

    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    setDockNestingEnabled(true);

    // Stage Tree
    {
        // stage_tree_widget = get_stage_tree_widget();
        stage_tree_dock_widget = new QDockWidget();
        stage_tree_dock_widget->setWindowTitle("Scenegraph");
        // stage_tree_dock_widget->setWidget(stage_tree_widget);
        stage_tree_dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, stage_tree_dock_widget);
    }

    if (viewer_enabled) {
        // data_model = StageView.DefaultDataModel();
        // stage_view = StageView(dataModel = data_model);

        // region Render Settings
        // render_settings_widget = get_render_settings_widget(stage_view);
        auto render_settings_dock_widget = new QDockWidget();
        render_settings_dock_widget->setWindowTitle("Render Settings");
        // render_settings_dock_widget->setWidget(render_settings_widget);
        render_settings_dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
        splitDockWidget(stage_tree_dock_widget, render_settings_dock_widget, Qt::Vertical);

        // region Stage View
        // stage_view_widget = get_stage_view_widget(data_model, stage_view);
        // stage_view_widget.fileDropped.connect(on_view_file_dropped);
        auto stage_view_dock_widget = new QDockWidget();
        stage_view_dock_widget->setWindowTitle("Viewport");
        // stage_view_dock_widget->setWidget(stage_view_widget);
        stage_view_dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, stage_view_dock_widget);

        // stage_view_widget.rendererChanged.connect(render_settings_widget.on_renderer_changed);
    }

    // region Properties
    {
        // properties = new PropertiesBinWidget(root_node_graph = qx_node_graph);
        auto properties_dock_widget = new QDockWidget();
        properties_dock_widget->setWindowTitle("Properties");
        // properties_dock_widget->setWidget(properties);
        properties_dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, properties_dock_widget);
    }

    // region events
    {
        // qx_node_graph.node_graph_changed.connect(on_node_graph_changed);
        // qx_node_graph.mx_data_updated.connect(stage_ctrl.refresh_mx_file);

        // qx_node_graph.mx_parameter_changed.connect(stage_ctrl.update_parameter);
        // qx_node_graph.mx_file_loaded.connect(on_mx_file_loaded);

        if (viewer_enabled) {
            // stage_ctrl.signal_stage_changed.connect(stage_view_widget.set_stage);
            // stage_ctrl.signal_stage_updated.connect(stage_view_widget.view.updateGL);
        }
        // stage_ctrl.signal_stage_changed.connect(stage_tree_widget.set_stage);
        // stage_ctrl.signal_stage_updated.connect(stage_tree_widget.refresh_tree);
    }

    // setCentralWidget(qx_node_graph_widget);
    // qx_node_graph_widget.setFocus();
}

void Windows::initMenuBar() {
    auto file_menu = menuBar()->addMenu("&File");
    auto options_menu = menuBar()->addMenu("&Options");
    auto view_menu = menuBar()->addMenu("&View");
    auto help_menu = menuBar()->addMenu("&Help");

    // file
    {
        auto load_mx_file = new QAction("Load MaterialX...", this);
        //        load_mx_file->triggered.connect(load_mx_file_triggered);
        file_menu->addAction(load_mx_file);

        auto save_mx_file = new QAction("Save MaterialX...", this);
        //        save_mx_file.triggered.connect(save_mx_file_triggered);
        file_menu->addAction(save_mx_file);

        file_menu->addSeparator();

        auto load_geo = new QAction("Load Geometry...", this);
        //        load_geo.triggered.connect(load_geometry_triggered);
        file_menu->addAction(load_geo);

        load_geo = new QAction("Load HDRI...", this);
        //        load_geo.triggered.connect(load_hdri_triggered);
        file_menu->addAction(load_geo);

        file_menu->addSeparator();

        auto show_mx_text = new QAction("Show MaterialX as text...", this);
        //        show_mx_text.triggered.connect(show_mx_text_triggered);
        file_menu->addAction(show_mx_text);

        file_menu->addSeparator();

        auto show_mx_view = new QAction("Open in MaterialX View...", this);
        //        show_mx_view.triggered.connect(show_mx_view_triggered);
        file_menu->addAction(show_mx_view);

        auto show_mx_editor = new QAction("Open in MaterialX Graph Editor...", this);
        // show_mx_editor.triggered.connect(show_mx_editor_triggered);
        file_menu->addAction(show_mx_editor);

        auto show_usdview = new QAction("Open in Usdview...", this);
        // show_usdview.triggered.connect(show_usdview_triggered);
        file_menu->addAction(show_usdview);
    }

    // options
    {
        auto act_update_ng = new QAction("Auto update on nodegraph change", this);
        act_update_ng->setCheckable(true);
        act_update_ng->setChecked(true);
        // act_update_ng.toggled.connect(lambda state : setattr(qx_node_graph, "auto_update_ng", state));
        // qx_node_graph.auto_update_ng = act_update_ng.isChecked();
        options_menu->addAction(act_update_ng);

        auto act_update_prop = new QAction("Auto update on property change", this);
        act_update_prop->setCheckable(true);
        act_update_prop->setChecked(true);
        // act_update_prop.toggled.connect(lambda state : setattr(qx_node_graph, "auto_update_prop", state));
        options_menu->addAction(act_update_prop);

        auto act_apply_mat = new QAction("Auto apply material to all prims", this);
        act_apply_mat->setCheckable(true);
        act_apply_mat->setChecked(true);
        options_menu->addAction(act_apply_mat);

        auto act_ng_abstraction = new QAction("Auto create Nodegraph around shader inputs", this);
        act_ng_abstraction->setCheckable(true);
        act_ng_abstraction->setChecked(true);
        options_menu->addAction(act_ng_abstraction);

        auto act_validate = new QAction("Validate MaterialX document...", this);
        // act_validate.triggered.connect(validate);
        options_menu->addAction(act_validate);

        auto act_reload_defs = new QAction("Reload Node Definitions", this);
        // act_reload_defs.triggered.connect(reload_defs);
        options_menu->addAction(act_reload_defs);
    }

    {
        if (viewer_enabled) {
            auto menu_set_current_renderer = new QMenu("&Set Renderer", this);
            // auto grp_set_current_renderer = new QActionGroup(this, true);
            // grp_set_current_renderer.triggered.connect(
            //     lambda action : stage_view_widget.set_current_renderer_by_name(action.text()));
            // menu_set_current_renderer->aboutToShow.connect(on_set_renderer_menu_showing);
            view_menu->addMenu(menu_set_current_renderer);

            auto act_hdri = new QAction("Enable HDRI", this);
            act_hdri->setCheckable(true);
            act_hdri->setChecked(true);
            // act_hdri.toggled.connect(stage_view_widget.set_hdri_enabled);
            // act_hdri.toggled.connect(lambda x : stage_tree_widget.refresh_tree());
            view_menu->addAction(act_hdri);
            view_menu->addSeparator();
        }
        // view_menu.aboutToShow.connect(on_view_menu_showing);
        auto act_prop = new QAction("Properties", this);
        act_prop->setCheckable(true);
        // act_prop.toggled.connect(on_properties_toggled);
        view_menu->addAction(act_prop);

        auto act_render_settings = new QAction("Render Settings", this);
        act_render_settings->setCheckable(true);
        // act_render_settings.toggled.connect(on_render_settings_toggled);
        view_menu->addAction(act_render_settings);

        auto act_scenegraph = new QAction("Scenegraph", this);
        act_scenegraph->setCheckable(true);
        // act_scenegraph.toggled.connect(on_scenegraph_toggled);
        view_menu->addAction(act_scenegraph);

        auto act_viewport = new QAction("Viewport", this);
        act_viewport->setCheckable(true);
        // act_viewport.toggled.connect(on_viewport_toggled);
        if (viewer_enabled) {
            view_menu->addAction(act_viewport);
        }
    }

    {
        auto mx_homepage = new QAction("MaterialX Homepage...", this);
        // mx_homepage.triggered.connect(open_mx_homepage_triggered);
        help_menu->addAction(mx_homepage);

        auto mx_spec = new QAction("Node Definitions...", this);
        // mx_spec.triggered.connect(open_mx_spec_triggered);
        help_menu->addAction(mx_spec);

        auto homepage_action = new QAction("HydraViewer Homepage...", this);
        auto homepage_url = "https://github.com/prismpipeline/QuiltiX/";
        // homepage_action.triggered.connect(lambda : webbrowser.open(homepage_url));
        help_menu->addAction(homepage_action);

        auto issues_action = new QAction("HydraViewer issues...", this);
        auto issues_url = "https://github.com/prismpipeline/QuiltiX/issues";
        // issues_action.triggered.connect(lambda : webbrowser.open(issues_url));
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
        // about_action.triggered.connect(show_about_triggered);
        help_menu->addAction(about_action);
    }
}
}// namespace vox