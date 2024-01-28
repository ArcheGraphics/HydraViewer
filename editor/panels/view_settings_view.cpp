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
#include <QMetaClassInfo>
#include <QLCDNumber>

namespace vox {
ViewSettingsWidget::ViewSettingsWidget(DataModel &model) : _model{model} {
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

QSize ViewSettingsWidget::sizeHint() const {
    return {250, 250};
}

void ViewSettingsWidget::_link_view_model() {
    int row = 0;
    _create_combo_box<RenderModes>(row++, "renderMode");
    _create_combo_box<ColorCorrectionModes>(row++, "colorCorrectionModes");
    _create_combo_box<PickModes>(row++, "pickMode");
    _create_combo_box<CameraMaskModes>(row++, "cameraMaskModes");
    _create_combo_box<ClearColors>(row++, "clearColorText");
    _create_combo_box<HighlightColors>(row++, "highlightColorName");
    _create_combo_box<SelectionHighlightModes>(row++, "selHighlightMode");

    auto meta = _model.viewSettings().metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        auto p = meta->property(i);
        if (p.typeId() == QMetaType::Bool) {
            _create_check_box(row++, p.name());
        }
        if (p.typeId() == QMetaType::Int) {
            _create_spin_box(row++, p.name());
        }
        if (p.typeId() == QMetaType::Float || p.typeId() == QMetaType::Double) {
            _create_double_spin_box(row++, p.name());
        }
    }
}

void ViewSettingsWidget::_create_check_box(int row, const char *label) {
    auto label_widget = new QLabel(label);
    label_widget->setMaximumWidth(150);
    label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
    grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

    auto value_widget = new QCheckBox();
    value_widget->setChecked(_model.viewSettings().property(label).toBool());
    connect(value_widget, &QCheckBox::toggled, this, [label, this](bool v) {
        _model.viewSettings().setProperty(label, v);
    });
    grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
}

void ViewSettingsWidget::_create_spin_box(int row, const char *label) {
    auto label_widget = new QLabel(label);
    label_widget->setMaximumWidth(150);
    label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
    grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

    auto value_widget = new QSpinBox();
    value_widget->setValue(_model.viewSettings().property(label).toInt());
    connect(value_widget, &QSpinBox::valueChanged, this, [label, this](int v) {
        _model.viewSettings().setProperty(label, v);
    });
    grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
}

void ViewSettingsWidget::_create_double_spin_box(int row, const char *label) {
    auto label_widget = new QLabel(label);
    label_widget->setMaximumWidth(150);
    label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
    grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

    auto value_widget = new QDoubleSpinBox();
    value_widget->setValue(_model.viewSettings().property(label).toDouble());
    connect(value_widget, &QDoubleSpinBox::valueChanged, this, [label, this](double v) {
        _model.viewSettings().setProperty(label, v);
    });
    grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
}

}// namespace vox