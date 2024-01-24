//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include "model/data_model.h"

namespace vox {
class ViewSettingsView: public QWidget {
public:
    explicit ViewSettingsView(DataModel& model);

    [[nodiscard]] QSize sizeHint() const override;

private:
    void _link_view_model();

    DataModel& _model;
    QGridLayout *grid_layout;
};
}// namespace vox