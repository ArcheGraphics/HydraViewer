//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QtNodes/GraphicsView>
#include "editor/viewport/viewport.h"
#include "editor/model/data_model.h"

namespace vox {
class Windows : public QMainWindow {
public:
    Windows(int width, int height);

    void run();

private:
    DataModel model;

    QDockWidget *stage_tree_dock_widget{};
    QLabel *l_status{};

    vox::Viewport *viewport{};

    void _loadStylesheet();
    void _initUI();
    void _initMenuBar();
    QtNodes::GraphicsView *_create_node_graph();
};
}// namespace vox