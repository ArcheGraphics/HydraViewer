//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "free_camera.h"
#include <QDebug>
#include <utility>
#include <pxr/base/gf/frustum.h>

FreeCamera::FreeCamera(bool isZup, float fov, float aspectRatio,
                       std::optional<float> overrideNear,
                       std::optional<float> overrideFar)
    : _isZUp{isZup} {
    _camera.SetPerspectiveFromAspectRatioAndFieldOfView(aspectRatio, fov,
                                                        pxr::GfCamera::FOVDirection::FOVVertical);
    _overrideNear = overrideNear;
    _overrideFar = overrideFar;
    resetClippingPlanes();

    if (isZup) {
        // This is also Gf.Camera.Y_UP_TO_Z_UP_MATRIX
        _yzUpMatrix = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), -90));
        _yzUpInvMatrix = _yzUpMatrix.GetInverse();
    } else {
        _yzUpMatrix = pxr::GfMatrix4d(1.0);
        _yzUpInvMatrix = pxr::GfMatrix4d(1.0);
    }
}

void FreeCamera::_pushToCameraTransform() {
    if (!_cameraTransformDirty) {
        return;
    }
    auto RotMatrix = [](pxr::GfVec3d vec, float angle) -> pxr::GfMatrix4d {
        return pxr::GfMatrix4d(1.0).SetRotate(pxr::GfRotation(vec, angle));
    };
    //  _YZUpInvMatrix influences the behavior about how the
    //  FreeCamera will tumble. It is the identity or a rotation about the
    //  x-Axis.
    _camera.SetTransform(
        pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d::ZAxis() * _dist) *
        RotMatrix(pxr::GfVec3d::ZAxis(), -_rotPsi) *
        RotMatrix(pxr::GfVec3d::XAxis(), -_rotPhi) *
        RotMatrix(pxr::GfVec3d::YAxis(), -_rotTheta) *
        _yzUpInvMatrix *
        pxr::GfMatrix4d().SetTranslate(_center));
    _camera.SetFocusDistance(_dist);

    _cameraTransformDirty = false;
}

void FreeCamera::_pullFromCameraTransform() {
    auto cam_transform = _camera.GetTransform();
    auto dist = _camera.GetFocusDistance();
    auto frustum = _camera.GetFrustum();
    auto cam_pos = frustum.GetPosition();
    auto cam_axis = frustum.ComputeViewDirection();

    // Compute translational parts
    _dist = dist;
    _selSize = dist / 10.0;
    _center = cam_pos + dist * cam_axis;

    // _YZUpMatrix influences the behavior about how the
    // FreeCamera will tumble. It is the identity or a rotation about the
    // x-Axis.

    // Compute rotational part
    auto transform = cam_transform * _yzUpMatrix;
    transform.Orthonormalize();
    auto rotation = transform.ExtractRotation();

    // Decompose and set angles
    auto decompose = -rotation.Decompose(pxr::GfVec3d::YAxis(), pxr::GfVec3d::XAxis(), pxr::GfVec3d::ZAxis());
    _rotTheta = decompose[0];
    _rotPhi = decompose[1];
    _rotPsi = decompose[2];

    _cameraTransformDirty = true;
}

std::pair<double, double> FreeCamera::_rangeOfBoxAlongRay(const pxr::GfRay &camRay,
                                                          const pxr::GfBBox3d &bbox,
                                                          bool debugClipping) const {
    auto maxDist = -std::numeric_limits<double>::infinity();
    auto minDist = std::numeric_limits<double>::infinity();
    auto boxRange = bbox.GetRange();
    auto boxXform = bbox.GetMatrix();
    for (int i = 0; i < 8; ++i) {
        // for each corner of the bounding box, transform to world
        // space and project
        auto point = boxXform.Transform(boxRange.GetCorner(i));
        auto pointDist = camRay.FindClosestPoint(point)[1];

        // find the projection of that point of the camera ray
        // and find the farthest and closest point.
        if (pointDist > maxDist)
            maxDist = pointDist;
        if (pointDist < minDist)
            minDist = pointDist;
    }
    if (debugClipping)
        qDebug("Projected bounds near/far: %f, %f", minDist, maxDist);

    // if part of the bbox is behind the ray origin (i.e. camera),
    // we clamp minDist to be positive.  Otherwise, reduce minDist by a bit
    // so that geometry at exactly the edge of the bounds won't be clipped -
    // do the same for maxDist, also!
    if (minDist < defaultNear) {
        minDist = defaultNear;
    } else {
        minDist *= 0.99;
    }
    maxDist *= 1.01;

    if (debugClipping)
        qDebug("Contracted bounds near/far: %f, %f", minDist, maxDist);

    return std::make_pair(minDist, maxDist);
}

