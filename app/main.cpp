//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <QApplication>
#include <QtGui/qevent.h>
#include <fmt/format.h>
#include "viewport/viewport.h"
#include "viewport/camera.h"
#include "viewport/framerate.h"

#include "windows.h"

int main(int argc, char *argv[]) {
    QApplication app{argc, argv};
    vox::Windows window(1280u, 720u);

    vox::Viewport viewport{&window};
    viewport.setFixedSize(window.contentsRect().size());
    viewport.move(window.contentsRect().topLeft());

    viewport.setupScene(pxr::UsdStage::Open("assets/Kitchen_set/Kitchen_set.usd"));

    window.show();
    vox::Framerate framerate;
    while (window.isVisible()) {
        viewport.draw();
        QApplication::processEvents();

        framerate.record();
        auto title = fmt::format("Display - {:.2f} fps", framerate.report());
        window.setWindowTitle(title.c_str());
    }

    QApplication::quit();
}
