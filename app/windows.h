//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QMainWindow>
#include <QLabel>
#include "viewport/viewport.h"

namespace vox {
class Windows : public QMainWindow {
public:
    Windows(int width, int height);

    void run();

    void resizeEvent(QResizeEvent *event) override;

private:
    bool viewer_enabled{true};
    QDockWidget* stage_tree_dock_widget{};
    QLabel* l_status{};

    vox::Viewport viewport;

    void initUI();
    void initMenuBar();
};
}// namespace vox