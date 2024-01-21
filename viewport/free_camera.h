//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QtCore/QObject>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/bbox3d.h>

class FreeCamera : public QObject {
    Q_OBJECT
signals:
    void signalFrustumChanged();

    void signalFrustumSettingsChanged();
public:
    float _selSize{10};

    static constexpr float defaultNear = 1;
    static constexpr float defaultFar = 2000000;
    // Experimentally on Nvidia M6000, if Far/Near is greater than this,
    // then geometry in the back half of the volume will disappear
    static constexpr float maxSafeZResolution = 1e6;
    // Experimentally on Nvidia M6000, if Far/Near is greater than this,
    // then we will often see Z-fighting artifacts even for geometry that
    // is close to camera, when rendering for picking
    static constexpr float maxGoodZResolution = 5e4;

    explicit FreeCamera(bool isZup, float fov = 60, float aspectRatio = 1.0,
                        std::optional<float> overrideNear = std::nullopt,
                        std::optional<float> overrideFar = std::nullopt);

    FreeCamera(pxr::GfCamera gfCamera, bool isZUp);

    /// Set near and far back to their uncomputed defaults.
    void resetClippingPlanes();

    /// Computes and sets automatic clipping plane distances using the
    //  camera's position and orientation, the bounding box
    //  surrounding the stage, and the distance to the closest rendered
    //  object in the central view of the camera (closestVisibleDist).
    //
    //  If either of the "override" clipping attributes are not None,
    //  we use those instead
    void setClippingPlanes(pxr::GfBBox3d stageBBox);

    /// Makes sure the FreeCamera's computed parameters are up-to-date, and
    //  returns the GfCamera object.  If 'autoClip' is True, then compute
    //  "optimal" positions for the near/far clipping planes based on the
    //  current closestVisibleDist, in order to maximize Z-buffer resolution
    pxr::GfCamera computeGfCamera(pxr::GfBBox3d stageBBox, bool autoClip = false);

    /// needs to be recomputed
    void frameSelection(pxr::GfBBox3d selBBox, float frameFit);

    void setClosestVisibleDistFromPoint(pxr::GfVec3d point);

    /// Computes the ratio that converts pixel distance into world units.
    //
    //  It treats the pixel distances as if they were projected to a plane going
    //  through the camera center
    float ComputePixelsToWorldFactor(float value);

    /// Tumbles the camera around the center point by (dTheta, dPhi) degrees.
    void Tumble(float dTheta, float dPhi);

    /// Scales the distance of the freeCamera from it's center typically by
    // scaleFactor unless it puts the camera into a "stuck" state.
    void AdjustDistance(float scaleFactor);

    /// Moves the camera by (deltaRight, deltaUp) in worldspace coordinates.
    //
    // This is similar to a camera Truck/Pedestal.
    void Truck(float deltaRight, float deltaUp);

    /// Rotates the camera around the current camera base (approx. the film
    //  plane).  Both parameters are in degrees.
    //
    //  This moves the center point that we normally tumble around.
    //
    //  This is similar to a camera Pan/Tilt.
    void PanTilt(float dPan, float dTilt);

    /// Specialized camera movement that moves it on the "horizontal" plane
    void Walk(float dForward, float dRight);

    [[nodiscard]] float rotTheta() const;

    void setRotTheta(float value);

    [[nodiscard]] float rotPhi() const;

    void setRotPhi(float value);

    pxr::GfVec3d center();

    void setCenter(pxr::GfVec3d value);

    [[nodiscard]] float dist() const;

    void setDist(float value);

    bool orthographic();

    void setOrthographic(bool value);

    /// The vertical field of view, in degrees, for perspective cameras.
    //  For orthographic cameras fov is the height of the view frustum, in
    //  world units.
    float fov();

    void setFov(float value);

    float aspectRatio();

    /// Sets the aspect ratio by adjusting the horizontal aperture.
    void setAspectRatio(float value);

    float horizontalAperture();

    void sethHorizontalAperture(float value);

    float verticalAperture();

    void setVerticalAperture(float value);

    float focalLength();

    void setFocalLength(float value);

    // near
    float clippingNear();

    // far
    float clippingFar();

    std::optional<float> overrideNear();

    void setOverrideNear(std::optional<float> value);

    std::optional<float> overrideFar();

    void setOverrideFar(std::optional<float> value);

private:
    pxr::GfCamera _camera;
    std::optional<float> _overrideNear;
    std::optional<float> _overrideFar;

    bool _isZUp;
    bool _cameraTransformDirty{true};
    float _rotTheta{0};
    float _rotPhi{0};
    float _rotPsi{0};
    pxr::GfVec3d _center{};
    float _dist{100};
    pxr::GfMatrix4d _yzUpMatrix{};
    pxr::GfMatrix4d _yzUpInvMatrix{};
    std::optional<float> _closestVisibleDist;
    std::optional<float> _lastFramedDist;
    std::optional<float> _lastFramedClosestDist;

    // Updates the camera's transform matrix, that is, the matrix that brings
    // the camera to the origin, with the camera view pointing down:
    //   +Y if this is a Zup camera, or
    //   -Z if this is a Yup camera .
    void _pushToCameraTransform();

    /// Updates parameters (center, rotTheta, etc.) from the camera transform.
    void _pullFromCameraTransform();

    [[nodiscard]] std::pair<double, double> _rangeOfBoxAlongRay(const pxr::GfRay &camRay,
                                                                const pxr::GfBBox3d &bbox,
                                                                bool debugClipping = false) const;
};