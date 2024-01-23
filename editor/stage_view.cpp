//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "stage_view.h"

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/camera.h>
#include <QtWidgets/QApplication>
#include <QWheelEvent>
#include <utility>

namespace vox {
namespace {
void _computeCameraFraming() {}
pxr::GfRange2d viewportMakeCenteredIntegral(pxr::GfRange2d viewport) {
    return {};
}
}// namespace

StageView::StageView(QWidget *parent) noexcept
    : QWidget{parent} {
}

pxr::UsdImagingGLRenderParams &StageView::renderParams() {
    return _renderParams;
}

void StageView::setRenderParams(const pxr::UsdImagingGLRenderParams &value) {
    _renderParams = value;
}

bool StageView::autoClip() {
    return _dataModel.viewSettings().autoComputeClippingPlanes();
}

bool StageView::showReticles() {
    return ((_dataModel.viewSettings().showReticles_Inside() || _dataModel.viewSettings().showReticles_Outside()) && hasLockedAspectRatio());
}

bool StageView::_fitCameraInViewport() {
    return ((_dataModel.viewSettings().showMask() || _dataModel.viewSettings().showMask_Outline() || showReticles()) && hasLockedAspectRatio());
}

bool StageView::_cropImageToCameraViewport() {
    return ((_dataModel.viewSettings().showMask() && _dataModel.viewSettings().showMask_Opaque()) && hasLockedAspectRatio());
}

std::optional<pxr::UsdPrim> StageView::cameraPrim() {
    return _dataModel.viewSettings().cameraPrim();
}

void StageView::setCameraPrim(std::optional<pxr::UsdPrim> value) {
    _dataModel.viewSettings().setCameraPrim(std::move(value));
}

bool StageView::rolloverPicking() const {
    return _rolloverPicking;
}

void StageView::setRolloverPicking(bool enabled) {
    _rolloverPicking = enabled;
    setMouseTracking(enabled);
}

std::vector<int> &StageView::fpsHUDInfo() {
    return _fpsHUDInfo;
}

void StageView::setFpsHUDInfo(const std::vector<int> &info) {
    _fpsHUDInfo = info;
}

std::vector<int> &StageView::fpsHUDKeys() {
    return _fpsHUDKeys;
}

void StageView::setFpsHUDKeys(const std::vector<int> &keys) {
    _fpsHUDKeys = keys;
}

std::vector<int> &StageView::upperHUDInfo() {
    return _upperHUDInfo;
}

void StageView::setUpperHUDInfo(const std::vector<int> &info) {
    _upperHUDInfo = info;
}

std::vector<int> &StageView::HUDStatKeys() {
    return _hudStatKeys;
}

void StageView::setHUDStatKeys(const std::vector<int> &keys) {
    _hudStatKeys = keys;
}

std::optional<pxr::GfCamera> StageView::gfCamera() {
    return _lastComputedGfCamera;
}

pxr::GfFrustum StageView::cameraFrustum() {
    return _lastComputedGfCamera->GetFrustum();
}

const std::string &StageView::rendererDisplayName() {
    return _rendererDisplayName;
}

const pxr::TfToken &StageView::rendererAovName() {
    return _rendererAovName;
}

std::shared_ptr<pxr::UsdImagingGLEngine> StageView::_getRenderer() {
    if (!_renderer) {
        _renderer = std::make_shared<pxr::UsdImagingGLEngine>();
        _handleRendererChanged(GetCurrentRendererId());
    }
    return _renderer;
}

void StageView::_handleRendererChanged(const pxr::TfToken &rendererId) {
    _rendererDisplayName = GetRendererDisplayName(rendererId);
    _rendererAovName = pxr::TfToken{"color"};
    _renderPauseState = false;
    _renderStopState = false;
    // XXX For HdSt we explicitely enable AOV via SetRendererAov
    // This is because ImagingGL / TaskController are spawned via prims in
    // Presto, so we default AOVs OFF until everything is AOV ready.
    SetRendererAov(rendererAovName());
}

QPoint StageView::_scaleMouseCoords(QPoint point) {
    return point * qGuiApp->devicePixelRatio();
}

void StageView::closeRenderer() {
    _renderer = nullptr;
}

pxr::TfTokenVector StageView::GetRendererPlugins() {
    if (_renderer) {
        return _renderer->GetRendererPlugins();
    } else {
        return {};
    }
}

std::string StageView::GetRendererDisplayName(const pxr::TfToken &plugId) {
    if (_renderer) {
        return _renderer->GetRendererDisplayName(plugId);
    } else {
        return "";
    }
}

pxr::TfToken StageView::GetCurrentRendererId() {
    if (_renderer) {
        return _renderer->GetCurrentRendererId();
    } else {
        return {};
    }
}

bool StageView::SetRendererPlugin(pxr::TfToken &plugId) {
    if (_renderer) {
        if (_renderer->SetRendererPlugin(plugId)) {
            _handleRendererChanged(plugId);
            updateGL();
            return true;
        }
    } else {
        return false;
    }
    return true;
}

pxr::TfTokenVector StageView::GetRendererAovs() {
    if (_renderer) {
        return _renderer->GetRendererAovs();
    } else {
        return {};
    }
}

bool StageView::SetRendererAov(const pxr::TfToken &aov) {
    if (_renderer) {
        if (_renderer->SetRendererAov(aov)) {
            _rendererAovName = aov;
            updateGL();
            return true;
        } else {
            return false;
        }
    }
    return true;
}

pxr::UsdImagingGLRendererSettingsList StageView::GetRendererSettingsList() {
    if (_renderer) {
        return _renderer->GetRendererSettingsList();
    } else {
        return {};
    }
}

pxr::VtValue StageView::GetRendererSetting(const pxr::TfToken &name) {
    if (_renderer) {
        return _renderer->GetRendererSetting(name);
    } else {
        return {};
    }
}

void StageView::SetRendererSetting(pxr::TfToken const &name,
                                   pxr::VtValue const &value) {
    if (_renderer) {
        _renderer->SetRendererSetting(name, value);
        updateGL();
    }
}

pxr::HdCommandDescriptors StageView::GetRendererCommands() {
    if (_renderer) {
        return _renderer->GetRendererCommandDescriptors();
    } else {
        return {};
    }
}

bool StageView::InvokeRendererCommand(const pxr::HdCommandDescriptor &command) {
    if (_renderer) {
        return _renderer->InvokeRendererCommand(command.commandName);
    } else {
        return false;
    }
}

void StageView::SetRendererPaused(bool paused) {
    if (_renderer && (!_renderer->IsConverged())) {
        if (paused) {
            _renderPauseState = _renderer->PauseRenderer();
        } else {
            _renderPauseState = !_renderer->ResumeRenderer();
        }
        updateGL();
    }
}

bool StageView::IsPauseRendererSupported() {
    if (_renderer) {
        if (_renderer->IsPauseRendererSupported()) {
            return true;
        }
    }
    return false;
}

bool StageView::IsRendererConverged() {
    return _renderer && _renderer->IsConverged();
}

void StageView::SetRendererStopped(bool stopped) {
    if (_renderer) {
        if (stopped) {
            _renderStopState = _renderer->StopRenderer();
        } else {
            _renderStopState = !_renderer->RestartRenderer();
        }
        updateGL();
    }
}

bool StageView::IsStopRendererSupported() {
    if (_renderer) {
        if (_renderer->IsStopRendererSupported()) {
            return true;
        }
    }

    return false;
}

void StageView::_stageReplaced() {
    if (_dataModel.stage()) {
        _stageIsZup = (pxr::UsdGeomGetStageUpAxis(_dataModel.stage()) == pxr::UsdGeomTokens->z);
        _dataModel.viewSettings().setFreeCamera(_createNewFreeCamera(_dataModel.viewSettings(), _stageIsZup));
    }
}

std::shared_ptr<FreeCamera> StageView::_createNewFreeCamera(ViewSettingsDataModel &viewSettings, bool isZUp) {
    auto aspectRatio = viewSettings.lockFreeCameraAspect() ? viewSettings.freeCameraAspect() : 1.0;
    return std::make_shared<FreeCamera>(
        isZUp,
        viewSettings.freeCameraFOV(),
        aspectRatio,
        viewSettings.freeCameraOverrideNear(),
        viewSettings.freeCameraOverrideFar());
}

void StageView::DrawAxis() {
    // todo
}

void StageView::_processBBoxes() {
    auto renderer = _getRenderer();
    if (!renderer) {
        // error has already been issued
        return;
    }

    // Determine if any bbox should be enabled
    auto enableBBoxes = _dataModel.viewSettings().showBBoxes() && (_dataModel.viewSettings().showBBoxPlayback() || !_dataModel.playing());

    if (enableBBoxes) {
        // Build the list of bboxes to draw
        pxr::UsdImagingGLRenderParams::BBoxVector bboxes = {};
        if (_dataModel.viewSettings().showAABBox()) {
            bboxes.emplace_back(_selectionBrange);
        }
        if (_dataModel.viewSettings().showOBBox()) {
            bboxes.emplace_back(_selectionBBox);
        }

        // Compute the color to use for the bbox lines
        auto col = _dataModel.viewSettings().clearColor();
        auto color = pxr::GfVec4f(col[0] > 0.5 ? col[0] - .6f : col[0] + .6f,
                                  col[1] > 0.5 ? col[1] - .6f : col[1] + .6f,
                                  col[2] > 0.5 ? col[2] - .6f : col[2] + .6f,
                                  1);
        color[0] = pxr::GfClamp(color[0], 0, 1);
        color[1] = pxr::GfClamp(color[1], 0, 1);
        color[2] = pxr::GfClamp(color[2], 0, 1);

        // Pass data to renderer via renderParams
        _renderParams.bboxes = bboxes;
        _renderParams.bboxLineColor = color;
        _renderParams.bboxLineDashSize = 3;
    } else {
        // No bboxes should be drawn
        _renderParams.bboxes = {};
    }
}

void StageView::DrawCameraGuides() {
    // todo
}

void StageView::updateBboxPurposes() {
    auto includedPurposes = _dataModel.includedPurposes();

    if (_dataModel.viewSettings().displayGuide()) {
        includedPurposes.insert(pxr::UsdGeomTokens->guide);
        auto iter = includedPurposes.find(pxr::UsdGeomTokens->guide);
    } else if (auto iter = includedPurposes.find(pxr::UsdGeomTokens->guide); iter != includedPurposes.end()) {
        includedPurposes.erase(iter);
    }

    if (_dataModel.viewSettings().displayProxy()) {
        includedPurposes.insert(pxr::UsdGeomTokens->proxy);
    } else if (auto iter = includedPurposes.find(pxr::UsdGeomTokens->proxy); iter != includedPurposes.end()) {
        includedPurposes.erase(iter);
    }

    if (_dataModel.viewSettings().displayRender()) {
        includedPurposes.insert(pxr::UsdGeomTokens->render);
    } else if (auto iter = includedPurposes.find(pxr::UsdGeomTokens->render); iter != includedPurposes.end()) {
        includedPurposes.erase(iter);
    }

    _dataModel.setIncludedPurposes(includedPurposes);
    // force the bbox to refresh
    _bbox = pxr::GfBBox3d();
}

void StageView::recomputeBBox() {
    auto selectedPrims = _dataModel.selection().getLCDPrims();

    auto startTime = std::chrono::steady_clock::now();
    _bbox = getStageBBox();
    if (selectedPrims.size() == 1 and selectedPrims[0].GetPath() == pxr::SdfPath("/")) {
        if (_bbox.GetRange().IsEmpty()) {
            _selectionBBox = _getDefaultBBox();
        } else {
            _selectionBBox = _bbox;
        }
    } else {
        _selectionBBox = getSelectionBBox();
    }

    // BBox computation time for HUD
    auto endTime = std::chrono::steady_clock::now();
    auto ms = endTime - startTime;
    emit signalBboxUpdateTimeChanged(ms.count());

    _selectionBrange = _selectionBBox.ComputeAlignedRange();
}

void StageView::resetCam(float frameFit) {
    auto validFrameRange = (!_selectionBrange.IsEmpty() && _selectionBrange.GetMax() != _selectionBrange.GetMin());
    if (validFrameRange) {
        switchToFreeCamera(false);
        _dataModel.viewSettings().freeCamera()->frameSelection(_selectionBBox, frameFit);
        if (_dataModel.viewSettings().autoComputeClippingPlanes()) {
            computeAndSetClosestDistance();
        }
    }
}

void StageView::updateView(bool resetCam, bool forceComputeBBox, float frameFit) {
    auto computeBBox = forceComputeBBox || (_dataModel.viewSettings().showBBoxes() && (_dataModel.viewSettings().showAABBox() || _dataModel.viewSettings().showOBBox())) || _bbox.GetRange().IsEmpty();
    if (computeBBox) {
        recomputeBBox();
    }
    if (resetCam) {
        this->resetCam(frameFit);
    }

    updateGL();
}

void StageView::updateSelection() {
    auto renderer = _getRenderer();
    if (!renderer) {
        // error has already been issued
        return;
    }

    renderer->ClearSelected();

    auto psuRoot = _dataModel.stage()->GetPseudoRoot();
    auto allInstances = _dataModel.selection().getPrimInstances();
    for (auto &prim : _dataModel.selection().getLCDPrims()) {
        if (prim == psuRoot) {
            continue;
        }
        auto primInstances = allInstances[prim];
        if (!primInstances.empty()) {
            for (auto &instanceIndex : primInstances) {
                renderer->AddSelected(prim.GetPath(), instanceIndex);
            }
        } else {
            renderer->AddSelected(
                prim.GetPath(), ALL_INSTANCES);
        }
    }
}

pxr::GfBBox3d StageView::_getEmptyBBox() {
    return {};
}

pxr::GfBBox3d StageView::_getDefaultBBox() {
    return pxr::GfBBox3d(pxr::GfRange3d({-10, -10, -10}, {10, 10, 10}));
}

bool StageView::_isInfiniteBBox(pxr::GfBBox3d bbox) {
    return isinf(bbox.GetRange().GetMin().GetLength()) ||
           isinf(bbox.GetRange().GetMax().GetLength());
}

pxr::GfBBox3d StageView::getStageBBox() {
    auto bbox = _dataModel.computeWorldBound(_dataModel.stage()->GetPseudoRoot());
    if (bbox.GetRange().IsEmpty() || _isInfiniteBBox(bbox)) {
        bbox = _getEmptyBBox();
    }
    return bbox;
}

pxr::GfBBox3d StageView::getSelectionBBox() {
    auto bbox = pxr::GfBBox3d();
    for (auto &n : _dataModel.selection().getLCDPrims()) {
        if (n.IsActive() && !n.IsInPrototype()) {
            auto primBBox = _dataModel.computeWorldBound(n);
            bbox = pxr::GfBBox3d::Combine(bbox, primBBox);
        }
    }
    return bbox;
}

void StageView::renderSinglePass(pxr::UsdImagingGLDrawMode renderMode, bool renderSelHighlights) {
    if (!_dataModel.stage()) {
        return;
    }
    auto renderer = _getRenderer();
    if (!renderer) {
        // error has already been issued
        return;
    }

    // update rendering parameters
    _renderParams.frame = _dataModel.currentFrame();
    _renderParams.complexity = _dataModel.viewSettings().complexity().value();
    _renderParams.drawMode = renderMode;
    _renderParams.showGuides = _dataModel.viewSettings().displayGuide();
    _renderParams.showProxy = _dataModel.viewSettings().displayProxy();
    _renderParams.showRender = _dataModel.viewSettings().displayRender();
    _renderParams.forceRefresh = _forceRefresh;
    _renderParams.cullStyle = _dataModel.viewSettings().cullBackfaces() ?
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED :
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;

    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = _dataModel.viewSettings().displayPrimId();
    _renderParams.enableSampleAlphaToCoverage = !_dataModel.viewSettings().displayPrimId();
    _renderParams.highlight = renderSelHighlights;
    _renderParams.enableSceneMaterials = _dataModel.viewSettings().enableSceneMaterials();
    _renderParams.enableSceneLights = _dataModel.viewSettings().enableSceneLights();
    _renderParams.clearColor = _dataModel.viewSettings().clearColor();

    auto ccMode = _dataModel.viewSettings().colorCorrectionMode();
    _renderParams.colorCorrectionMode = pxr::TfToken(to_constants(ccMode));
    if (ccMode == ColorCorrectionModes::OPENCOLORIO) {
        _renderParams.ocioDisplay = pxr::TfToken(_dataModel.viewSettings().ocioSettings().display());
        _renderParams.ocioView = pxr::TfToken(_dataModel.viewSettings().ocioSettings().view());
        _renderParams.ocioColorSpace = pxr::TfToken(_dataModel.viewSettings().ocioSettings().colorSpace());
    }
    auto pseudoRoot = _dataModel.stage()->GetPseudoRoot();

    renderer->SetSelectionColor(_dataModel.viewSettings().highlightColor());
    renderer->SetRendererSetting(
        pxr::TfToken("domeLightCameraVisibility"),
        pxr::VtValue(_dataModel.viewSettings().domeLightTexturesVisible()));

    _processBBoxes();

    renderer->Render(pseudoRoot, _renderParams);
    _forceRefresh = false;
}

void StageView::updateGL() {
    if (!_dataModel.playing()) {
        paintGL();
    }
}

std::optional<pxr::UsdPrim> StageView::getActiveSceneCamera() {
    auto cameraPrim = _dataModel.viewSettings().cameraPrim();
    if (cameraPrim && cameraPrim->IsActive()) {
        return cameraPrim;
    }
    return std::nullopt;
}

bool StageView::hasLockedAspectRatio() {
    return getActiveSceneCamera() || _dataModel.viewSettings().lockFreeCameraAspect();
}

pxr::CameraUtilConformWindowPolicy StageView::computeWindowPolicy(float cameraAspectRatio) {
    auto windowPolicy = pxr::CameraUtilMatchVertically;

    if (hasLockedAspectRatio()) {
        if (_cropImageToCameraViewport()) {
            auto targetAspect = float(size().width()) / std::max(1.f, float(size().height()));

            if (targetAspect < cameraAspectRatio) {
                windowPolicy = pxr::CameraUtilMatchHorizontally;
            }
        } else {
            if (_fitCameraInViewport()) {
                windowPolicy = pxr::CameraUtilFit;
            }
        }
    }

    return windowPolicy;
}

pxr::GfVec2i StageView::computeWindowSize() {
    auto size = this->size() * devicePixelRatioF();
    return {int(size.width()), int(size.height())};
}

pxr::GfRange2d StageView::computeWindowViewport() {
    return {pxr::GfVec2i{0, 0}, computeWindowSize()};
}

std::pair<pxr::GfCamera, float> StageView::resolveCamera() {
    // If 'camera' is None, make sure we have a valid freeCamera
    auto sceneCam = getActiveSceneCamera();
    pxr::GfCamera gfCam;
    if (sceneCam) {
        gfCam = pxr::UsdGeomCamera(sceneCam.value()).GetCamera(_dataModel.currentFrame());
    } else {
        switchToFreeCamera();
        gfCam = _dataModel.viewSettings().freeCamera()->computeGfCamera(_bbox, autoClip());

        if (hasLockedAspectRatio()) {
            // Copy the camera before calling ConformWindow so we don't
            // overwrite the camera's aspect ratio.
            gfCam = pxr::GfCamera(gfCam);
        }
    }
    auto cameraAspectRatio = gfCam.GetAspectRatio();

    // Conform the camera's frustum to the window viewport, if necessary.
    if (!_cropImageToCameraViewport()) {
        auto targetAspect = float(size().width()) / std::max(1.f, float(size().height()));
        if (_fitCameraInViewport()) {
            pxr::CameraUtilConformWindow(&gfCam, pxr::CameraUtilFit, targetAspect);
        } else {
            pxr::CameraUtilConformWindow(&gfCam, pxr::CameraUtilMatchVertically, targetAspect);
        }
    }
    auto frustumChanged = ((!_lastComputedGfCamera) || _lastComputedGfCamera->GetFrustum() != gfCam.GetFrustum());

    // We need to COPY the camera, not assign it...
    _lastComputedGfCamera = pxr::GfCamera(gfCam);
    _lastAspectRatio = cameraAspectRatio;
    if (frustumChanged) {
        emit signalFrustumChanged();
    }
    return {gfCam, cameraAspectRatio};
}

pxr::GfRange2d StageView::computeCameraViewport(float cameraAspectRatio) {
    auto windowPolicy = pxr::CameraUtilMatchVertically;
    auto targetAspect = float(size().width()) / std::max(1.f, float(size().height()));
    if (targetAspect < cameraAspectRatio) {
        windowPolicy = pxr::CameraUtilMatchHorizontally;
    }
    auto viewport = pxr::GfRange2d(pxr::GfVec2d(0, 0),
                                   pxr::GfVec2d(computeWindowSize()));
    viewport = pxr::CameraUtilConformedWindow(viewport, windowPolicy, cameraAspectRatio);

    viewport = pxr::GfRange2d(pxr::GfVec2d(viewport.GetMin()[0], viewport.GetMin()[1]),
                              pxr::GfVec2d(viewport.GetSize()[0], viewport.GetSize()[1]));
    viewport = viewportMakeCenteredIntegral(viewport);

    return viewport;
}

void StageView::copyViewState() {
    // todo
}

void StageView::restoreViewState() {
    // todo
}

void StageView::paintGL() {
    // todo
}

void StageView::drawHUD() {
    // todo
}

void StageView::grabFrameBuffer() {
    // todo
}

QSize StageView::sizeHint() {
    return {460, 460};
}

void StageView::switchToFreeCamera(bool computeAndSetClosestDistance) {
    auto &viewSettings = _dataModel.viewSettings();
    if (viewSettings.cameraPrim() != std::nullopt) {
        std::shared_ptr<FreeCamera> freeCamera{};
        // cameraPrim may no longer be valid,; so use the last-computed gf camera
        if (_lastComputedGfCamera) {
            freeCamera = std::make_shared<FreeCamera>(_lastComputedGfCamera.value(), _stageIsZup);
        } else {
            freeCamera = _createNewFreeCamera(viewSettings, _stageIsZup);
        }

        if (viewSettings.lockFreeCameraAspect()) {
            // Update free camera aspect ratio to match the current camera.
            if (_lastAspectRatio < freeCamera->aspectRatio()) {
                freeCamera->sethHorizontalAperture(_lastAspectRatio * freeCamera->verticalAperture());
            } else {
                freeCamera->setVerticalAperture(freeCamera->horizontalAperture() / _lastAspectRatio);
            }

            viewSettings.setCameraPrim(std::nullopt);
            viewSettings.setFreeCamera(freeCamera);

            if (computeAndSetClosestDistance) {
                this->computeAndSetClosestDistance();
            }
            // let the controller know we've done this!
            emit signalSwitchedToFreeCam();
        }
    }
}

void StageView::mousePressEvent(QMouseEvent *event) {
    _dragActive = true;
    auto x = event->position().x() * devicePixelRatioF();
    auto y = event->position().y() * devicePixelRatioF();

    if ((event->modifiers() & (Qt::AltModifier | Qt::MetaModifier))) {
        if (event->button() == Qt::LeftButton) {
            switchToFreeCamera();

            auto ctrlModifier = event->modifiers() & Qt::ControlModifier;
            _cameraMode = ctrlModifier ? CameraMode::Truck : CameraMode::Tumble;
        }
        if (event->button() == Qt::MiddleButton) {
            switchToFreeCamera();
            _cameraMode = CameraMode::Truck;
        }
        if (event->button() == Qt::RightButton) {
            switchToFreeCamera();
            _cameraMode = CameraMode::Zoom;
        }
    } else {
        _cameraMode = CameraMode::Pick;
        pickObject(x, y, event->button(), event->modifiers());
    }
    _lastX = x;
    _lastY = y;
}

void StageView::mouseReleaseEvent(QMouseEvent *event) {
    _cameraMode = CameraMode::None;
    _dragActive = false;
}

void StageView::mouseMoveEvent(QMouseEvent *event) {
    auto x = event->x() * devicePixelRatioF();
    auto y = event->y() * devicePixelRatioF();

    if (_dragActive) {
        auto dx = x - _lastX;
        auto dy = y - _lastY;
        if (dx == 0 && dy == 0) {
            return;
        }

        auto freeCam = _dataModel.viewSettings().freeCamera();
        if (_cameraMode == CameraMode::Tumble) {
            freeCam->Tumble(0.25f * dx, 0.25f * dy);
        } else if (_cameraMode == CameraMode::Zoom) {
            auto zoomDelta = -.002 * (dx + dy);
            if (freeCam->orthographic()) {
                // orthographic cameras zoom by scaling fov
                // fov is the height of the view frustum in world units
                freeCam->setFov(freeCam->fov() * (1 + zoomDelta));
            } else {
                // perspective cameras dolly forward or back
                freeCam->AdjustDistance(1.0 + zoomDelta);
            }
        } else if (_cameraMode == CameraMode::Truck) {
            auto height = float(size().height());
            auto pixelsToWorld = freeCam->ComputePixelsToWorldFactor(height);

            _dataModel.viewSettings().freeCamera()->Truck(
                -dx * pixelsToWorld,
                dy * pixelsToWorld);
        }

        _lastX = x;
        _lastY = y;
        updateGL();

        emit signalMouseDrag();
    } else if (_cameraMode == CameraMode::None) {
        // Mouse tracking is only enabled when rolloverPicking is enabled,
        // and this function only gets called elsewise when mouse-tracking
        // is enabled
        pickObject(event->position().x(), event->position().x(), Qt::MouseButton::NoButton, event->modifiers());
    } else {
        event->ignore();
    }
}

void StageView::wheelEvent(QWheelEvent *event) {
    switchToFreeCamera();
    _dataModel.viewSettings().freeCamera()->AdjustDistance(1.f - std::max(-0.5f, std::min(0.5f, (float(event->angleDelta().y()) / 1000.f))));
    updateGL();
}

void StageView::_onAutoComputeClippingChanged() {
    if (_dataModel.viewSettings().autoComputeClippingPlanes()) {
        if (!_dataModel.viewSettings().freeCamera()) {
            switchToFreeCamera();
        } else {
            computeAndSetClosestDistance();
        }
    }
}

void StageView::_onFreeCameraSettingChanged() {
    switchToFreeCamera();
    update();
}

void StageView::computeAndSetClosestDistance() {
    if (!_dataModel.viewSettings().freeCamera()) {
        return;
    }
    auto cameraFrustum = resolveCamera().first.GetFrustum();
    auto trueFar = cameraFrustum.GetNearFar().GetMax();
    auto smallNear = std::min(FreeCamera::defaultNear, _dataModel.viewSettings().freeCamera()->_selSize / 10.f);
    cameraFrustum.SetNearFar(pxr::GfRange1d(smallNear, smallNear * FreeCamera::maxSafeZResolution));
    auto pickResults = pick(cameraFrustum);
    if (!pickResults.has_value() || pickResults->outHitPrimPath == pxr::SdfPath::EmptyPath()) {
        cameraFrustum.SetNearFar(pxr::GfRange1d(trueFar / FreeCamera::maxSafeZResolution, trueFar));
        pickResults = pick(cameraFrustum);
    }

    if (pickResults.has_value() && pickResults->outHitPrimPath != pxr::SdfPath::EmptyPath()) {
        _dataModel.viewSettings().freeCamera()->setClosestVisibleDistFromPoint(pickResults->outHitPoint);
        updateView();
    }
}

std::optional<StageView::PickResult> StageView::pick(const pxr::GfFrustum &pickFrustum) {
    auto renderer = _getRenderer();
    if (!_dataModel.stage() || !renderer) {
        // error has already been issued
        return {};
    }

    // Need a correct OpenGL Rendering context for FBOs
    //   makeCurrent();

    // update rendering parameters
    _renderParams.frame = _dataModel.currentFrame();
    _renderParams.complexity = _dataModel.viewSettings().complexity().value();
    _renderParams.drawMode = _renderModeDict[_dataModel.viewSettings().renderMode()];
    _renderParams.showGuides = _dataModel.viewSettings().displayGuide();
    _renderParams.showProxy = _dataModel.viewSettings().displayProxy();
    _renderParams.showRender = _dataModel.viewSettings().displayRender();
    _renderParams.forceRefresh = _forceRefresh;
    _renderParams.cullStyle = _dataModel.viewSettings().cullBackfaces() ?
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED :
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = true;
    _renderParams.enableSampleAlphaToCoverage = false;
    _renderParams.enableSceneMaterials = _dataModel.viewSettings().enableSceneMaterials();
    _renderParams.enableSceneLights = _dataModel.viewSettings().enableSceneLights();

    PickResult pickResult;
    auto result = renderer->TestIntersection(
        pickFrustum.ComputeViewMatrix(),
        pickFrustum.ComputeProjectionMatrix(),
        _dataModel.stage()->GetPseudoRoot(), _renderParams,
        &pickResult.outHitPoint, &pickResult.outHitNormal, &pickResult.outHitPrimPath,
        &pickResult.outHitInstancerPath, &pickResult.outHitInstanceIndex, &pickResult.outInstancerContext);
    if (result) {
        return pickResult;
    } else {
        return std::nullopt;
    }
}

std::pair<bool, pxr::GfFrustum> StageView::computePickFrustum(qreal x, qreal y) {
    // compute pick frustum
    auto [gfCamera, cameraAspect] = resolveCamera();
    auto cameraFrustum = gfCamera.GetFrustum();

    auto viewport = computeWindowViewport();
    if (_cropImageToCameraViewport()) {
        viewport = computeCameraViewport(cameraAspect);
    }

    // normalize position and pick size by the viewport size
    auto point = pxr::GfVec2d((x - viewport.GetMin()[0]) / float(viewport.GetMax()[0]),
                              (y - viewport.GetMin()[1]) / float(viewport.GetMax()[1]));
    point[0] = (point[0] * 2.0 - 1.0);
    point[1] = -1.0 * (point[1] * 2.0 - 1.0);

    auto size = pxr::GfVec2d(1.0 / viewport.GetMax()[0], 1.0 / viewport.GetMax()[1]);

    // "point" is normalized to the image viewport size, but if the image
    // is cropped to the camera viewport, the image viewport won't fill the
    // whole window viewport.  Clicking outside the image will produce
    // normalized coordinates > 1 or < -1; in this case, we should skip
    // picking.
    auto inImageBounds = (abs(point[0]) <= 1.0 && abs(point[1]) <= 1.0);

    return {inImageBounds, cameraFrustum.ComputeNarrowedFrustum(point, size)};
}

void StageView::pickObject(qreal x, qreal y, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) {
    if (!_dataModel.stage()) {
        return;
    }
    auto renderer = _getRenderer();
    if (!renderer) {
        // error has already been issued
        return;
    }

    auto [inImageBounds, pickFrustum] = computePickFrustum(x, y);

    pxr::GfVec3d outHitPoint{};
    pxr::GfVec3d outHitNormal{};
    pxr::SdfPath outHitPrimPath;
    pxr::SdfPath outHitInstancerPath;
    int outHitInstanceIndex;
    pxr::HdInstancerContext outInstancerContext;
    if (inImageBounds) {
        auto result = pick(pickFrustum);
        outHitPoint = result->outHitPoint;
        outHitNormal = result->outHitNormal;
        outHitPrimPath = result->outHitPrimPath;
        outHitInstancerPath = result->outHitInstancerPath;
        outHitInstanceIndex = result->outHitInstanceIndex;
        outInstancerContext = result->outInstancerContext;
    } else {
        // If we're picking outside the image viewport (maybe because
        // camera guides are on), treat that as a de-select.
        outHitPoint = {-1.0, -1.0, -1.0};
        outHitPrimPath = pxr::SdfPath::EmptyPath();
        outHitInstancerPath = pxr::SdfPath::EmptyPath();
        outHitInstanceIndex = -1;
    }
    // Correct for high DPI displays
    // Cast to int explicitly as some versions of PySide/Shiboken throw
    // when converting extremely small doubles held in selectedPoint
    auto coord = _scaleMouseCoords(QPoint(int(outHitPoint[0]), int(outHitPoint[1])));
    outHitPoint[0] = coord.x();
    outHitPoint[1] = coord.y();

    if (button) {
        emit signalPrimSelected(outHitPrimPath, outHitInstanceIndex, outHitInstancerPath, outInstancerContext, outHitPoint, button, modifiers);
    } else {
        emit signalPrimRollover(outHitPrimPath, outHitInstanceIndex, outHitInstancerPath, outInstancerContext, outHitPoint, modifiers);
    }
}

void StageView::glDraw() {
    // todo
}

void StageView::SetForceRefresh(bool value) {
    _forceRefresh = value;
}

void StageView::ExportFreeCameraToStage(pxr::UsdStagePtr const &stage, const std::string &defcamName,
                                        std::optional<int> w, std::optional<int> h) {
    if (!_dataModel.viewSettings().freeCamera()) {
        return;
    }

    auto imgWidth = w.value_or(width());
    auto imgHeight = h.value_or(height());

    auto defcam = pxr::UsdGeomCamera::Define(stage, pxr::SdfPath("/" + defcamName));

    // Map free camera params to usd camera.We do * * not **want to burn
    // auto - clipping near / far into our exported camera
    auto gfCamera = _dataModel.viewSettings().freeCamera()->computeGfCamera(_bbox, false);

    auto targetAspect = float(imgWidth) / std::max(1.f, float(imgHeight));
    pxr::CameraUtilConformWindow(&gfCamera, pxr::CameraUtilMatchVertically, targetAspect);

    auto when = stage->HasAuthoredTimeCodeRange() ? _dataModel.currentFrame() : pxr::UsdTimeCode::Default();
    defcam.SetFromCamera(gfCamera, when);
}

void StageView::ExportSession(const std::string &stagePath, const std::string &defcamName,
                              std::optional<int> w, std::optional<int> h) {
    auto tmpStage = pxr::UsdStage::CreateNew(stagePath);
    if (_dataModel.stage()) {
        tmpStage->GetRootLayer()->TransferContent(_dataModel.stage()->GetSessionLayer());
    }
    if (!cameraPrim()) {
        // Export the free camera if it's the currently-visible camera
        ExportFreeCameraToStage(tmpStage, defcamName, w, h);
    }
    tmpStage->GetRootLayer()->Save();

    // Reopen just the tmp layer, to sublayer in the pose cache without
    // incurring Usd composition cost.
    if (_dataModel.stage()) {
        auto sdfLayer = pxr::SdfLayer::FindOrOpen(stagePath);
        sdfLayer->GetSubLayerPaths().push_back(_dataModel.stage()->GetRootLayer()->GetRealPath());
        sdfLayer->Save();
    }
}

void StageView::_primSelectionChanged() {
    updateSelection();
    update();
}

}// namespace vox