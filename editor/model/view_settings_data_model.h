//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include "root_data_model.h"
#include "common.h"
#include "free_camera.h"
#include <QObject>

struct RefinementComplexities {
    RefinementComplexities(std::string compId, std::string name, float value);
    static const RefinementComplexities LOW;
    static const RefinementComplexities MEDIUM;
    static const RefinementComplexities HIGH;
    static const RefinementComplexities VERY_HIGH;

    inline float value() const {
        return _value;
    }

private:
    std::string _id;
    std::string _name;
    float _value;
};

/// Class to hold OCIO display, view, and colorSpace config settings as strings.
class OCIOSettings {
public:
    explicit OCIOSettings(std::string display = "", std::string view = "", std::string colorSpace = "");

    const std::string &display();

    const std::string &view();

    const std::string &colorSpace();

private:
    std::string _display;
    std::string _view;
    std::string _colorSpace;
};

/// Data model containing settings related to the rendered view of a USD file.
class ViewSettingsDataModel : public QObject {
    Q_OBJECT
signals:
    /// emitted when any view setting changes
    void signalSettingChanged();
    /// emitted when any view setting which may affect the rendered image changes
    void signalVisibleSettingChanged();
    /// emitted when any view setting that affects the free camera changes. This
    /// signal allows clients to switch to the free camera whenever its settings
    /// are modified. Some operations may cause this signal to be emitted multiple
    /// times.
    void signalFreeCameraSettingChanged();
    /// emitted when autoClipping changes value, so that clients can initialize
    /// it efficiently.  This signal will be emitted *before*
    /// signalVisibleSettingChanged when autoClipping changes.
    void signalAutoComputeClippingPlanesChanged();
    /// emitted when any aspect of the defaultMaterial changes
    void signalDefaultMaterialChanged();
    /// emitted when any setting affecting the GUI style changes
    void signalStyleSettingsChanged();

public:
    static constexpr float DEFAULT_AMBIENT = 0.2;
    static constexpr float DEFAULT_SPECULAR = 0.1;

    explicit ViewSettingsDataModel(RootDataModel &rootDataModel);

    pxr::GfVec4f cameraMaskColor();

    void setCameraMaskColor(pxr::GfVec4f value);

    pxr::GfVec4f cameraReticlesColor();

    void setCameraReticlesColor(pxr::GfVec4f value);

    [[nodiscard]] float defaultMaterialAmbient() const;
    void setDefaultMaterialAmbient(float val);

    [[nodiscard]] float defaultMaterialSpecular() const;
    void setDefaultMaterialSpecular(float val);

    void setDefaultMaterial(float ambient, float specular);

    void resetDefaultMaterial();

    RefinementComplexities complexity();

    void setComplexity(RefinementComplexities value);

    RenderModes renderMode();

    void setRenderMode(RenderModes value);

    [[nodiscard]] float freeCameraFOV() const;

    void setFreeCameraFOV(float value);

    /// Returns the free camera's near clipping plane value, if it has been
    /// overridden by the user. Returns None if there is no user-defined near
    /// clipping plane.
    [[nodiscard]] std::optional<float> freeCameraOverrideNear() const;

    /// Sets the near clipping plane to the given value. Passing in None will
    ///  clear the current override.
    void setFreeCameraOverrideNear(std::optional<float> value);

    /// Returns the free camera's far clipping plane value, if it has been
    //  overridden by the user. Returns None if there is no user-defined far
    //  clipping plane.
    [[nodiscard]] std::optional<float> freeCameraOverrideFar() const;

    /// Sets the far clipping plane to the given value. Passing in None will
    ///  clear the current override
    void setFreeCameraOverrideFar(std::optional<float> value);

    [[nodiscard]] float freeCameraAspect() const;

    void setFreeCameraAspect(float value);

    [[nodiscard]] bool lockFreeCameraAspect() const;

    void setLockFreeCameraAspect(bool val);

