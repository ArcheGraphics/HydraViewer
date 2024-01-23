//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "viewport.h"
#include "camera.h"

#include <pxr/pxr.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/imaging/hgi/blitCmdsOps.h>
#include <pxr/imaging/hgiMetal/hgi.h>
#include <pxr/imaging/hgiMetal/texture.h>
#include <MetalKit/MetalKit.h>
#import <CoreImage/CIContext.h>

#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <QResizeEvent>

using namespace pxr;

namespace vox {
namespace {
const uint32_t MaxBuffersInFlight = 3;

/// Returns the current time in seconds from the system high-resolution clock.
inline double getCurrentTimeInSeconds() {
    using Clock = std::chrono::high_resolution_clock;
    using Ns = std::chrono::nanoseconds;
    std::chrono::time_point<Clock, Ns> tp = std::chrono::high_resolution_clock::now();
    return tp.time_since_epoch().count() / 1e9;
}

pxr::CameraUtilFraming _computeCameraFraming(pxr::GfVec4d viewport, pxr::GfVec2i renderBufferSize) {
    auto x = viewport[0];
    auto y = viewport[1];
    auto w = viewport[2];
    auto h = viewport[3];
    auto renderBufferWidth = renderBufferSize[0];
    auto renderBufferHeight = renderBufferSize[1];

    // Set display window equal to viewport - but flipped
    // since viewport is in y-Up coordinate system but
    // display window is y-Down.
    auto displayWindow = pxr::GfRange2f(
        GfVec2f(x, renderBufferHeight - y - h),
        GfVec2f(x + w, renderBufferHeight - y));

    // Intersect the display window with render buffer rect for
    // data window.
    auto renderBufferRect = GfRect2i(
        GfVec2i(0, 0), renderBufferWidth, renderBufferHeight);
    auto dataWindow = renderBufferRect.GetIntersection(
        GfRect2i(GfVec2i(x, renderBufferHeight - y - h), w, h));

    return CameraUtilFraming(displayWindow, dataWindow);
}

pxr::GfVec4i viewportMakeCenteredIntegral(pxr::GfVec4d viewport) {
    // The values are initially integral and containing the
    // the given rect
    auto left = int(floor(viewport[0]));
    auto bottom = int(floor(viewport[1]));
    auto right = int(ceil(viewport[0] + viewport[2]));
    auto top = int(ceil(viewport[1] + viewport[3]));

    auto width = right - left;
    auto height = top - bottom;

    // Compare the integral height to the original height
    // and do a centered 1 pixel adjustment if more than
    // a pixel off.
    if ((height - viewport[3]) > 1.0) {
        bottom += 1;
        height -= 2;
    }
    // Compare the integral width to the original width
    // and do a centered 1 pixel adjustment if more than
    // a pixel off.
    if ((width - viewport[2]) > 1.0) {
        left += 1;
        width -= 2;
    }
    return {left, bottom, width, height};
}
}// namespace

//----------------------------------------------------------------------------------------------------------------------
Viewport::Viewport(QWidget *parent, DataModel &model)
    : QWidget{parent}, _model{model} {
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAutoFillBackground(true);

    initializeEngine();
    auto size = parent->contentsRect().size();
    _swapchain = std::make_unique<Swapchain>((MTL::Device *)static_cast<HgiMetal *>(_hgi.get())->GetPrimaryDevice(),
                                             winId(), size.width(), size.height());

    _startTimeInSeconds = 0;

    connect(&_model, &DataModel::signalStageReplaced, this, &Viewport::_stageReplaced);
}

/// Initializes the Storm engine.
void Viewport::initializeEngine() {
    _inFlightSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);

    _hgi = Hgi::CreatePlatformDefaultHgi();
    HdDriver driver{HgiTokens->renderDriver, VtValue(_hgi.get())};

    _engine = std::make_unique<pxr::UsdImagingGLEngine>(driver);
    _engine->SetEnablePresentation(false);
    _engine->SetRendererAov(HdAovTokens->color);
}

void Viewport::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    auto size = computeWindowSize();
    _swapchain->resize(size[0], size[1]);
}

/// Updates the animation timing variables.
double Viewport::updateTime() {
    double currentTimeInSeconds = getCurrentTimeInSeconds();

    // Store the ticks for the first frame.
    if (_startTimeInSeconds == 0) {
        _startTimeInSeconds = currentTimeInSeconds;
    }

    // Calculate the elapsed time in seconds from the start.
    double elapsedTimeInSeconds = currentTimeInSeconds - _startTimeInSeconds;

    // Loop the animation if it is past the end.
    double timeCode = _startTimeCode + elapsedTimeInSeconds * _timeCodesPerSecond;
    if (timeCode > _endTimeCode) {
        timeCode = _startTimeCode;
        _startTimeInSeconds = currentTimeInSeconds;
    }

    return timeCode;
}

