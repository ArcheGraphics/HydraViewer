//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include "root_data_model.h"
#include "../common.h"
#include "free_camera.h"
#include <QObject>

struct RefinementComplexities {
    explicit RefinementComplexities(float value);
    static const RefinementComplexities LOW;
    static const RefinementComplexities MEDIUM;
    static const RefinementComplexities HIGH;
    static const RefinementComplexities VERY_HIGH;

    [[nodiscard]] inline float value() const {
        return _value;
    }

private:
    float _value;
};

inline bool operator==(const RefinementComplexities &lhs, const RefinementComplexities &rhs) { return lhs.value() == rhs.value(); }

QString to_constants(RefinementComplexities value);

/// Class to hold OCIO display, view, and colorSpace config settings as strings.
class OCIOSettings {
public:
    explicit OCIOSettings(pxr::TfToken display = {}, pxr::TfToken view = {}, pxr::TfToken colorSpace = {});

    const pxr::TfToken &display();

    const pxr::TfToken &view();

    const pxr::TfToken &colorSpace();

private:
    pxr::TfToken _display;
    pxr::TfToken _view;
    pxr::TfToken _colorSpace;
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

    Q_PROPERTY(float defaultMaterialAmbient READ defaultMaterialAmbient WRITE setDefaultMaterialAmbient)
    [[nodiscard]] float defaultMaterialAmbient() const;
    void setDefaultMaterialAmbient(float val);

    Q_PROPERTY(float defaultMaterialSpecular READ defaultMaterialSpecular WRITE setDefaultMaterialSpecular)
    [[nodiscard]] float defaultMaterialSpecular() const;
    void setDefaultMaterialSpecular(float val);

    void setDefaultMaterial(float ambient, float specular);
    void resetDefaultMaterial();

    Q_PROPERTY(RefinementComplexities complexity READ complexity WRITE setComplexity)
    RefinementComplexities complexity();
    void setComplexity(RefinementComplexities value);

    Q_PROPERTY(RenderModes renderMode READ renderMode WRITE setRenderMode)
    RenderModes renderMode();
    void setRenderMode(RenderModes value);

    Q_PROPERTY(float freeCameraFOV READ freeCameraFOV WRITE setFreeCameraFOV)
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

    Q_PROPERTY(float freeCameraAspect READ freeCameraAspect WRITE setFreeCameraAspect)
    [[nodiscard]] float freeCameraAspect() const;
    void setFreeCameraAspect(float value);

    Q_PROPERTY(bool lockFreeCameraAspect READ lockFreeCameraAspect WRITE setLockFreeCameraAspect)
    [[nodiscard]] bool lockFreeCameraAspect() const;
    void setLockFreeCameraAspect(bool val);

    Q_PROPERTY(ColorCorrectionModes colorCorrectionMode READ colorCorrectionMode WRITE setColorCorrectionMode)
    ColorCorrectionModes colorCorrectionMode();
    void setColorCorrectionMode(ColorCorrectionModes value);

    Q_PROPERTY(OCIOSettings ocioSettings READ ocioSettings WRITE setOcioSettings)
    OCIOSettings &ocioSettings();
    /// Specifies the OCIO settings to be used. Setting the OCIO 'display'
    //  requires a 'view' to be specified.
    void setOcioSettings(OCIOSettings value);

    Q_PROPERTY(PickModes pickMode READ pickMode WRITE setPickMode)
    PickModes pickMode();
    void setPickMode(PickModes value);

    Q_PROPERTY(bool showAABBox READ showAABBox WRITE setShowAABBox)
    [[nodiscard]] bool showAABBox() const;
    void setShowAABBox(bool value);

    Q_PROPERTY(bool showOBBox READ showOBBox WRITE setShowOBBox)
    [[nodiscard]] bool showOBBox() const;
    void setShowOBBox(bool value);

    Q_PROPERTY(bool showBBoxes READ showBBoxes WRITE setShowBBoxes)
    [[nodiscard]] bool showBBoxes() const;
    void setShowBBoxes(bool value);

    Q_PROPERTY(bool autoComputeClippingPlanes READ autoComputeClippingPlanes WRITE setAutoComputeClippingPlanes)
    [[nodiscard]] bool autoComputeClippingPlanes() const;
    void setAutoComputeClippingPlanes(bool value);