    ColorCorrectionModes colorCorrectionMode();

    void setColorCorrectionMode(ColorCorrectionModes value);

    OCIOSettings &ocioSettings();

    /// Specifies the OCIO settings to be used. Setting the OCIO 'display'
    //  requires a 'view' to be specified.
    void setOcioSettings(OCIOSettings value);

    PickModes pickMode();

    void setPickMode(PickModes value);

    [[nodiscard]] bool showAABBox() const;

    void setShowAABBox(bool value);

    [[nodiscard]] bool showOBBox() const;

    void setShowOBBox(bool value);

    [[nodiscard]] bool showBBoxes() const;

    void setShowBBoxes(bool value);

    [[nodiscard]] bool autoComputeClippingPlanes() const;

    void setAutoComputeClippingPlanes(bool value);

    [[nodiscard]] bool showBBoxPlayback() const;

    void setShowBBoxPlayback(bool value);

    [[nodiscard]] bool displayGuide() const;

    void setDisplayGuide(bool value);

    [[nodiscard]] bool displayProxy() const;

    void setDisplayProxy(bool value);

    [[nodiscard]] bool displayRender() const;

    void setDisplayRender(bool value);

    [[nodiscard]] bool displayCameraOracles() const;

    void setDisplayCameraOracles(bool value);

    [[nodiscard]] bool displayPrimId() const;

    void setDisplayPrimId(bool value);

    [[nodiscard]] bool enableSceneMaterials() const;

    void setEnableSceneMaterials(bool value);

    [[nodiscard]] bool enableSceneLights() const;

    void setEnableSceneLights(bool value);

    [[nodiscard]] bool cullBackfaces() const;

    void setCullBackfaces(bool value);

    [[nodiscard]] bool showInactivePrims() const;

    void setShowInactivePrims(bool value);

    [[nodiscard]] bool showAllPrototypePrims() const;

    void setShowAllPrototypePrims(bool value);

    [[nodiscard]] bool showUndefinedPrims() const;

    void setShowUndefinedPrims(bool value);

    [[nodiscard]] bool showAbstractPrims() const;

    void setShowAbstractPrims(bool value);

    [[nodiscard]] bool showPrimDisplayNames() const;

    void setShowPrimDisplayNames(bool value);

    [[nodiscard]] bool rolloverPrimInfo() const;

    void setRolloverPrimInfo(bool value);

    CameraMaskModes cameraMaskMode();

    void setCameraMaskMode(CameraMaskModes value);

    bool showMask();

    bool showMask_Opaque();

    [[nodiscard]] bool showMask_Outline() const;

    void setShowMask_Outline(bool value);

    [[nodiscard]] bool showReticles_Inside() const;

    void setShowReticles_Inside(bool value);

    [[nodiscard]] bool showReticles_Outside() const;

    void setShowReticles_Outside(bool value);

    [[nodiscard]] bool showHUD() const;

    void setShowHUD(bool value);

    [[nodiscard]] bool showHUD_Info() const;

    void setShowHUD_Info(bool value);

    [[nodiscard]] bool showHUD_Complexity() const;

    void setShowHUD_Complexity(bool value);

    [[nodiscard]] bool showHUD_Performance() const;

    void setShowHUD_Performance(bool value);

    [[nodiscard]] bool showHUD_GPUstats() const;

    void setShowHUD_GPUstats(bool value);

    [[nodiscard]] bool ambientLightOnly() const;

    void setAmbientLightOnly(bool value);

    [[nodiscard]] bool domeLightEnabled() const;

    void setDomeLightEnabled(bool value);

    [[nodiscard]] bool domeLightTexturesVisible() const;

    void setDomeLightTexturesVisible(bool value);

    ClearColors clearColorText();

    void setClearColorText(ClearColors value);

    pxr::GfVec4f clearColor();

    HighlightColors highlightColorName();

    void setHighlightColorName(HighlightColors value);

    pxr::GfVec4f highlightColor();

