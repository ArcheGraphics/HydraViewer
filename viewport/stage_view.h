//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
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
    void signalBboxUpdateTimeChanged(int);

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
        SelectionDataModel selectionDataModel;
        ViewSettingsDataModel viewSettingsDataModel;

        DefaultDataModel()
            : selectionDataModel(*this),
              viewSettingsDataModel(*this, nullptr) {}
    };

    [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }

    explicit StageView(QWidget *parent) noexcept;

    void renderParams();

    void setRenderParams();

    void autoClip();

    void showReticles();

    void cameraPrim();

    void setCameraPrim();

    void rolloverPicking();

    void setRolloverPicking();

    void fpsHUDInfo();

    void setFpsHUDInfo();

    void fpsHUDKeys();

    void setFpsHUDKeys();

    void upperHUDInfo();

    void setUpperHUDInfo();

    void HUDStatKeys();

    void setHUDStatKeys();

    void camerasWithGuides();

    void setCamerasWithGuides();

    void gfCamera();

    void cameraFrustum();

    void rendererDisplayName();

    void rendererAovName();

    void _fitCameraInViewport();

    void _cropImageToCameraViewport();

    void _getRenderer();

    void _handleRendererChanged();

    void _scaleMouseCoords();

    void closeRenderer();

    void GetRendererPlugins();

    void GetRendererDisplayName();

    void GetCurrentRendererId();

    void SetRendererPlugin();

    void GetRendererAovs();

    void SetRendererAov();

    void GetRendererSettingsList();

    void GetRendererSetting();

    void SetRendererSetting();

    void GetRendererCommands();

    void InvokeRendererCommand();

    void SetRendererPaused();

    void IsPauseRendererSupported();

    void IsRendererConverged();

    void SetRendererStopped();

    void IsStopRendererSupported();

    void _stageReplaced();

    void _createNewFreeCamera();

    void DrawAxis();

    void _processBBoxes();

    void DrawCameraGuides();

    void updateBboxPurposes();

    void recomputeBBox();

    void resetCam();

    void updateView();

    void updateSelection();

    void _getEmptyBBox();

    void _getDefaultBBox();

    void _isInfiniteBBox();

    void getStageBBox();

    void getSelectionBBox();

    void renderSinglePass();

    void initializeGL();

    void updateGL();

    void updateForPlayback();

    void getActiveSceneCamera();

    void hasLockedAspectRatio();

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

    void switchToFreeCamera();

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

    void *_lastComputedGfCamera{};
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

    void *_renderer{};
    bool _renderPauseState = false;
    bool _renderStopState = false;
    bool _reportedContextError = false;
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
};
}// namespace vox