//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include "viewport/viewport.h"

namespace vox {
class RenderSettingsWidget : public QWidget {
public:
    explicit RenderSettingsWidget(Viewport *viewport);

    [[nodiscard]] QSize sizeHint() const override;

    void onRendererChanged();

    void _clear_widgets();

    void _populate_widgets();

    QWidget *_create_value_widget(const pxr::UsdImagingGLRendererSetting &renderer_setting);

private:
    Viewport *_viewport;
    QGridLayout *grid_layout;
};
}// namespace vox