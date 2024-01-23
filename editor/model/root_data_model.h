//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QObject>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/base/tf/notice.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>

enum class ChangeNotice {
    NONE = 0,
    RESYNC = 1,
    INFOCHANGES = 2,
};

/// Data model providing centralized, moderated access to fundamental
/// information used throughout Usdview controllers, data models, and plugins.
class RootDataModel : public QObject, public pxr::TfWeakBase {
    Q_OBJECT
signals:
    void signalStageReplaced();
    void signalPrimsChanged(ChangeNotice primChange, ChangeNotice propertyChange);

public:
    RootDataModel();

    /// Get the current Usd.Stage object.
    pxr::UsdStageRefPtr &stage();
    /// Sets the current Usd.Stage object, and emits a signal if it is
    /// different from the previous stage.
    void setStage(pxr::UsdStageRefPtr stage);

    /// Get a Usd.TimeCode object which represents the current frame being
    ///   considered in Usdview.
    pxr::UsdTimeCode &currentFrame();
    /// Set the current frame to a new Usd.TimeCode object.
    void setCurrentFrame(pxr::UsdTimeCode &frame);

    bool playing() const;
    void setPlaying(bool flags);

    /// Return True if bounding box calculations use extents hints from prims.
    bool useExtentsHint();
    /// Set whether whether bounding box calculations should use extents from prims.
    void setUseExtentsHint(bool value);

    /// Get the set of included purposes used for bounding box calculations.
    std::set<pxr::TfToken> includedPurposes();
    /// Set a new set of included purposes for bounding box calculations.
    void setIncludedPurposes(const std::set<pxr::TfToken> &value);

    /// Compute the world-space bounds of a prim.
    pxr::GfBBox3d computeWorldBound(const pxr::UsdPrim &prim);
    /// Compute the transformation matrix of a prim.
    pxr::GfMatrix4d getLocalToWorldTransform(const pxr::UsdPrim &prim);
    /// Compute the material that the prim is bound to, for the given value of material purpose.
    static pxr::UsdShadeMaterial computeBoundMaterial(const pxr::UsdPrim &prim, const pxr::TfToken &materialPurpose);

private:
    pxr::UsdStageRefPtr _stage;
    pxr::UsdTimeCode _currentFrame{};
    bool _playing{false};
    pxr::UsdGeomBBoxCache _bboxCache;
    pxr::UsdGeomXformCache _xformCache;
    std::optional<pxr::TfNotice::Key> _pcListener;

    void _emitPrimsChanged(ChangeNotice primChange, ChangeNotice propertyChange);

    void _onPrimsChanged(pxr::UsdNotice::ObjectsChanged const &notice,
                         pxr::UsdStageWeakPtr const &sender);

    /// Clears internal caches of bounding box and transform data. Should be
    ///  called when the current stage is changed in a way which affects this
    ///  data.
    void _clearCaches();
};