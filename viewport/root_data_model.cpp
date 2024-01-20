//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "root_data_model.h"
#include "common.h"

RootDataModel::RootDataModel()
    : _bboxCache{_currentFrame,
                 {to_constants(IncludedPurposes::DEFAULT),
                  to_constants(IncludedPurposes::PROXY)},
                 true},
      _xformCache{_currentFrame} {
}

std::optional<pxr::UsdStageRefPtr> &RootDataModel::stage() {
    return _stage;
}
void RootDataModel::setStage(std::optional<pxr::UsdStageRefPtr> &value) {
    if (value != _stage) {
        if (_pcListener) {
            pxr::TfNotice::Revoke(_pcListener.value());
            _pcListener = std::nullopt;
        }

        _stage = value;

        if (_stage) {
            _pcListener = pxr::TfNotice::Register(pxr::TfCreateWeakPtr(this),
                                                  &RootDataModel::_onPrimsChanged,
                                                  _stage.value());
        }
        emit signalStageReplaced();
    }
}

pxr::UsdTimeCode &RootDataModel::currentFrame() {
    return _currentFrame;
}
void RootDataModel::setCurrentFrame(pxr::UsdTimeCode &frame) {
    _currentFrame = frame;
    _bboxCache.SetTime(_currentFrame);
    _xformCache.SetTime(_currentFrame);
}

bool RootDataModel::playing() const {
    return _playing;
}
void RootDataModel::setPlaying(bool flags) {
    _playing = flags;
}

bool RootDataModel::useExtentsHint() {
    return _bboxCache.GetUseExtentsHint();
}
void RootDataModel::setUseExtentsHint(bool value) {
    if (value != _bboxCache.GetUseExtentsHint()) {
        // Unfortunate that we must blow the entire BBoxCache, but we have no
        // other alternative, currently.
        auto purposes = _bboxCache.GetIncludedPurposes();
        _bboxCache = pxr::UsdGeomBBoxCache(_currentFrame, purposes, value);
    }
}

const pxr::TfTokenVector &RootDataModel::includedPurposes() {
    return _bboxCache.GetIncludedPurposes();
}
void RootDataModel::setIncludedPurposes(const pxr::TfTokenVector &value) {
    _bboxCache.SetIncludedPurposes(value);
}

pxr::GfBBox3d RootDataModel::computeWorldBound(const pxr::UsdPrim &prim) {
    return _bboxCache.ComputeWorldBound(prim);
}

pxr::GfMatrix4d RootDataModel::getLocalToWorldTransform(const pxr::UsdPrim &prim) {
    return _xformCache.GetLocalToWorldTransform(prim);
}

pxr::UsdShadeMaterial RootDataModel::computeBoundMaterial(const pxr::UsdPrim &prim, const pxr::TfToken &materialPurpose) {
    return pxr::UsdShadeMaterialBindingAPI(prim).ComputeBoundMaterial(materialPurpose);
}

void RootDataModel::_emitPrimsChanged(ChangeNotice primChange, ChangeNotice propertyChange) {
    emit signalPrimsChanged(primChange, propertyChange);
}

void RootDataModel::_onPrimsChanged(pxr::UsdNotice::ObjectsChanged const &notice,
                                    pxr::UsdStageWeakPtr const &sender) {
    auto primChange = ChangeNotice::NONE;
    auto propertyChange = ChangeNotice::NONE;

    notice.GetResyncedPaths();

    for (const auto &p : notice.GetResyncedPaths()) {
        if (p.IsAbsoluteRootOrPrimPath()) {
            primChange = ChangeNotice::RESYNC;
        }
        if (p.IsPropertyPath()) {
            propertyChange = ChangeNotice::RESYNC;
        }
    }

    if (primChange == ChangeNotice::NONE or propertyChange == ChangeNotice::NONE) {
        for (auto &p : notice.GetChangedInfoOnlyPaths()) {
            if (p.IsPrimPath() and primChange == ChangeNotice::NONE) {
                primChange = ChangeNotice::INFOCHANGES;
            }
            if (p.IsPropertyPath() and propertyChange == ChangeNotice::NONE) {
                propertyChange = ChangeNotice::INFOCHANGES;
            }
        }
    }

    _emitPrimsChanged(primChange, propertyChange);
}
void RootDataModel::_clearCaches() {
    _bboxCache.Clear();
    _xformCache.Clear();
}