//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <QWidget>

#include "selection_data_model.h"
#include "view_settings_data_model.h"

namespace vox {
class Rect {
public:
    void fromXYWH();
    void fromCorners();
    void scaledAndBiased();
    void difference();

private:
    void _splitAlongY();
    void _splitAlongX();
};

class OutlineRect : public Rect {
public:
};

class FilledRect : public Rect {
public:
};

class Prim2DSetupTask {
public:
};

class Prim2DDrawTask {
public:
};

class Outline : public Prim2DDrawTask {
public:
};

class Reticles : public Prim2DDrawTask {
public:
};

class Mask : public Prim2DDrawTask {
public:
};

class HUD {
public:
};

class StageView : public QWidget {
    Q_OBJECT
signals:
    void signalBboxUpdateTimeChanged(long long);

    void signalPrimSelected(pxr::SdfPath, int, pxr::SdfPath, int, pxr::GfVec3f,
                            Qt::MouseButton,
                            Qt::KeyboardModifiers);

    void signalPrimRollover(pxr::SdfPath, int, pxr::SdfPath, int,
                            pxr::GfVec3f, Qt::KeyboardModifiers);

    void signalMouseDrag();

    void signalErrorMessage(const QString &);

    void signalSwitchedToFreeCam();

    void signalFrustumChanged();

public:
    enum class CameraMode {
        None,
        Truck,
        Tumble,
        Zoom,
        Pick,
    };

    struct DefaultDataModel : public RootDataModel {
        DefaultDataModel()
            : _selectionDataModel(*this),
              _viewSettingsDataModel(*this, nullptr) {}

        inline SelectionDataModel &selection() {
            return _selectionDataModel;
        };

        inline ViewSettingsDataModel &viewSettings() {
            return _viewSettingsDataModel;
        }

    private:
        SelectionDataModel _selectionDataModel;
        ViewSettingsDataModel _viewSettingsDataModel;
    };

    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

    explicit StageView(QWidget *parent) noexcept;

    pxr::UsdImagingGLRenderParams &renderParams();

    void setRenderParams(const pxr::UsdImagingGLRenderParams &value);

    bool autoClip();

    bool showReticles();

    bool _fitCameraInViewport();

    bool _cropImageToCameraViewport();

    std::optional<pxr::UsdPrim> cameraPrim();

    void setCameraPrim(std::optional<pxr::UsdPrim> value);

    bool rolloverPicking() const;

    void setRolloverPicking(bool enabled);

    std::vector<int> &fpsHUDInfo();

    void setFpsHUDInfo(const std::vector<int> &info);

    std::vector<int> &fpsHUDKeys();

    void setFpsHUDKeys(const std::vector<int> &info);

    std::vector<int> &upperHUDInfo();

    void setUpperHUDInfo(const std::vector<int> &info);

    std::vector<int> &HUDStatKeys();

    void setHUDStatKeys(const std::vector<int> &keys);

    void *camerasWithGuides();

    void setCamerasWithGuides(void *value);

    /// Return the last computed Gf Camera
    pxr::GfCamera &gfCamera();

    /// Unlike the StageView.freeCamera property, which is invalid/None
    //  whenever we are viewing from a scene/stage camera, the 'cameraFrustum'
    //  property will always return the last-computed camera frustum, regardless
    //  of source.
    pxr::GfFrustum cameraFrustum();

    const std::string &rendererDisplayName();

    const pxr::TfToken &rendererAovName();

    std::shared_ptr<pxr::UsdImagingGLEngine> _getRenderer();

    void _handleRendererChanged(const pxr::TfToken &rendererId);

    QPoint _scaleMouseCoords(QPoint point);

    /// Close the current renderer.
    void closeRenderer();

    pxr::TfTokenVector GetRendererPlugins();

    std::string GetRendererDisplayName(const pxr::TfToken &rendererId);

    pxr::TfToken GetCurrentRendererId();

    bool SetRendererPlugin(pxr::TfToken &);

    pxr::TfTokenVector GetRendererAovs();