/// Draw the scene, and blit the result to the view.
/// Returns false if the engine wasn't initialized.
void Viewport::draw() {
    double timeCode = updateTime();

    // Start the next frame.
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    auto *hgi = static_cast<HgiMetal *>(_hgi.get());
    hgi->StartFrame();

    // Draw the scene using Hydra, and recast the result to a MTLTexture.
    CGSize viewSize = _swapchain->layer()->drawableSize();
    HgiTextureHandle hgiTexture = drawWithHydra(timeCode, viewSize);
    auto texture = static_cast<HgiMetalTexture *>(hgiTexture.Get())->GetTextureId();

    // Create a command buffer to blit the texture to the view.
    id<MTLCommandBuffer> commandBuffer = hgi->GetPrimaryCommandBuffer();
    __block dispatch_semaphore_t blockSemaphore = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSemaphore);
    }];

    // Copy the rendered texture to the view.
    _swapchain->present((MTL::CommandBuffer *)(commandBuffer), (MTL::Texture *)(texture));

    // Tell Hydra to commit the command buffer, and complete the work.
    hgi->CommitPrimaryCommandBuffer();
    hgi->EndFrame();
}

/// Draws the scene using Hydra.
pxr::HgiTextureHandle Viewport::drawWithHydra(double timeCode, CGSize viewSize) {
    // Camera projection setup.
    auto [gfCamera, cameraAspect] = resolveCamera();
    auto frustum = gfCamera.GetFrustum();
    auto cameraViewport = computeCameraViewport(cameraAspect);
    auto viewport = computeWindowViewport();
    if (_cropImageToCameraViewport()) {
        viewport = cameraViewport;
    }

    auto renderBufferSize = computeWindowSize();
    _engine->SetRenderBufferSize(renderBufferSize);
    _engine->SetFraming(_computeCameraFraming(viewport, renderBufferSize));
    _engine->SetWindowPolicy(computeWindowPolicy(cameraAspect));

    auto sceneCam = getActiveSceneCamera();
    if (sceneCam) {
        _engine->SetCameraPath(sceneCam->GetPath());
    } else {
        _engine->SetCameraState(frustum.ComputeViewMatrix(),
                                frustum.ComputeProjectionMatrix());
    }

    // Light and material setup.
    {
        GlfSimpleLightVector lights;
        auto l = pxr::GlfSimpleLight();
        l.SetAmbient({0, 0, 0, 0});
        auto cam_pos = frustum.GetPosition();
        l.SetPosition({(float)cam_pos[0], (float)cam_pos[1], (float)cam_pos[2], 1});
        lights.push_back(l);

        float kA = 0.2f;
        float kS = 0.1f;
        pxr::GlfSimpleMaterial light_mat;
        light_mat.SetAmbient(GfVec4f(kA, kA, kA, 1.0f));
        light_mat.SetSpecular(GfVec4f(kS, kS, kS, 1.0f));
        light_mat.SetShininess(32.0);
        pxr::GfVec4f sceneAmbient = GfVec4f(0.01f, 0.01f, 0.01f, 1.0f);
        _engine->SetLightingState(lights, light_mat, sceneAmbient);
    }

    // Nondefault render parameters.
    UsdImagingGLRenderParams params;
    params.clearColor = GfVec4f(0.0f, 0.0f, 0.0f, 0.0f);
    params.colorCorrectionMode = HdxColorCorrectionTokens->sRGB;
    params.frame = timeCode;

    // Render the frame.
    TfErrorMark mark;
    _engine->Render(_model.stage()->GetPseudoRoot(), params);
    TF_VERIFY(mark.IsClean(), "Errors occurred while rendering!");

    // Return the color output.
    return _engine->GetAovTexture(HdAovTokens->color);
}

pxr::UsdImagingGLRendererSettingsList Viewport::rendererSettingLists() {
    return _engine->GetRendererSettingsList();
}

pxr::VtValue Viewport::rendererSetting(pxr::TfToken const &id) {
    return _engine->GetRendererSetting(id);
}

void Viewport::setRendererSetting(pxr::TfToken const &id, pxr::VtValue const &value) {
    _engine->SetRendererSetting(id, value);
}

