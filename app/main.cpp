//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <QApplication>
#include "editor/windows.h"

int main(int argc, char *argv[]) {
    QApplication app{argc, argv};
    {
        vox::Windows window(1280u, 720u);
        window.run();
    }
    QApplication::quit();
}
