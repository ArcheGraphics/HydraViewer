//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "custom_attributes.h"

const std::string ComputedPropertyNames::WORLD_BBOX = "World Bounding Box";
const std::string ComputedPropertyNames::LOCAL_WORLD_XFORM = "Local to World Xform";
const std::string ComputedPropertyNames::RESOLVED_PREVIEW_MATERIAL = "Resolved Preview Material";
const std::string ComputedPropertyNames::RESOLVED_FULL_MATERIAL = "Resolved Full Material";

std::vector<std::shared_ptr<CustomAttribute>> getCustomAttributes(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel) {
    if (currentPrim.IsA<pxr::UsdGeomImageable>()) {
        return {
            std::make_shared<BoundingBoxAttribute>(currentPrim, rootDataModel),
            std::make_shared<LocalToWorldXformAttribute>(currentPrim, rootDataModel),
            std::make_shared<ResolvedPreviewMaterial>(currentPrim, rootDataModel),
            std::make_shared<ResolvedFullMaterial>(currentPrim, rootDataModel)};
    }
    return {};
}

CustomAttribute::CustomAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel)
    : _rootDataModel{rootDataModel}, _currentPrim{currentPrim} {}

void CustomAttribute::IsVisible() {}

void CustomAttribute::GetTypeName() {}

pxr::SdfPath CustomAttribute::GetPrimPath() {
    return _currentPrim.GetPath();
}

BoundingBoxAttribute::BoundingBoxAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel)
    : CustomAttribute{currentPrim, rootDataModel} {}

std::string BoundingBoxAttribute::GetName() { return ComputedPropertyNames::WORLD_BBOX; }

pxr::GfRange3d BoundingBoxAttribute::Get() {
    auto bbox = _rootDataModel.computeWorldBound(_currentPrim);
    return bbox.ComputeAlignedRange();
}

LocalToWorldXformAttribute::LocalToWorldXformAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel)
    : CustomAttribute{currentPrim, rootDataModel} {}

std::string LocalToWorldXformAttribute::GetName() { return ComputedPropertyNames::LOCAL_WORLD_XFORM; }

pxr::GfMatrix4d LocalToWorldXformAttribute::Get() {
    return _rootDataModel.getLocalToWorldTransform(_currentPrim);
}

ResolvedBoundMaterial::ResolvedBoundMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel, pxr::TfToken purpose)
    : CustomAttribute{currentPrim, rootDataModel}, _purpose{std::move(purpose)} {}

std::string ResolvedBoundMaterial::GetName() {
    if (_purpose == pxr::UsdShadeTokensType().full) {
        return ComputedPropertyNames::RESOLVED_FULL_MATERIAL;
    } else if (_purpose == pxr::UsdShadeTokensType().preview) {
        return ComputedPropertyNames::RESOLVED_PREVIEW_MATERIAL;
    }
    return "";
}

pxr::SdfPath ResolvedBoundMaterial::Get() {
    auto boundMaterial = RootDataModel::computeBoundMaterial(_currentPrim, _purpose);
    return boundMaterial.GetPrim().GetPath();
}

ResolvedFullMaterial::ResolvedFullMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel)
    : ResolvedBoundMaterial{currentPrim, rootDataModel, pxr::UsdShadeTokensType().full} {}

ResolvedPreviewMaterial::ResolvedPreviewMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel)
    : ResolvedBoundMaterial{currentPrim, rootDataModel, pxr::UsdShadeTokensType().preview} {}

ComputedPropertyFactory::ComputedPropertyFactory(RootDataModel &rootDataModel)
    : _rootDataModel{rootDataModel} {
}

std::shared_ptr<CustomAttribute> ComputedPropertyFactory::getComputedProperty(pxr::UsdPrim &prim, const std::string &propName) {
    if (propName == ComputedPropertyNames::WORLD_BBOX) {
        return std::make_shared<BoundingBoxAttribute>(prim, _rootDataModel);
    } else if (propName == ComputedPropertyNames::LOCAL_WORLD_XFORM) {
        return std::make_shared<LocalToWorldXformAttribute>(prim, _rootDataModel);
    } else if (propName == ComputedPropertyNames::RESOLVED_FULL_MATERIAL) {
        return std::make_shared<ResolvedFullMaterial>(prim, _rootDataModel);
    } else if (propName == ComputedPropertyNames::RESOLVED_PREVIEW_MATERIAL) {
        return std::make_shared<ResolvedPreviewMaterial>(prim, _rootDataModel);
    }
    return nullptr;
}
