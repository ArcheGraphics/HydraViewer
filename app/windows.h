//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QMainWindow>

namespace vox {
class Windows : public QMainWindow {
public:
    Windows(int width, int height);

private:
    bool viewer_enabled{true};

    void initMenuBar();
};
}// namespace vox