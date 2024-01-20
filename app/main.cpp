//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <QApplication>
#include <QPushButton>
#include <QtGui/qevent.h>
#include <fmt/format.h>
#include "viewport/viewport.h"
#include "viewport/camera.h"
#include "viewport/framerate.h"

#include "windows.h"

static constexpr auto win_width = 1280u;
static constexpr auto win_height = 720u;

class Canvas : public QWidget {

public:
    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

public:
    explicit Canvas(QWidget *parent) noexcept
        : QWidget{parent},
          viewport{winId(), win_width, win_height} {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_DontCreateNativeAncestors);
        setAutoFillBackground(true);

        auto stage = pxr::UsdStage::Open("assets/Kitchen_set/Kitchen_set.usd");
        viewport.setupScene(stage);
    }

    void wheelEvent(QWheelEvent *event) override {
        auto delta = event->pixelDelta();
        viewport.viewCamera()->panByDelta({(float)delta.x(), (float)delta.y()});
    }

    void draw() {
        viewport.draw();
    }

    vox::Viewport viewport;
};

int main(int argc, char *argv[]) {
    QApplication app{argc, argv};
    vox::Windows window(win_width, win_height);

    Canvas canvas{&window};
    canvas.setFixedSize(window.contentsRect().size());
    canvas.move(window.contentsRect().topLeft());

    window.show();
    vox::Framerate framerate;
    while (window.isVisible()) {
        canvas.draw();
        QApplication::processEvents();

        framerate.record();
        auto title = fmt::format("Display - {:.2f} fps", framerate.report());
        window.setWindowTitle(title.c_str());
    }

    QApplication::quit();
}
