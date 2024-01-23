//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "view_settings_data_model.h"
#include <pxr/usd/usdGeom/camera.h>

RefinementComplexities::RefinementComplexities(std::string compId, std::string name, float value)
    : _id{std::move(compId)}, _name(std::move(name)), _value{value} {
}

const RefinementComplexities RefinementComplexities::LOW = RefinementComplexities("low", "Low", 1.0);
const RefinementComplexities RefinementComplexities::MEDIUM = RefinementComplexities("medium", "Medium", 1.1);
const RefinementComplexities RefinementComplexities::HIGH = RefinementComplexities("high", "High", 1.2);
const RefinementComplexities RefinementComplexities::VERY_HIGH = RefinementComplexities("veryhigh", "Very High", 1.3);

OCIOSettings::OCIOSettings(std::string display, std::string view, std::string colorSpace)
    : _display(std::move(display)), _view(std::move(view)), _colorSpace(std::move(colorSpace)) {}

const std::string &OCIOSettings::display() {
    return _display;
}

const std::string &OCIOSettings::view() {
    return _view;
}

const std::string &OCIOSettings::colorSpace() {
    return _colorSpace;
}

ViewSettingsDataModel::ViewSettingsDataModel(RootDataModel &rootDataModel)
    : _rootDataModel{rootDataModel} {
    _cameraMaskColor = {0.1, 0.1, 0.1, 1.0};
    _cameraReticlesColor = {0.0, 0.7, 1.0, 1.0};
    _defaultMaterialAmbient = DEFAULT_AMBIENT;
    _defaultMaterialSpecular = DEFAULT_SPECULAR;
    _redrawOnScrub = true;
    _renderMode = RenderModes::SMOOTH_SHADED;
    _freeCameraFOV = 60.f;
    _freeCameraAspect = 1.f;
    // For freeCameraOverrideNear/Far, Use -inf as a sentinel value to mean
    // None. (We cannot use None directly because that would cause a type-
    // checking error in Settings.)
    _clippingPlaneNoneValue = -std::numeric_limits<float>::infinity();
    _freeCameraOverrideNear = _clippingPlaneNoneValue;
    _freeCameraOverrideFar = _clippingPlaneNoneValue;
    if (_freeCameraOverrideNear == _clippingPlaneNoneValue)
        _freeCameraOverrideNear = std::nullopt;
    if (_freeCameraOverrideFar == _clippingPlaneNoneValue)
        _freeCameraOverrideFar = std::nullopt;
    _lockFreeCameraAspect = false;
    _colorCorrectionMode = ColorCorrectionModes::SRGB;
    _ocioSettings = OCIOSettings();
    _pickMode = PickModes::PRIMS;

    // We need to store the trinary selHighlightMode state here,
    // because the stageView only deals in true/false (because it
    // cannot know anything about playback state).
    _selHighlightMode = SelectionHighlightModes::ONLY_WHEN_PAUSED;

    // We store the highlightColorName so that we can compare state during
    // initialization without inverting the name->value logic
    _highlightColorName = HighlightColors::YELLOW;
    _ambientLightOnly = true;
    _domeLightEnabled = false;
    _domeLightTexturesVisible = true;
    _clearColorText = ClearColors::DARK_GREY;
    _autoComputeClippingPlanes = false;
    _showBBoxPlayback = false;
    _showBBoxes = true;
    _showAABBox = true;
    _showOBBox = true;
    _displayGuide = false;
    _displayProxy = true;
    _displayRender = false;
    _displayPrimId = false;
    _enableSceneMaterials = true;
    _enableSceneLights = true;
    _cullBackfaces = false;
    _showInactivePrims = true;

    showAllMasterPrims = false;
    _showAllPrototypePrims = false;

    _showUndefinedPrims = false;
    _showAbstractPrims = false;
    _showPrimDisplayNames = true;
    _rolloverPrimInfo = false;
    _displayCameraOracles = false;
    _cameraMaskMode = CameraMaskModes::NONE;
    _showMask_Outline = false;
    _showReticles_Inside = false;
    _showReticles_Outside = false;
    _showHUD = true;

    _showHUD_Info = false;

    _showHUD_Complexity = true;
    _showHUD_Performance = true;
    _showHUD_GPUstats = false;

    _fontSize = 10;
}

