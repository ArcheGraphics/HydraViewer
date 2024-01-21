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

void FreeCamera::_pullFromCameraTransform() {}

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

void FreeCamera::setClippingPlanes(pxr::GfBBox3d stageBBox) {}

void FreeCamera::computeGfCamera(pxr::GfBBox3d stageBBox, bool autoClip) {}

void FreeCamera::frameSelection(pxr::GfBBox3d selBBox, float frameFit) {}

void FreeCamera::setClosestVisibleDistFromPoint(pxr::GfVec3d point) {
    auto frustum = _camera.GetFrustum();
    auto camPos = frustum.GetPosition();
    auto camRay = pxr::GfRay(camPos, frustum.ComputeViewDirection());
    _closestVisibleDist = camRay.FindClosestPoint(point)[1];
    _lastFramedDist = dist();
    _lastFramedClosestDist = _closestVisibleDist;
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

void FreeCamera::AdjustDistance(float scaleFactor) {}

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
