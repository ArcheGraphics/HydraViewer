//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <string>
#include <utility>
#include "root_data_model.h"

/// Names of all available computed properties.
struct ComputedPropertyNames {
    static const std::string WORLD_BBOX;
    static const std::string LOCAL_WORLD_XFORM;
    static const std::string RESOLVED_PREVIEW_MATERIAL;
    static const std::string RESOLVED_FULL_MATERIAL;
};

class CustomAttribute;
std::vector<std::shared_ptr<CustomAttribute>> getCustomAttributes(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);

class CustomAttribute {
public:
    CustomAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);

    void IsVisible();

    virtual std::string GetName() = 0;

    void GetTypeName();

    pxr::SdfPath GetPrimPath();

protected:
    RootDataModel &_rootDataModel;
    pxr::UsdPrim &_currentPrim;
};

class BoundingBoxAttribute : public CustomAttribute {
public:
    BoundingBoxAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);

    std::string GetName() override;

    pxr::GfRange3d Get();
};

class LocalToWorldXformAttribute : public CustomAttribute {
public:
    LocalToWorldXformAttribute(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);

    std::string GetName() override;

    pxr::GfMatrix4d Get();
};

class ResolvedBoundMaterial : public CustomAttribute {
public:
    ResolvedBoundMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel, pxr::TfToken purpose);

    std::string GetName() override;

    pxr::SdfPath Get();

protected:
    pxr::TfToken _purpose;
};

class ResolvedFullMaterial : public ResolvedBoundMaterial {
public:
    ResolvedFullMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);
};

class ResolvedPreviewMaterial : public ResolvedBoundMaterial {
public:
    ResolvedPreviewMaterial(pxr::UsdPrim &currentPrim, RootDataModel &rootDataModel);
};

class ComputedPropertyFactory {
public:
    explicit ComputedPropertyFactory(RootDataModel &rootDataModel);

    /// Create a new computed property from a prim and property name.
    std::shared_ptr<CustomAttribute> getComputedProperty(pxr::UsdPrim &prim, const std::string &propName);

private:
    RootDataModel &_rootDataModel;
};