pxr::GfVec4f ViewSettingsDataModel::cameraMaskColor() {
    return _cameraMaskColor;
}

void ViewSettingsDataModel::setCameraMaskColor(pxr::GfVec4f value) {
    _cameraMaskColor = value;
    _visibleViewSetting();
}

pxr::GfVec4f ViewSettingsDataModel::cameraReticlesColor() {
    return _cameraReticlesColor;
}

void ViewSettingsDataModel::setCameraReticlesColor(pxr::GfVec4f value) {
    _cameraReticlesColor = value;
    _visibleViewSetting();
}

float ViewSettingsDataModel::defaultMaterialAmbient() const { return _defaultMaterialAmbient; }

void ViewSettingsDataModel::setDefaultMaterialAmbient(float value) {
    if (value != _defaultMaterialAmbient) {
        _defaultMaterialAmbient = value;
        emit signalDefaultMaterialChanged();
    }
}

float ViewSettingsDataModel::defaultMaterialSpecular() const { return _defaultMaterialSpecular; }

void ViewSettingsDataModel::setDefaultMaterialSpecular(float value) {
    if (value != _defaultMaterialSpecular) {
        _defaultMaterialSpecular = value;
        emit signalDefaultMaterialChanged();
    }
}

void ViewSettingsDataModel::setDefaultMaterial(float ambient, float specular) {
    if (ambient != _defaultMaterialAmbient || specular != _defaultMaterialSpecular) {
        _defaultMaterialAmbient = ambient;
        _defaultMaterialSpecular = specular;
        emit signalDefaultMaterialChanged();
    }
}

void ViewSettingsDataModel::resetDefaultMaterial() {
    setDefaultMaterial(DEFAULT_AMBIENT, DEFAULT_SPECULAR);
}

RefinementComplexities ViewSettingsDataModel::complexity() {
    return _complexity;
}

void ViewSettingsDataModel::setComplexity(RefinementComplexities value) {
    _complexity = std::move(value);
    _visibleViewSetting();
}

RenderModes ViewSettingsDataModel::renderMode() {
    return _renderMode;
}

void ViewSettingsDataModel::setRenderMode(RenderModes value) {
    _renderMode = value;
    _visibleViewSetting();
}

float ViewSettingsDataModel::freeCameraFOV() const {
    return _freeCameraFOV;
}

void ViewSettingsDataModel::setFreeCameraFOV(float value) {
    if (_freeCamera) {
        // Setting the freeCamera's fov will trigger our own update
        _freeCamera->setFov(value);
    } else {
        _freeCameraFOV = value;
    }
    _freeCameraViewSetting();
}

std::optional<float> ViewSettingsDataModel::freeCameraOverrideNear() const {
    return _freeCameraOverrideNear;
}

void ViewSettingsDataModel::setFreeCameraOverrideNear(std::optional<float> value) {
    _freeCameraOverrideNear = value;
    _freeCameraViewSetting();
}

std::optional<float> ViewSettingsDataModel::freeCameraOverrideFar() const {
    return _freeCameraOverrideFar;
}

void ViewSettingsDataModel::setFreeCameraOverrideFar(std::optional<float> value) {
    _freeCameraOverrideFar = value;
    _freeCameraViewSetting();
}

float ViewSettingsDataModel::freeCameraAspect() const {
    return _freeCameraAspect;
}

void ViewSettingsDataModel::setFreeCameraAspect(float value) {
    if (_freeCamera) {
        // Setting the freeCamera's aspect ratio will trigger our own update
        _freeCamera->setAspectRatio(value);
    } else {
        _freeCameraAspect = value;
    }
    _freeCameraViewSetting();
}

bool ViewSettingsDataModel::lockFreeCameraAspect() const { return _lockFreeCameraAspect; }

