//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "view_settings_view.h"
#include <QScrollArea>
#include <QLabel>
#include <fmt/format.h>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>

namespace vox {
ViewSettingsView::ViewSettingsView(DataModel &model) : _model{model} {
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
    _link_view_model();
}

QSize ViewSettingsView::sizeHint() const {
    return {250, 250};
}

void ViewSettingsView::_link_view_model() {
    {
        auto row = 0;
        auto label_widget = new QLabel("dome light enabled");
        label_widget->setMaximumWidth(150);
        label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
        grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

        auto value_widget = new QCheckBox();
        value_widget->setChecked(_model.viewSettings().domeLightEnabled());
        connect(value_widget, &QCheckBox::toggled, this, [&](bool v) {
            _model.viewSettings().setDomeLightEnabled(v);
        });
        grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
    }

    {
        auto row = 1;
        auto label_widget = new QLabel("dome light texture visible");
        label_widget->setMaximumWidth(150);
        label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
        grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

        auto value_widget = new QCheckBox();
        value_widget->setChecked(_model.viewSettings().domeLightTexturesVisible());
        connect(value_widget, &QCheckBox::toggled, this, [&](bool v) {
            _model.viewSettings().setDomeLightTexturesVisible(v);
        });
        grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
    }

    {
        auto row = 2;
        auto label_widget = new QLabel("render mode");
        label_widget->setMaximumWidth(150);
        label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
        grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

        auto value_widget = new QComboBox();
        for (int i = 0; i < int(RenderModes::Count); ++i) {
            value_widget->addItem(QString(to_constants(RenderModes(i)).c_str()));
        }
        value_widget->setCurrentIndex(int(_model.viewSettings().renderMode()));
        connect(value_widget, &QComboBox::activated, this, [&](int index) {
            _model.viewSettings().setRenderMode(RenderModes{index});
        });
        grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
    }
}

}// namespace vox