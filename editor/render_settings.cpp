//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include <QScrollArea>
#include <QLabel>
#include <fmt/format.h>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include "render_settings.h"

namespace vox {
RenderSettingsWidget::RenderSettingsWidget(Viewport *viewport)
    : _viewport{viewport} {
    auto layout_root = new QHBoxLayout(this);
    auto scroll_area = new QScrollArea();
    layout_root->addWidget(scroll_area);
    auto scroll_area_main_widget = new QWidget();
    scroll_area->setWidget(scroll_area_main_widget);
    auto scroll_area_main_layout = new QVBoxLayout();
    scroll_area_main_widget->setLayout(scroll_area_main_layout);
    grid_layout = new QGridLayout();
    scroll_area_main_layout->addLayout(grid_layout);

    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setWidgetResizable(true);

    scroll_area_main_layout->setAlignment(Qt::AlignTop);

    // TODO enable both columns to be resized simultaneously
    grid_layout->setSpacing(6);

    setAttribute(Qt::WA_StyledBackground, true);
}

QSize RenderSettingsWidget::sizeHint() const {
    return {250, 250};
}

void RenderSettingsWidget::onRendererChanged() {
    _clear_widgets();
    _populate_widgets();
}

void RenderSettingsWidget::_clear_widgets() {
    for (int i = 0; i < grid_layout->count(); ++i) {
        auto item_to_remove = grid_layout->itemAt(i);
        if (item_to_remove) {
            auto widget_to_remove = item_to_remove->widget();
            if (widget_to_remove) {
                grid_layout->removeWidget(widget_to_remove);
                widget_to_remove->deleteLater();
            }
        }
    }
}

void RenderSettingsWidget::_populate_widgets() {
    auto settings = _viewport->rendererSettingLists();

    for (int row = 0; row < settings.size(); ++row) {
        auto &setting = settings[row];
        auto label_widget = new QLabel(fmt::format("{}: ", setting.key.data()).c_str());
        label_widget->setMaximumWidth(150);
        label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
        grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

        auto value_widget = _create_value_widget(setting);
        if (value_widget) {
            value_widget->setMaximumWidth(80);
            grid_layout->addWidget(value_widget, row, 1, Qt::AlignRight);
        }
    }
}

QWidget *RenderSettingsWidget::_create_value_widget(const pxr::UsdImagingGLRendererSetting &renderer_setting) {
    QWidget *widget{};
    auto value = _viewport->rendererSetting(renderer_setting.key);
    if (renderer_setting.type == pxr::UsdImagingGLRendererSetting::TYPE_FLAG) {
        auto value_widget = new QCheckBox();
        value_widget->setChecked(value.Get<bool>());
        connect(value_widget, &QCheckBox::toggled, this, [=](bool v) {
            _viewport->setRendererSetting(renderer_setting.key, pxr::VtValue(v));
        });
        widget = value_widget;
    } else if (renderer_setting.type == pxr::UsdImagingGLRendererSetting::TYPE_INT) {
        auto value_widget = new QSpinBox();
        value_widget->setMinimum(std::numeric_limits<int>::min());
        value_widget->setMaximum(std::numeric_limits<int>::max());
        value_widget->setValue(_viewport->rendererSetting(renderer_setting.key).Get<int>());
        connect(value_widget, &QSpinBox::valueChanged, this, [=](int v) {
            _viewport->setRendererSetting(renderer_setting.key, pxr::VtValue(v));
        });
        widget = value_widget;
    } else if (renderer_setting.type == pxr::UsdImagingGLRendererSetting::TYPE_FLOAT) {
        auto value_widget = new QDoubleSpinBox();
        value_widget->setDecimals(4);
        value_widget->setMinimum(std::numeric_limits<float>::min());
        value_widget->setMaximum(std::numeric_limits<float>::max());
        value_widget->setValue(_viewport->rendererSetting(renderer_setting.key).Get<float>());
        connect(value_widget, &QDoubleSpinBox::valueChanged, this, [=](float v) {
            _viewport->setRendererSetting(renderer_setting.key, pxr::VtValue(v));
        });
        widget = value_widget;
    } else if (renderer_setting.type == pxr::UsdImagingGLRendererSetting::TYPE_STRING) {
        auto value_widget = new QLineEdit();
        value_widget->setText(_viewport->rendererSetting(renderer_setting.key).Get<std::string>().c_str());
        connect(value_widget, &QLineEdit::textChanged, this, [=](const QString &v) {
            _viewport->setRendererSetting(renderer_setting.key, pxr::VtValue(v.toStdString()));
        });
    }
    return widget;
}

}// namespace vox