void ViewSettingsDataModel::setLockFreeCameraAspect(bool value) {
    _lockFreeCameraAspect = value;

    if (value && !showMask()) {
        // Make sure the camera mask is turned on so the locked aspect ratio
        // is visible in the viewport.
        setCameraMaskMode(CameraMaskModes::FULL);
    }
    _visibleViewSetting();
}

ColorCorrectionModes ViewSettingsDataModel::colorCorrectionMode() {
    return _colorCorrectionMode;
}

void ViewSettingsDataModel::setColorCorrectionMode(ColorCorrectionModes value) {
    _colorCorrectionMode = value;
    _visibleViewSetting();
}

OCIOSettings &ViewSettingsDataModel::ocioSettings() {
    return _ocioSettings;
}

void ViewSettingsDataModel::setOcioSettings(OCIOSettings value) {
    _ocioSettings = std::move(value);
    _visibleViewSetting();
}

PickModes ViewSettingsDataModel::pickMode() {
    return _pickMode;
}

void ViewSettingsDataModel::setPickMode(PickModes value) {
    _pickMode = value;
    _invisibleViewSetting();
}

bool ViewSettingsDataModel::showAABBox() const {
    return _showAABBox;
}

void ViewSettingsDataModel::setShowAABBox(bool value) {
    _showAABBox = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showOBBox() const {
    return _showOBBox;
}

void ViewSettingsDataModel::setShowOBBox(bool value) {
    _showOBBox = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showBBoxes() const {
    return _showBBoxes;
}

void ViewSettingsDataModel::setShowBBoxes(bool value) {
    _showBBoxes = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::autoComputeClippingPlanes() const {
    return _autoComputeClippingPlanes;
}

void ViewSettingsDataModel::setAutoComputeClippingPlanes(bool value) {
    _autoComputeClippingPlanes = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showBBoxPlayback() const {
    return _showBBoxPlayback;
}

void ViewSettingsDataModel::setShowBBoxPlayback(bool value) {
    _showBBoxPlayback = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::displayGuide() const {
    return _displayGuide;
}

void ViewSettingsDataModel::setDisplayGuide(bool value) {
    _displayGuide = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::displayProxy() const {
    return _displayProxy;
}

void ViewSettingsDataModel::setDisplayProxy(bool value) {
    _displayProxy = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::displayRender() const {
    return _displayRender;
}

void ViewSettingsDataModel::setDisplayRender(bool value) {
    _displayRender = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::displayCameraOracles() const {
    return _displayCameraOracles;
}

void ViewSettingsDataModel::setDisplayCameraOracles(bool value) {
    _displayCameraOracles = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::displayPrimId() const {
    return _displayPrimId;
}

void ViewSettingsDataModel::setDisplayPrimId(bool value) {
    _displayPrimId = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::enableSceneMaterials() const {
    return _enableSceneMaterials;
}

void ViewSettingsDataModel::setEnableSceneMaterials(bool value) {
    _enableSceneMaterials = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::enableSceneLights() const {
    return _enableSceneLights;
}

void ViewSettingsDataModel::setEnableSceneLights(bool value) {
    _enableSceneLights = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::cullBackfaces() const {
    return _cullBackfaces;
}

void ViewSettingsDataModel::setCullBackfaces(bool value) {
    _cullBackfaces = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showInactivePrims() const {
    return _showInactivePrims;
}

void ViewSettingsDataModel::setShowInactivePrims(bool value) {
    _showInactivePrims = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showAllPrototypePrims() const {
    return _showAllPrototypePrims;
}

void ViewSettingsDataModel::setShowAllPrototypePrims(bool value) {
    _showAllPrototypePrims = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showUndefinedPrims() const {
    return _showUndefinedPrims;
}

void ViewSettingsDataModel::setShowUndefinedPrims(bool value) {
    _showUndefinedPrims = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showAbstractPrims() const {
    return _showAbstractPrims;
}

void ViewSettingsDataModel::setShowAbstractPrims(bool value) {
    _showAbstractPrims = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showPrimDisplayNames() const {
    return _showPrimDisplayNames;
}

void ViewSettingsDataModel::setShowPrimDisplayNames(bool value) {
    _showPrimDisplayNames = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::rolloverPrimInfo() const {
    return _rolloverPrimInfo;
}

void ViewSettingsDataModel::setRolloverPrimInfo(bool value) {
    _rolloverPrimInfo = value;
    _visibleViewSetting();
}

CameraMaskModes ViewSettingsDataModel::cameraMaskMode() {
    return _cameraMaskMode;
}

void ViewSettingsDataModel::setCameraMaskMode(CameraMaskModes value) {
    _cameraMaskMode = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showMask() {
    if (_cameraMaskMode == CameraMaskModes::FULL || _cameraMaskMode == CameraMaskModes::PARTIAL) {
        return true;
    }
    return false;
}

bool ViewSettingsDataModel::showMask_Opaque() {
    if (_cameraMaskMode == CameraMaskModes::FULL) {
        return true;
    }
    return false;
}

bool ViewSettingsDataModel::showMask_Outline() const {
    return _showMask_Outline;
}

void ViewSettingsDataModel::setShowMask_Outline(bool value) {
    _showMask_Outline = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showReticles_Inside() const {
    return _showReticles_Inside;
}

void ViewSettingsDataModel::setShowReticles_Inside(bool value) {
    _showReticles_Inside = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showReticles_Outside() const {
    return _showReticles_Outside;
}

void ViewSettingsDataModel::setShowReticles_Outside(bool value) {
    _showReticles_Outside = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showHUD() const {
    return _showHUD;
}

void ViewSettingsDataModel::setShowHUD(bool value) {
    _showHUD = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showHUD_Info() const {
    return _showHUD_Info;
}

void ViewSettingsDataModel::setShowHUD_Info(bool value) {
    _showHUD_Info = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showHUD_Complexity() const {
    return _showHUD_Complexity;
}

void ViewSettingsDataModel::setShowHUD_Complexity(bool value) {
    _showHUD_Complexity = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showHUD_Performance() const {
    return _showHUD_Performance;
}

void ViewSettingsDataModel::setShowHUD_Performance(bool value) {
    _showHUD_Performance = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::showHUD_GPUstats() const {
    return _showHUD_GPUstats;
}

void ViewSettingsDataModel::setShowHUD_GPUstats(bool value) {
    _showHUD_GPUstats = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::ambientLightOnly() const {
    return _ambientLightOnly;
}

void ViewSettingsDataModel::setAmbientLightOnly(bool value) {
    _ambientLightOnly = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::domeLightEnabled() const {
    return _domeLightEnabled;
}

void ViewSettingsDataModel::setDomeLightEnabled(bool value) {
    _domeLightEnabled = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::domeLightTexturesVisible() const {
    return _domeLightTexturesVisible;
}

void ViewSettingsDataModel::setDomeLightTexturesVisible(bool value) {
    _domeLightTexturesVisible = value;
    _visibleViewSetting();
}

ClearColors ViewSettingsDataModel::clearColorText() {
    return _clearColorText;
}

void ViewSettingsDataModel::setClearColorText(ClearColors value) {
    _clearColorText = value;
    _visibleViewSetting();
}

pxr::GfVec4f ViewSettingsDataModel::clearColor() {
    return _to_colors(clearColorText());
}

HighlightColors ViewSettingsDataModel::highlightColorName() {
    return _highlightColorName;
}

void ViewSettingsDataModel::setHighlightColorName(HighlightColors value) {
    _highlightColorName = value;
    _visibleViewSetting();
}

pxr::GfVec4f ViewSettingsDataModel::highlightColor() {
    return _to_colors(highlightColorName());
}

SelectionHighlightModes ViewSettingsDataModel::selHighlightMode() {
    return _selHighlightMode;
}

void ViewSettingsDataModel::setSelHighlightMode(SelectionHighlightModes value) {
    _selHighlightMode = value;
    _visibleViewSetting();
}

bool ViewSettingsDataModel::redrawOnScrub() const {
    return _redrawOnScrub;
}

void ViewSettingsDataModel::setRedrawOnScrub(bool value) {
    _redrawOnScrub = value;
    _visibleViewSetting();
}

std::shared_ptr<FreeCamera> ViewSettingsDataModel::freeCamera() {
    return _freeCamera;
}

void ViewSettingsDataModel::setFreeCamera(std::shared_ptr<FreeCamera> value) {
    if (_freeCamera) {
        QObject::disconnect(_freeCamera.get(), &FreeCamera::signalFrustumChanged,
                            this, &ViewSettingsDataModel::_frustumChanged);
        QObject::disconnect(_freeCamera.get(), &FreeCamera::signalFrustumSettingsChanged,
                            this, &ViewSettingsDataModel::_frustumSettingsChanged);
    }
    _freeCamera = std::move(value);
    if (_freeCamera) {
        QObject::connect(_freeCamera.get(), &FreeCamera::signalFrustumChanged,
                         this, &ViewSettingsDataModel::_frustumChanged);
        QObject::connect(_freeCamera.get(), &FreeCamera::signalFrustumSettingsChanged,
                         this, &ViewSettingsDataModel::_frustumSettingsChanged);
        _updateFreeCameraData();
    }
    _visibleViewSetting();
}

std::optional<pxr::SdfPath> ViewSettingsDataModel::cameraPath() {
    return _cameraPath;
}

void ViewSettingsDataModel::setCameraPath(std::optional<pxr::SdfPath> value) {
    _cameraPath = std::move(value);
    _visibleViewSetting();
}

std::optional<pxr::UsdPrim> ViewSettingsDataModel::cameraPrim() {
    if (cameraPath().has_value() && _rootDataModel.stage() != nullptr) {
        return _rootDataModel.stage()->GetPrimAtPath(cameraPath().value());
    } else {
        return std::nullopt;
    }
}

void ViewSettingsDataModel::setCameraPrim(std::optional<pxr::UsdPrim> value) {
    if (value.has_value()) {
        if (value.value().IsA<pxr::UsdGeomCamera>()) {
            setCameraPath(value.value().GetPrimPath());
        }
    }
    setCameraPath(std::nullopt);
}

int ViewSettingsDataModel::fontSize() const {
    return _fontSize;
}

void ViewSettingsDataModel::setFontSize(int value) {
    _fontSize = value;
    _visibleViewSetting();
}

void ViewSettingsDataModel::_frustumSettingsChanged() {
    _updateFreeCameraData();
    emit signalSettingChanged();
}

void ViewSettingsDataModel::_updateFreeCameraData() {
    if (_freeCamera) {
        _freeCameraFOV = _freeCamera->fov();
        _freeCameraOverrideNear = _freeCamera->overrideNear();
        _freeCameraOverrideFar = _freeCamera->overrideFar();
        if (_lockFreeCameraAspect)
            _freeCameraAspect = _freeCamera->aspectRatio();
    }
}

void ViewSettingsDataModel::_frustumChanged() {
    emit signalFreeCameraSettingChanged();
}

void ViewSettingsDataModel::_visibleViewSetting() {
    emit signalVisibleSettingChanged();
    emit signalSettingChanged();
}

void ViewSettingsDataModel::_invisibleViewSetting() {
    emit signalSettingChanged();
}

void ViewSettingsDataModel::_freeCameraViewSetting() {
    emit signalFreeCameraSettingChanged();
    emit signalSettingChanged();
}

pxr::GfVec4f ViewSettingsDataModel::_to_colors(ClearColors value) {
    switch (value) {
        case ClearColors::BLACK: return {0.0, 0.0, 0.0, 1.0};
        case ClearColors::DARK_GREY: return {0.07074, 0.07074, 0.07074, 1.0};
        case ClearColors::LIGHT_GREY: return {0.45626, 0.45626, 0.45626, 1.0};
        case ClearColors::WHITE: return {1.0, 1.0, 1.0, 1.0};
    }
}

pxr::GfVec4f ViewSettingsDataModel::_to_colors(HighlightColors value) {
    switch (value) {
        case HighlightColors::WHITE: return {1.0, 1.0, 1.0, 0.5};
        case HighlightColors::YELLOW: return {1.0, 1.0, 0.0, 0.5};
        case HighlightColors::CYAN: return {0.0, 1.0, 1.0, 0.5};
    }
}