    Q_PROPERTY(bool showBBoxPlayback READ showBBoxPlayback WRITE setShowBBoxPlayback)
    [[nodiscard]] bool showBBoxPlayback() const;
    void setShowBBoxPlayback(bool value);

    Q_PROPERTY(bool displayGuide READ displayGuide WRITE setDisplayGuide)
    [[nodiscard]] bool displayGuide() const;
    void setDisplayGuide(bool value);

    Q_PROPERTY(bool displayProxy READ displayProxy WRITE setDisplayProxy)
    [[nodiscard]] bool displayProxy() const;
    void setDisplayProxy(bool value);

    Q_PROPERTY(bool displayRender READ displayRender WRITE setDisplayRender)
    [[nodiscard]] bool displayRender() const;
    void setDisplayRender(bool value);

    Q_PROPERTY(bool displayCameraOracles READ displayCameraOracles WRITE setDisplayCameraOracles)
    [[nodiscard]] bool displayCameraOracles() const;
    void setDisplayCameraOracles(bool value);

    Q_PROPERTY(bool displayPrimId READ displayPrimId WRITE setDisplayPrimId)
    [[nodiscard]] bool displayPrimId() const;
    void setDisplayPrimId(bool value);

    Q_PROPERTY(bool enableSceneMaterials READ enableSceneMaterials WRITE setEnableSceneMaterials)
    [[nodiscard]] bool enableSceneMaterials() const;
    void setEnableSceneMaterials(bool value);

    Q_PROPERTY(bool enableSceneLights READ enableSceneLights WRITE setEnableSceneLights)
    [[nodiscard]] bool enableSceneLights() const;
    void setEnableSceneLights(bool value);

    Q_PROPERTY(bool cullBackfaces READ cullBackfaces WRITE setCullBackfaces)
    [[nodiscard]] bool cullBackfaces() const;
    void setCullBackfaces(bool value);

    Q_PROPERTY(bool showHUD READ showHUD WRITE setShowHUD)
    [[nodiscard]] bool showHUD() const;
    void setShowHUD(bool value);

    Q_PROPERTY(bool ambientLightOnly READ ambientLightOnly WRITE setAmbientLightOnly)
    [[nodiscard]] bool ambientLightOnly() const;
    void setAmbientLightOnly(bool value);

    Q_PROPERTY(bool domeLightEnabled READ domeLightEnabled WRITE setDomeLightEnabled)
    [[nodiscard]] bool domeLightEnabled() const;
    void setDomeLightEnabled(bool value);

    Q_PROPERTY(bool domeLightTexturesVisible READ domeLightTexturesVisible WRITE setDomeLightTexturesVisible)
    [[nodiscard]] bool domeLightTexturesVisible() const;
    void setDomeLightTexturesVisible(bool value);

    Q_PROPERTY(ClearColors clearColorText READ clearColorText WRITE setClearColorText)
    ClearColors clearColorText();
    void setClearColorText(ClearColors value);

    pxr::GfVec4f clearColor();

    Q_PROPERTY(HighlightColors highlightColorName READ highlightColorName WRITE setHighlightColorName)
    HighlightColors highlightColorName();
    void setHighlightColorName(HighlightColors value);

    pxr::GfVec4f highlightColor();

    Q_PROPERTY(SelectionHighlightModes selHighlightMode READ selHighlightMode WRITE setSelHighlightMode)
    SelectionHighlightModes selHighlightMode();
    void setSelHighlightMode(SelectionHighlightModes value);

    std::shared_ptr<FreeCamera> freeCamera();
    void setFreeCamera(std::shared_ptr<FreeCamera> value);

    std::optional<pxr::SdfPath> cameraPath();
    void setCameraPath(std::optional<pxr::SdfPath> value);

    std::optional<pxr::UsdPrim> cameraPrim();
    void setCameraPrim(std::optional<pxr::UsdPrim> value);

private:
    RootDataModel &_rootDataModel;
    float _defaultMaterialAmbient;
    float _defaultMaterialSpecular;
    RenderModes _renderMode;
    float _freeCameraFOV;
    float _freeCameraAspect;
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

    bool _displayCameraOracles;
    bool _showHUD;

    RefinementComplexities _complexity = RefinementComplexities::LOW;
    std::shared_ptr<FreeCamera> _freeCamera{};
    std::optional<pxr::SdfPath> _cameraPath{};

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