    SelectionHighlightModes selHighlightMode();

    void setSelHighlightMode(SelectionHighlightModes value);

    [[nodiscard]] bool redrawOnScrub() const;

    void setRedrawOnScrub(bool value);

    std::shared_ptr<FreeCamera> freeCamera();

    void setFreeCamera(std::shared_ptr<FreeCamera> value);

    std::optional<pxr::SdfPath> cameraPath();

    void setCameraPath(std::optional<pxr::SdfPath> value);

    std::optional<pxr::UsdPrim> cameraPrim();

    void setCameraPrim(std::optional<pxr::UsdPrim> value);

    [[nodiscard]] int fontSize() const;

    void setFontSize(int value);

private:
    RootDataModel &_rootDataModel;
    pxr::GfVec4f _cameraMaskColor{};
    pxr::GfVec4f _cameraReticlesColor{};
    float _defaultMaterialAmbient;
    float _defaultMaterialSpecular;
    bool _redrawOnScrub;
    RenderModes _renderMode;
    float _freeCameraFOV;
    float _freeCameraAspect;
    // For freeCameraOverrideNear/Far, Use -inf as a sentinel value to mean
    // None. (We cannot use None directly because that would cause a type-
    // checking error in Settings.)
    float _clippingPlaneNoneValue;
    std::optional<float> _freeCameraOverrideNear;
    std::optional<float> _freeCameraOverrideFar;

    bool _lockFreeCameraAspect;
    ColorCorrectionModes _colorCorrectionMode;
    OCIOSettings _ocioSettings;
    PickModes _pickMode;

    // We need to store the trinary selHighlightMode state here,
    // because the stageView only deals in True/False (because it
    // cannot know anything about playback state).
    SelectionHighlightModes _selHighlightMode;

    // We store the highlightColorName so that we can compare state during
    // initialization without inverting the name->value logic
    HighlightColors _highlightColorName;
    bool _ambientLightOnly;
    bool _domeLightEnabled;
    bool _domeLightTexturesVisible;
    ClearColors _clearColorText;
    bool _autoComputeClippingPlanes;
    bool _showBBoxPlayback;
    bool _showBBoxes;
    bool _showAABBox;
    bool _showOBBox;
    bool _displayGuide;
    bool _displayProxy;
    bool _displayRender;
    bool _displayPrimId;
    bool _enableSceneMaterials;
    bool _enableSceneLights;
    bool _cullBackfaces;
    bool _showInactivePrims;

    bool showAllMasterPrims;
    bool _showAllPrototypePrims;

    bool _showUndefinedPrims;
    bool _showAbstractPrims;
    bool _showPrimDisplayNames;
    bool _rolloverPrimInfo;
    bool _displayCameraOracles;
    CameraMaskModes _cameraMaskMode;
    bool _showMask_Outline;
    bool _showReticles_Inside;
    bool _showReticles_Outside;
    bool _showHUD;

    bool _showHUD_Info;

    bool _showHUD_Complexity;
    bool _showHUD_Performance;
    bool _showHUD_GPUstats;

    RefinementComplexities _complexity = RefinementComplexities::LOW;
    std::shared_ptr<FreeCamera> _freeCamera{};
    std::optional<pxr::SdfPath> _cameraPath{};
    int _fontSize;

    /// Needed when updating any camera setting (including movements). Will not
    /// update the property viewer.
    void _frustumChanged();

    /// Needed when updating specific camera settings (e.g., aperture). See
    /// _updateFreeCameraData for the full list of dependent settings. Will
    /// update the property viewer.
    void _frustumSettingsChanged();

    /// Updates member variables with the current free camera view settings.
    void _updateFreeCameraData();

    void _visibleViewSetting();

    void _invisibleViewSetting();

    void _freeCameraViewSetting();

    static pxr::GfVec4f _to_colors(ClearColors value);

    static pxr::GfVec4f _to_colors(HighlightColors value);
};