    bool SetRendererAov(const pxr::TfToken &aov);

    pxr::UsdImagingGLRendererSettingsList GetRendererSettingsList();

    pxr::VtValue GetRendererSetting(const pxr::TfToken &name);

    void SetRendererSetting(pxr::TfToken const &name,
                            pxr::VtValue const &value);

    pxr::HdCommandDescriptors GetRendererCommands();

    bool InvokeRendererCommand(const pxr::HdCommandDescriptor &command);

    void SetRendererPaused(bool paused);

    bool IsPauseRendererSupported();

    bool IsRendererConverged();

    void SetRendererStopped(bool);

    bool IsStopRendererSupported();

    /// Set the USD Stage this widget will be displaying. To decommission
    /// (even temporarily) this widget, supply None as 'stage'.
    void _stageReplaced();

    std::shared_ptr<FreeCamera> _createNewFreeCamera(ViewSettingsDataModel &viewSettings, bool isZUp);

    void DrawAxis();

    void _processBBoxes();

    void DrawCameraGuides();

    void updateBboxPurposes();

    void recomputeBBox();

    void resetCam(float frameFit = 1.1);

    void updateView(bool resetCam = false, bool forceComputeBBox = false, float frameFit = 1.1);

    void updateSelection();

    pxr::GfBBox3d _getEmptyBBox();

    pxr::GfBBox3d _getDefaultBBox();

    bool _isInfiniteBBox(pxr::GfBBox3d bbox);

    pxr::GfBBox3d getStageBBox();

    pxr::GfBBox3d getSelectionBBox();

    void renderSinglePass(pxr::UsdImagingGLDrawMode renderMode, bool renderSelHighlights);

    void updateGL();

    std::optional<pxr::UsdPrim> getActiveSceneCamera();

    /// True if the camera has a defined aspect ratio that should not change when the viewport is resized.
    bool hasLockedAspectRatio();

    void computeWindowPolicy();

    void computeWindowSize();

    void computeWindowViewport();

    void resolveCamera();

    void computeCameraViewport();

    void copyViewState();

    void restoreViewState();

    void paintGL();

    void drawHUD();

    void grabFrameBuffer();

    QSize sizeHint();

    void switchToFreeCamera(bool computeAndSetClosestDistance = true);

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void _onAutoComputeClippingChanged();

    void _onFreeCameraSettingChanged();

    void computeAndSetClosestDistance();

    void pick();

    void computePickFrustum();

    void pickObject();

    void glDraw();

    void SetForceRefresh();

    void ExportFreeCameraToStage();

    void ExportSession();

    void _primSelectionChanged();

private:
    DefaultDataModel _dataModel;
    bool _isFirstImage{true};

    pxr::GfCamera _lastComputedGfCamera{};
    float _lastAspectRatio = 1.0;
    Mask _mask;
    Outline _maskOutline;
    Reticles _reticles;
    HUD _hud;
    bool _stageIsZup = true;
    CameraMode _cameraMode = CameraMode::None;
    bool _rolloverPicking = false;
    bool _dragActive = false;
    int _lastX = 0;
    int _lastY = 0;

    std::shared_ptr<pxr::UsdImagingGLEngine> _renderer{};
    bool _renderPauseState = false;
    bool _renderStopState = false;
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

    float _dist = 50;
    pxr::GfBBox3d _bbox;
    pxr::GfBBox3d _selectionBBox;
    pxr::GfRange3d _selectionBrange;

    bool _forceRefresh{false};
    float _renderTime = 0;

    void *_camerasWithGuides{};

    std::vector<int> _fpsHUDInfo;
    std::vector<int> _fpsHUDKeys;
    std::vector<int> _upperHUDInfo;
    std::vector<int> _hudStatKeys;

    void *_glPrimitiveGeneratedQuery{};
    void *_glTimeElapsedQuery{};

    void *_axisVBO{};
    void *_bboxVBO{};
    void *_cameraGuidesVBO{};
    int _vao = 0;

    std::string _rendererDisplayName;
    pxr::TfToken _rendererAovName;
};
}// namespace vox