//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "model/data_model.h"

namespace vox {
class ViewSettingsWidget : public QWidget {
public:
    explicit ViewSettingsWidget(DataModel &model);

    [[nodiscard]] QSize sizeHint() const override;

private:
    void _link_view_model();

    void _create_check_box(int row, const char *label);

    void _create_spin_box(int row, const char *label);

    void _create_double_spin_box(int row, const char *label);

    template<typename TYPE>
    void _create_combo_box(int row, const char *label) {
        auto label_widget = new QLabel(label);
        label_widget->setMaximumWidth(150);
        label_widget->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
        grid_layout->addWidget(label_widget, row, 0, Qt::AlignLeft);

        auto value_widget = new QComboBox();
        for (int i = 0; i < int(TYPE::Count); ++i) {
            value_widget->addItem(QString(to_constants(TYPE(i)).c_str()));
        }
        value_widget->setCurrentIndex(_model.viewSettings().property(label).toInt());
        connect(value_widget, &QComboBox::activated, this, [this, label](int i) {
            _model.viewSettings().setProperty(label, i);
        });
        grid_layout->addWidget(value_widget, row, 1, Qt::AlignLeft);
    }

    DataModel &_model;
    QGridLayout *grid_layout;
};
}// namespace vox