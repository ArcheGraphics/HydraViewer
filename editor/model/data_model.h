//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include "root_data_model.h"
#include "view_settings_data_model.h"
#include "selection_data_model.h"
#include "../node/graph_model.h"

namespace vox {
struct DataModel : public RootDataModel {
    DataModel();

    inline SelectionDataModel &selection() {
        return _selectionDataModel;
    };

    inline ViewSettingsDataModel &viewSettings() {
        return _viewSettingsDataModel;
    }

    inline SimpleGraphModel &graphModel() {
        return _graphModel;
    }

private:
    SimpleGraphModel _graphModel;
    SelectionDataModel _selectionDataModel;
    ViewSettingsDataModel _viewSettingsDataModel;
};
}// namespace vox