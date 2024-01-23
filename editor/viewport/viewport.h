//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <Metal/Metal.hpp>

#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>
#include <pxr/imaging/hgi/blitCmdsOps.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#import <QWidget>

#include "swapchain.h"
#include "camera.h"
#include "../model/data_model.h"

namespace vox {
class Viewport : public QWidget {
    Q_OBJECT
signals:
    void signalBboxUpdateTimeChanged(long long);

    void signalMouseDrag();

    void signalSwitchedToFreeCam();

    void signalFrustumChanged();

public:
    explicit Viewport(QWidget *parent, DataModel &model);

    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

    /// Draw the scene, and blit the result to the view.
    /// Returns false if the engine wasn't initialized.
    void draw();

    void resizeEvent(QResizeEvent *event) override;

    pxr::UsdImagingGLRendererSettingsList rendererSettingLists();

    pxr::VtValue rendererSetting(pxr::TfToken const &id);

    void setRendererSetting(pxr::TfToken const &id, pxr::VtValue const &value);

private:
    /// Initializes the Storm engine.
    void initializeEngine();

    /// Updates the animation timing variables.
    double updateTime();

    /// Draws the scene using Hydra.
    pxr::HgiTextureHandle drawWithHydra(double timeCode, CGSize viewSize);

private:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    /// Returns a tuple of the camera to use for rendering (either a scene
    /// camera or a free camera) and that camera's original aspect ratio.
    /// Depending on camera guide settings, the camera frustum may be conformed
    /// to fit the window viewport. Emits a signalFrustumChanged if the
    /// camera frustum has changed since the last time resolveCamera was called.
    std::pair<pxr::GfCamera, float> resolveCamera();

    std::shared_ptr<FreeCamera> _createNewFreeCamera(ViewSettingsDataModel &viewSettings, bool isZUp);

    std::optional<pxr::UsdPrim> getActiveSceneCamera();

    void switchToFreeCamera(bool computeAndSetClosestDistance = true);

    /// True if the camera has a defined aspect ratio that should not change when the viewport is resized.
    bool hasLockedAspectRatio();

    void computeAndSetClosestDistance();

    bool autoClip();

    bool _fitCameraInViewport();

    bool _cropImageToCameraViewport();

    void recomputeBBox();

    void resetCam(float frameFit = 1.1);

    void updateView(bool resetCam = false, bool forceComputeBBox = false, float frameFit = 1.1);

    pxr::GfBBox3d _getDefaultBBox();

    bool _isInfiniteBBox(pxr::GfBBox3d bbox);

    pxr::GfBBox3d getStageBBox();

    pxr::GfBBox3d getSelectionBBox();

    pxr::GfVec4i computeCameraViewport(float cameraAspectRatio);

    pxr::GfVec4i computeWindowViewport();

    pxr::CameraUtilConformWindowPolicy computeWindowPolicy(float cameraAspectRatio);

    pxr::GfVec2i computeWindowSize();

    bool showReticles();

    /// Set the USD Stage this widget will be displaying. To decommission
    /// (even temporarily) this widget, supply None as 'stage'.
    void _stageReplaced();

    struct PickResult {
        pxr::GfVec3d outHitPoint;
        pxr::GfVec3d outHitNormal;
        pxr::SdfPath outHitPrimPath;
        pxr::SdfPath outHitInstancerPath;
        int outHitInstanceIndex;
        pxr::HdInstancerContext outInstancerContext;
    };
    std::optional<PickResult> pick(const pxr::GfFrustum &pickFrustum);
    void pickObject(qreal x, qreal y, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

    std::optional<pxr::GfCamera> _lastComputedGfCamera{};
    float _lastAspectRatio = 1.0;
    bool _stageIsZup = true;
    pxr::GfBBox3d _bbox;
    pxr::GfBBox3d _selectionBBox;
    pxr::GfRange3d _selectionBrange;
    std::unordered_map<RenderModes, pxr::UsdImagingGLDrawMode> _renderModeDict = {
        {RenderModes::WIREFRAME, pxr::UsdImagingGLDrawMode::DRAW_WIREFRAME},
        {RenderModes::WIREFRAME_ON_SURFACE, pxr::UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE},
        {RenderModes::SMOOTH_SHADED, pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH},
        {RenderModes::POINTS, pxr::UsdImagingGLDrawMode::DRAW_POINTS},
        {RenderModes::FLAT_SHADED, pxr::UsdImagingGLDrawMode::DRAW_SHADED_FLAT},
        {RenderModes::GEOM_ONLY, pxr::UsdImagingGLDrawMode::DRAW_GEOM_ONLY},
        {RenderModes::GEOM_SMOOTH, pxr::UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH},
        {RenderModes::GEOM_FLAT, pxr::UsdImagingGLDrawMode::DRAW_GEOM_FLAT},
        {RenderModes::HIDDEN_SURFACE_WIREFRAME, pxr::UsdImagingGLDrawMode::DRAW_WIREFRAME}};
    pxr::UsdImagingGLRenderParams _renderParams;
    bool _forceRefresh{false};
    bool _dragActive = false;
    enum class CameraMode {
        None,
        Truck,
        Tumble,
        Zoom,
        Pick,
    };
    CameraMode _cameraMode = CameraMode::None;
    qreal _lastX = 0;
    qreal _lastY = 0;

private:
    DataModel &_model;

    dispatch_semaphore_t _inFlightSemaphore{};
    pxr::HgiUniquePtr _hgi;
    std::unique_ptr<pxr::UsdImagingGLEngine> _engine;
    std::unique_ptr<Swapchain> _swapchain{};

    double _startTimeInSeconds{};
    double _timeCodesPerSecond{};
    double _startTimeCode{};
    double _endTimeCode{};
};
}// namespace vox