void FreeCamera::resetClippingPlanes() {
    auto near = _overrideNear.value_or(FreeCamera::defaultNear);
    auto far = _overrideFar.value_or(FreeCamera::defaultFar);
    _camera.SetClippingRange(pxr::GfRange1f(near, far));
}

void FreeCamera::setClippingPlanes(pxr::GfBBox3d stageBBox) {
    bool debugClipping = true;
    float computedNear, computedFar;
    // If the scene bounding box is empty, or we are fully on manual
    // override, then just initialize to defaults.
    if (stageBBox.GetRange().IsEmpty() || (_overrideNear && _overrideFar)) {
        computedNear = FreeCamera::defaultNear;
        computedFar = FreeCamera::defaultFar;
    } else {
        // The problem: We want to include in the camera frustum all the
        // geometry the viewer should be able to see, i.e. everything within
        // the inifinite frustum starting at distance epsilon from the
        // camera it  However, the further the imageable geometry is
        // from the near-clipping plane, the less depth precision we will
        // have to resolve nearly colinear/incident polygons (which we get
        // especially with any doubleSided geometry).  We can run into such
        // situations astonishingly easily with large sets when we are
        // focussing in on just a part of a set that spans 10^5 units or
        // more.

        // Our solution: Begin by projecting the endpoints of the imageable
        // world's bounds onto the ray piercing the center of the camera
        // frustum, and take the near/far clipping distances from its
        // extent, clamping at a positive value for near.  To address the
        // z-buffer precision issue, we rely on someone having told us how
        // close the closest imageable geometry actually is to the camera,
        // by having called setClosestVisibleDistFromPoint(). This gives us
        // the most liberal near distance we can use and not clip the
        // geometry we are looking at.  We actually choose some fraction of
        // that distance instead, because we do not expect the someone to
        // recompute the closest point with every camera manipulation, as
        // it can be expensive (we do emit signalFrustumChanged to notify
        // them, however).  We only use this if the current range of the
        // bbox-based frustum will have precision issues.
        auto frustum = _camera.GetFrustum();
        auto camPos = frustum.GetPosition();

        auto camRay = pxr::GfRay(camPos, frustum.ComputeViewDirection());
        auto result = _rangeOfBoxAlongRay(camRay, stageBBox, debugClipping);
        computedNear = result.first;
        computedFar = result.second;
        auto precisionNear = computedFar / FreeCamera::maxGoodZResolution;

        if (_closestVisibleDist) {
            if (debugClipping)
                qDebug("Proposed near for precision: %f, closestDist: %f", precisionNear, _closestVisibleDist.value());

            // Because of our concern about orbit/truck causing
            // clipping, make sure we don't go closer than half the
            // distance to the closest visible point
            auto halfClose = _closestVisibleDist.value() / 2.f;

            if (_closestVisibleDist < _lastFramedClosestDist) {
                // This can happen if we have zoomed in closer since
                // the last time setClosestVisibleDistFromPoint() was called.
                // Clamp to precisionNear, which gives a balance between
                // clipping as we zoom in, vs bad z-fighting as we zoom in.
                // See AdjustDistance() for comment about better solution.
                halfClose = std::max(std::max(precisionNear, halfClose), computedNear);
                if (debugClipping)
                    qDebug("ADJUSTING: Accounting for zoom-in");
            }

            if (halfClose < computedNear) {
                // If there's stuff very very close to the camera, it
                // may have been clipped by computedNear.  Get it back!
                computedNear = halfClose;
                if (debugClipping)
                    qDebug("ADJUSTING: closestDist was closer than bboxNear");
            } else if (precisionNear > computedNear) {
                computedNear = std::min((precisionNear + halfClose) / 2.f, halfClose);
                if (debugClipping) {
                    qDebug("ADJUSTING: gaining precision by pushing out");
                }
            }
        }
    }
    auto near = _overrideNear.value_or(computedNear);
    auto far = _overrideFar.value_or(computedFar);
    // Make sure far is greater than near
    far = std::max(near + 1, far);

    if (debugClipping)
        qDebug("***Final Near/Far: %f, %f", near, far);

    _camera.SetClippingRange(pxr::GfRange1f(near, far));
}

