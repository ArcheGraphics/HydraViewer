//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "data_model.h"

namespace vox {
DataModel::DataModel()
    : _selectionDataModel(*this),
      _viewSettingsDataModel(*this) {}
}// namespace vox