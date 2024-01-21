//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "stage_view.h"

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/range3d.h>
#include <QtWidgets/QApplication>
#include <utility>
#include <pxr/usd/usdGeom/metrics.h>

namespace vox {
namespace {
void _computeCameraFraming() {}
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

void *StageView::camerasWithGuides() {
    return _camerasWithGuides;
}

void StageView::setCamerasWithGuides(void *value) {
    _camerasWithGuides = value;
}

pxr::GfCamera &StageView::gfCamera() {
    return _lastComputedGfCamera;
}

pxr::GfFrustum StageView::cameraFrustum() {
    return _lastComputedGfCamera.GetFrustum();
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
    setCamerasWithGuides(nullptr);

    if (_dataModel.stage()) {
        _stageIsZup = (pxr::UsdGeomGetStageUpAxis(*_dataModel.stage()) == pxr::UsdGeomTokens->z);
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

    auto psuRoot = _dataModel.stage().value()->GetPseudoRoot();
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
    auto bbox = _dataModel.computeWorldBound(_dataModel.stage().value()->GetPseudoRoot());
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
    _renderParams.clearColor = pxr::GfVec4f(_dataModel.viewSettings().clearColor().data());

    auto ccMode = _dataModel.viewSettings().colorCorrectionMode();
    _renderParams.colorCorrectionMode = pxr::TfToken(to_constants(ccMode));
    if (ccMode == ColorCorrectionModes::OPENCOLORIO) {
        _renderParams.ocioDisplay = pxr::TfToken(_dataModel.viewSettings().ocioSettings().display());
        _renderParams.ocioView = pxr::TfToken(_dataModel.viewSettings().ocioSettings().view());
        _renderParams.ocioColorSpace = pxr::TfToken(_dataModel.viewSettings().ocioSettings().colorSpace());
    }
    auto pseudoRoot = _dataModel.stage().value()->GetPseudoRoot();

    renderer->SetSelectionColor(pxr::GfVec4f(_dataModel.viewSettings().highlightColor().data()));
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

void StageView::computeWindowPolicy() {}

void StageView::computeWindowSize() {}

void StageView::computeWindowViewport() {}

void StageView::resolveCamera() {}

void StageView::computeCameraViewport() {}

void StageView::copyViewState() {}

void StageView::restoreViewState() {}

void StageView::paintGL() {}

void StageView::drawHUD() {}

void StageView::grabFrameBuffer() {}

QSize StageView::sizeHint() {
    return {460, 460};
}

void StageView::switchToFreeCamera(bool computeAndSetClosestDistance) {}

void StageView::mousePressEvent(QMouseEvent *event) {}

void StageView::mouseReleaseEvent(QMouseEvent *event) {}

void StageView::mouseMoveEvent(QMouseEvent *event) {}

void StageView::wheelEvent(QWheelEvent *event) {}

void StageView::_onAutoComputeClippingChanged() {}

void StageView::_onFreeCameraSettingChanged() {}

void StageView::computeAndSetClosestDistance() {}

void StageView::pick() {}

void StageView::computePickFrustum() {}

void StageView::pickObject() {}

void StageView::glDraw() {}

void StageView::SetForceRefresh() {}

void StageView::ExportFreeCameraToStage() {}

void StageView::ExportSession() {}

void StageView::_primSelectionChanged() {
    updateSelection();
    update();
}

}// namespace vox