pxr::GfCamera FreeCamera::computeGfCamera(pxr::GfBBox3d stageBBox, bool autoClip) {
    _pushToCameraTransform();
    if (autoClip) {
        setClippingPlanes(stageBBox);
    } else {
        resetClippingPlanes();
    }
    return _camera;
}

void FreeCamera::frameSelection(pxr::GfBBox3d selBBox, float frameFit) {
    _closestVisibleDist = std::nullopt;

    setCenter(selBBox.ComputeCentroid());
    auto selRange = selBBox.ComputeAlignedRange();
    auto size = selRange.GetSize();
    _selSize = std::max(std::max(size[0], size[1]), size[2]);
    if (orthographic()) {
        setFov(_selSize * frameFit);
        setDist(_selSize + FreeCamera::defaultNear);
    } else {
        auto halfFov = std::max(fov() * 0.5, 0.5);// don't divide by zero
        auto lengthToFit = _selSize * frameFit * 0.5;
        setDist(lengthToFit / atan(pxr::GfDegreesToRadians(halfFov)));
        // Very small objects that fill out their bounding boxes (like cubes)
        // may well pierce our 1 unit default near-clipping plane. Make sure
        // that doesn't happen.
        if (dist() < FreeCamera::defaultNear + _selSize * 0.5)
            setDist(FreeCamera::defaultNear + lengthToFit);
    }
}

void FreeCamera::setClosestVisibleDistFromPoint(pxr::GfVec3d point) {
    auto frustum = _camera.GetFrustum();
    auto camPos = frustum.GetPosition();
    auto camRay = pxr::GfRay(camPos, frustum.ComputeViewDirection());
    _closestVisibleDist = camRay.FindClosestPoint(point)[1];
    _lastFramedDist = dist();
    _lastFramedClosestDist = _closestVisibleDist.value();
}

float FreeCamera::ComputePixelsToWorldFactor(float viewportHeight) {
    _pushToCameraTransform();
    if (orthographic()) {
        return fov() / viewportHeight;
    } else {
        auto frustumHeight = _camera.GetFrustum().GetWindow().GetSize()[1];
        return float(frustumHeight) * _dist / viewportHeight;
    }
}

void FreeCamera::Tumble(float dTheta, float dPhi) {
    _rotTheta += dTheta;
    _rotPhi += dPhi;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

void FreeCamera::AdjustDistance(float scaleFactor) {
    // When dist gets very small, you can get stuck and not be able to
    // zoom back out, if you just keep multiplying.  Switch to addition
    // in that case, choosing an incr that works for the scale of the
    // framed geometry.
    if (scaleFactor > 1 && dist() < 2) {
        auto selBasedIncr = _selSize / 25.f;
        scaleFactor -= 1.0;
        setDist(dist() + std::min(selBasedIncr, scaleFactor));
    } else {
        setDist(dist() * scaleFactor);
    }

    // Make use of our knowledge that we are changing distance to camera
    // to also adjust _closestVisibleDist to keep it useful.  Make sure
    // not to recede farther than the last *computed* closeDist, since that
    // will generally cause unwanted clipping of close objects.
    // XXX:  This heuristic does a good job of preventing undesirable
    // clipping as we zoom in and out, but sacrifices the z-buffer
    // precision we worked hard to get.  If Hd/UsdImaging could cheaply
    // provide us with the closest-point from the last-rendered image,
    // we could use it safely here to update _closestVisibleDist much
    // more accurately than this calculation.
    if (_closestVisibleDist) {
        if (dist() > _lastFramedDist) {
            _closestVisibleDist = _lastFramedClosestDist;
        } else {
            _closestVisibleDist = _lastFramedClosestDist - _lastFramedDist + dist();
        }
    }
}

void FreeCamera::Truck(float deltaRight, float deltaUp) {
    // need to update the camera transform before we access the frustum
    _pushToCameraTransform();
    auto frustum = _camera.GetFrustum();
    auto cam_up = frustum.ComputeUpVector();
    auto cam_right = pxr::GfCross(frustum.ComputeViewDirection(), cam_up);
    _center += (deltaRight * cam_right + deltaUp * cam_up);
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

void FreeCamera::PanTilt(float dPan, float dTilt) {
    _camera.SetTransform((pxr::GfMatrix4d(1.0).SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), dTilt)) *
                          pxr::GfMatrix4d(1.0).SetRotate(pxr::GfRotation(pxr::GfVec3d::YAxis(), dPan)) *
                          _camera.GetTransform()));
    _pullFromCameraTransform();

    // When we Pan/Tilt, we don't want to roll the camera, so we just zero it
    // out here.
    _rotPsi = 0.0;

    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