std::pair<pxr::GfCamera, float> Viewport::resolveCamera() {
    // If 'camera' is None, make sure we have a valid freeCamera
    auto sceneCam = getActiveSceneCamera();
    pxr::GfCamera gfCam;
    if (sceneCam) {
        gfCam = pxr::UsdGeomCamera(sceneCam.value()).GetCamera(_model.currentFrame());
    } else {
        switchToFreeCamera();
        gfCam = _model.viewSettings().freeCamera()->computeGfCamera(_bbox, autoClip());

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

std::shared_ptr<FreeCamera> Viewport::_createNewFreeCamera(ViewSettingsDataModel &viewSettings, bool isZUp) {
    auto aspectRatio = viewSettings.lockFreeCameraAspect() ? viewSettings.freeCameraAspect() : 1.0;
    return std::make_shared<FreeCamera>(
        isZUp,
        viewSettings.freeCameraFOV(),
        aspectRatio,
        viewSettings.freeCameraOverrideNear(),
        viewSettings.freeCameraOverrideFar());
}

std::optional<pxr::UsdPrim> Viewport::getActiveSceneCamera() {
    auto cameraPrim = _model.viewSettings().cameraPrim();
    if (cameraPrim && cameraPrim->IsActive()) {
        return cameraPrim;
    }
    return std::nullopt;
}

void Viewport::switchToFreeCamera(bool computeAndSetClosestDistance) {
    auto &viewSettings = _model.viewSettings();
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

bool Viewport::hasLockedAspectRatio() {
    return getActiveSceneCamera() || _model.viewSettings().lockFreeCameraAspect();
}

void Viewport::computeAndSetClosestDistance() {
    if (!_model.viewSettings().freeCamera()) {
        return;
    }
    auto cameraFrustum = resolveCamera().first.GetFrustum();
    auto trueFar = cameraFrustum.GetNearFar().GetMax();
    auto smallNear = std::min(FreeCamera::defaultNear, _model.viewSettings().freeCamera()->_selSize / 10.f);
    cameraFrustum.SetNearFar(pxr::GfRange1d(smallNear, smallNear * FreeCamera::maxSafeZResolution));
    auto pickResults = pick(cameraFrustum);
    if (!pickResults.has_value() || pickResults->outHitPrimPath == pxr::SdfPath::EmptyPath()) {
        cameraFrustum.SetNearFar(pxr::GfRange1d(trueFar / FreeCamera::maxSafeZResolution, trueFar));
        pickResults = pick(cameraFrustum);
    }

    if (pickResults.has_value() && pickResults->outHitPrimPath != pxr::SdfPath::EmptyPath()) {
        _model.viewSettings().freeCamera()->setClosestVisibleDistFromPoint(pickResults->outHitPoint);
        updateView();
    }
}

bool Viewport::autoClip() {
    return _model.viewSettings().autoComputeClippingPlanes();
}

bool Viewport::_fitCameraInViewport() {
    return ((_model.viewSettings().showMask() || _model.viewSettings().showMask_Outline() || showReticles()) && hasLockedAspectRatio());
}

bool Viewport::_cropImageToCameraViewport() {
    return ((_model.viewSettings().showMask() && _model.viewSettings().showMask_Opaque()) && hasLockedAspectRatio());
}

void Viewport::recomputeBBox() {
    auto selectedPrims = _model.selection().getLCDPrims();

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

void Viewport::resetCam(float frameFit) {
    auto validFrameRange = (!_selectionBrange.IsEmpty() && _selectionBrange.GetMax() != _selectionBrange.GetMin());
    if (validFrameRange) {
        switchToFreeCamera(false);
        _model.viewSettings().freeCamera()->frameSelection(_selectionBBox, frameFit);
        if (_model.viewSettings().autoComputeClippingPlanes()) {
            computeAndSetClosestDistance();
        }
    }
}

void Viewport::updateView(bool resetCam, bool forceComputeBBox, float frameFit) {
    auto computeBBox = forceComputeBBox || (_model.viewSettings().showBBoxes() && (_model.viewSettings().showAABBox() || _model.viewSettings().showOBBox())) || _bbox.GetRange().IsEmpty();
    if (computeBBox) {
        recomputeBBox();
    }
    if (resetCam) {
        this->resetCam(frameFit);
    }
}

pxr::GfBBox3d Viewport::_getDefaultBBox() {
    return pxr::GfBBox3d(pxr::GfRange3d({-10, -10, -10}, {10, 10, 10}));
}

bool Viewport::_isInfiniteBBox(pxr::GfBBox3d bbox) {
    return isinf(bbox.GetRange().GetMin().GetLength()) ||
           isinf(bbox.GetRange().GetMax().GetLength());
}

pxr::GfBBox3d Viewport::getStageBBox() {
    auto bbox = _model.computeWorldBound(_model.stage()->GetPseudoRoot());
    if (bbox.GetRange().IsEmpty() || _isInfiniteBBox(bbox)) {
        bbox = {};
    }
    return bbox;
}

pxr::GfBBox3d Viewport::getSelectionBBox() {
    auto bbox = pxr::GfBBox3d();
    for (auto &n : _model.selection().getLCDPrims()) {
        if (n.IsActive() && !n.IsInPrototype()) {
            auto primBBox = _model.computeWorldBound(n);
            bbox = pxr::GfBBox3d::Combine(bbox, primBBox);
        }
    }
    return bbox;
}

pxr::GfVec4i Viewport::computeCameraViewport(float cameraAspectRatio) {
    auto windowPolicy = pxr::CameraUtilMatchVertically;
    auto targetAspect = float(size().width()) / std::max(1.f, float(size().height()));
    if (targetAspect < cameraAspectRatio) {
        windowPolicy = pxr::CameraUtilMatchHorizontally;
    }
    auto viewport = pxr::GfRange2d(pxr::GfVec2d(0, 0),
                                   pxr::GfVec2d(size().width(), size().height()));
    viewport = pxr::CameraUtilConformedWindow(viewport, windowPolicy, cameraAspectRatio);

    pxr::GfVec4d viewport_float = {viewport.GetMin()[0], viewport.GetMin()[1], viewport.GetSize()[0], viewport.GetSize()[1]};
    return viewportMakeCenteredIntegral(viewport_float);
}

pxr::GfVec4i Viewport::computeWindowViewport() {
    auto size = computeWindowSize();
    return {0, 0, size[0], size[1]};
}

bool Viewport::showReticles() {
    return ((_model.viewSettings().showReticles_Inside() || _model.viewSettings().showReticles_Outside()) && hasLockedAspectRatio());
}

std::optional<Viewport::PickResult> Viewport::pick(const pxr::GfFrustum &pickFrustum) {
    if (!_model.stage()) {
        // error has already been issued
        return {};
    }

    // Need a correct OpenGL Rendering context for FBOs
    //   makeCurrent();

    // update rendering parameters
    _renderParams.frame = _model.currentFrame();
    _renderParams.complexity = _model.viewSettings().complexity().value();
    _renderParams.drawMode = _renderModeDict[_model.viewSettings().renderMode()];
    _renderParams.showGuides = _model.viewSettings().displayGuide();
    _renderParams.showProxy = _model.viewSettings().displayProxy();
    _renderParams.showRender = _model.viewSettings().displayRender();
    _renderParams.forceRefresh = _forceRefresh;
    _renderParams.cullStyle = _model.viewSettings().cullBackfaces() ?
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED :
                                  pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = true;
    _renderParams.enableSampleAlphaToCoverage = false;
    _renderParams.enableSceneMaterials = _model.viewSettings().enableSceneMaterials();
    _renderParams.enableSceneLights = _model.viewSettings().enableSceneLights();

    PickResult pickResult;
    auto result = _engine->TestIntersection(
        pickFrustum.ComputeViewMatrix(),
        pickFrustum.ComputeProjectionMatrix(),
        _model.stage()->GetPseudoRoot(), _renderParams,
        &pickResult.outHitPoint, &pickResult.outHitNormal, &pickResult.outHitPrimPath,
        &pickResult.outHitInstancerPath, &pickResult.outHitInstanceIndex, &pickResult.outInstancerContext);
    if (result) {
        return pickResult;
    } else {
        return std::nullopt;
    }
}

void Viewport::pickObject(qreal x, qreal y, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) {
    // todo
}

pxr::CameraUtilConformWindowPolicy Viewport::computeWindowPolicy(float cameraAspectRatio) {
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

pxr::GfVec2i Viewport::computeWindowSize() {
    return {int(double(this->size().width()) * devicePixelRatioF()), int(double(this->size().height()) * devicePixelRatioF())};
}

void Viewport::mousePressEvent(QMouseEvent *event) {
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

void Viewport::mouseReleaseEvent(QMouseEvent *event) {
    _cameraMode = CameraMode::None;
    _dragActive = false;
}

void Viewport::mouseMoveEvent(QMouseEvent *event) {
    auto x = event->x() * devicePixelRatioF();
    auto y = event->y() * devicePixelRatioF();

    if (_dragActive) {
        auto dx = x - _lastX;
        auto dy = y - _lastY;
        if (dx == 0 && dy == 0) {
            return;
        }

        auto freeCam = _model.viewSettings().freeCamera();
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

            _model.viewSettings().freeCamera()->Truck(
                -dx * pixelsToWorld,
                dy * pixelsToWorld);
        }

        _lastX = x;
        _lastY = y;

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

void Viewport::wheelEvent(QWheelEvent *event) {
    switchToFreeCamera();
    _model.viewSettings().freeCamera()->AdjustDistance(1.f - std::max(-0.5f, std::min(0.5f, (float(event->angleDelta().y()) / 1000.f))));
}

void Viewport::_stageReplaced() {
    if (_model.stage()) {
        _stageIsZup = (pxr::UsdGeomGetStageUpAxis(_model.stage()) == pxr::UsdGeomTokens->z);
        _model.viewSettings().setFreeCamera(_createNewFreeCamera(_model.viewSettings(), _stageIsZup));
    }
}

}// namespace vox
