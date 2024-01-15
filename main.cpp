//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <QApplication>
#include <QPushButton>
#include <QMainWindow>

class Canvas : public QWidget {

public:
    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

public:
    explicit Canvas(QWidget *parent) noexcept : QWidget{parent} {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_DontCreateNativeAncestors);
        setAutoFillBackground(true);
    }
};

int main(int argc, char *argv[]) {
    static constexpr auto width = 1280u;
    static constexpr auto height = 720u;

    QApplication app{argc, argv};
    QMainWindow window;
    window.setFixedSize(width, height);
    window.setWindowTitle("Display");
    window.setAutoFillBackground(true);

    Canvas canvas{&window};
    canvas.setFixedSize(window.contentsRect().size());
    canvas.move(window.contentsRect().topLeft());

    QWidget overlay{&window};
    overlay.setFixedSize(window.contentsRect().size() / 2);
    overlay.move(window.contentsRect().center() - overlay.rect().center());
    overlay.setAutoFillBackground(true);

    QPushButton button{"Quit", &overlay};
    button.move(overlay.contentsRect().center() - button.rect().center());
    QObject::connect(&button, &QPushButton::clicked, [&] {
        window.setVisible(false);
    });

    window.show();
    while (window.isVisible()) {
        QApplication::processEvents();
    }

    QApplication::quit();
}