void FreeCamera::Walk(float dForward, float dRight) {
    _pushToCameraTransform();
    auto frustum = _camera.GetFrustum();
    auto cam_up = frustum.ComputeUpVector().GetNormalized();
    auto cam_forward = frustum.ComputeViewDirection().GetNormalized();
    auto cam_right = pxr::GfCross(cam_forward, cam_up);
    auto delta = dForward * cam_forward + dRight * cam_right;
    _center += delta;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

FreeCamera::FreeCamera(pxr::GfCamera gfCamera, bool isZUp)
    : FreeCamera(isZUp) {
    _camera = std::move(gfCamera);
    _pullFromCameraTransform();
}

float FreeCamera::rotTheta() const {
    return _rotTheta;
}

void FreeCamera::setRotTheta(float value) {
    _rotTheta = value;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

float FreeCamera::rotPhi() const {
    return _rotPhi;
}

void FreeCamera::setRotPhi(float value) {
    _rotPhi = value;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

pxr::GfVec3d FreeCamera::center() {
    return _center;
}

void FreeCamera::setCenter(pxr::GfVec3d value) {
    _center = value;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

float FreeCamera::dist() const {
    return _dist;
}

void FreeCamera::setDist(float value) {
    _dist = value;
    _cameraTransformDirty = true;
    emit signalFrustumChanged();
}

bool FreeCamera::orthographic() {
    return _camera.GetProjection() == pxr::GfCamera::Orthographic;
}

void FreeCamera::setOrthographic(bool orthographic) {
    if (orthographic) {
        _camera.SetProjection(pxr::GfCamera::Orthographic);
    } else {
        _camera.SetProjection(pxr::GfCamera::Perspective);
    }
    emit signalFrustumChanged();
    emit signalFrustumSettingsChanged();
}

float FreeCamera::fov() {
    if (_camera.GetProjection() == pxr::GfCamera::Perspective) {
        return _camera.GetFieldOfView(pxr::GfCamera::FOVVertical);
    } else {
        return float(_camera.GetVerticalAperture() * pxr::GfCamera::APERTURE_UNIT);
    }
}

void FreeCamera::setFov(float value) {
    if (_camera.GetProjection() == pxr::GfCamera::Perspective) {
        _camera.SetPerspectiveFromAspectRatioAndFieldOfView(
            _camera.GetAspectRatio(), value, pxr::GfCamera::FOVVertical);
    } else {
        _camera.SetOrthographicFromAspectRatioAndSize(
            _camera.GetAspectRatio(), value, pxr::GfCamera::FOVVertical);
    }
    emit signalFrustumChanged();
    emit signalFrustumSettingsChanged();
}

float FreeCamera::aspectRatio() {
    return _camera.GetAspectRatio();
}

void FreeCamera::setAspectRatio(float value) {
    sethHorizontalAperture(value * verticalAperture());
}

float FreeCamera::horizontalAperture() {
    return _camera.GetHorizontalAperture();
}

void FreeCamera::sethHorizontalAperture(float value) {
    _camera.SetHorizontalAperture(value);
    emit signalFrustumChanged();
    emit signalFrustumSettingsChanged();
}

float FreeCamera::verticalAperture() {
    return _camera.GetVerticalAperture();
}

void FreeCamera::setVerticalAperture(float value) {
    _camera.SetVerticalAperture(value);
    emit signalFrustumChanged();
    emit signalFrustumSettingsChanged();
}

float FreeCamera::focalLength() {
    return _camera.GetFocalLength();
}

void FreeCamera::setFocalLength(float value) {
    _camera.SetFocalLength(value);
    emit signalFrustumChanged();
    emit signalFrustumSettingsChanged();
}

float FreeCamera::clippingNear() {
    return _camera.GetClippingRange().GetMin();
}

float FreeCamera::clippingFar() {
    return _camera.GetClippingRange().GetMax();
}

std::optional<float> FreeCamera::overrideNear() {
    return _overrideNear;
}

void FreeCamera::setOverrideNear(std::optional<float> value) {
    _overrideNear = value;
}

std::optional<float> FreeCamera::overrideFar() {
    return _overrideFar;
}

void FreeCamera::setOverrideFar(std::optional<float> value) {
    _overrideFar = value;
}
