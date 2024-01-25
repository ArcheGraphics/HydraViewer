//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <pxr/usd/usdGeom/tokens.h>
#include <QtGui/QBrush>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFont>

/// Names of available background colors.
enum class ClearColors {
    BLACK,
    DARK_GREY,
    LIGHT_GREY,
    WHITE,

    Count
};
std::string to_constants(ClearColors value);

/// Names of the default font family and monospace font family to be used with usdview
enum class DefaultFontFamily {
    FONT_FAMILY,
    MONOSPACE_FONT_FAMILY
};
std::string to_constants(DefaultFontFamily value);

/// Names of available highlight colors for selected objects.
enum class HighlightColors {
    WHITE,
    YELLOW,
    CYAN,

    Count
};
std::string to_constants(HighlightColors value);

enum class UIBaseColors {
    RED,
    LIGHT_SKY_BLUE,
    DARK_YELLOW
};
QBrush to_constants(UIBaseColors value);

enum class UIPrimTypeColors {
    HAS_ARCS,
    NORMAL,
    INSTANCE,
    PROTOTYPE
};
QBrush to_constants(UIPrimTypeColors value);

enum class UIPrimTreeColors {
    SELECTED,
    SELECTED_HOVER,
    ANCESTOR_OF_SELECTED,
    ANCESTOR_OF_SELECTED_HOVER,
    UNSELECTED_HOVER
};
QBrush to_constants(UIPrimTreeColors value);

enum class UIPropertyValueSourceColors {
    FALLBACK,
    TIME_SAMPLE,
    DEFAULT,
    NONE,
    VALUE_CLIPS
};
QBrush to_constants(UIPropertyValueSourceColors value);

enum class UIFonts {
    ITALIC,
    NORMAL,
    BOLD,
    BOLD_ITALIC,
    OVER_PRIM,
    DEFINED_PRIM,
    ABSTRACT_PRIM,
    INHERITED
};
QFont to_constants(UIFonts value);

enum class KeyboardShortcuts {
    FramingKey
};
Qt::Key to_constants(KeyboardShortcuts value);

enum class PropertyViewIndex {
    TYPE,
    NAME,
    VALUE
};

enum class PropertyViewIcons {
    ATTRIBUTE,
    ATTRIBUTE_WITH_CONNECTIONS,
    RELATIONSHIP,
    RELATIONSHIP_WITH_TARGETS,
    TARGET,
    CONNECTION,
    COMPOSED
};

enum class PropertyViewDataRoles {
    ATTRIBUTE,
    RELATIONSHIP,
    ATTRIBUTE_WITH_CONNNECTIONS,
    RELATIONSHIP_WITH_TARGETS,
    TARGET,
    CONNECTION,
    COMPOSED
};
std::string to_constants(PropertyViewDataRoles value);

enum class RenderModes: int {
    WIREFRAME,
    WIREFRAME_ON_SURFACE,
    SMOOTH_SHADED,
    FLAT_SHADED,
    POINTS,
    GEOM_ONLY,
    GEOM_FLAT,
    GEOM_SMOOTH,
    HIDDEN_SURFACE_WIREFRAME,

    Count
};
std::string to_constants(RenderModes value);

enum class ShadedRenderModes {
    SMOOTH_SHADED,
    FLAT_SHADED,
    WIREFRAME_ON_SURFACE,
    GEOM_FLAT,
    GEOM_SMOOTH
};
std::string to_constants(ShadedRenderModes value);

/// Color correction used when render is presented to screen
/// These strings should match HdxColorCorrectionTokens
enum class ColorCorrectionModes {
    DISABLED,
    SRGB,
    OPENCOLORIO,

    Count
};
std::string to_constants(ColorCorrectionModes value);

enum class PickModes {
    PRIMS,
    MODELS,
    INSTANCES,
    PROTOTYPES,

    Count
};
std::string to_constants(PickModes value);

enum class SelectionHighlightModes {
    NEVER,
    ONLY_WHEN_PAUSED,
    ALWAYS,

    Count
};
std::string to_constants(SelectionHighlightModes value);

enum class CameraMaskModes {
    NONE,
    PARTIAL,
    FULL,

    Count
};
std::string to_constants(CameraMaskModes value);

enum class IncludedPurposes {
    DEFAULT,
    PROXY,
    GUIDE,
    RENDER
};
pxr::TfToken to_constants(IncludedPurposes value);

void propTreeWidgetGetRole();

void propTreeWidgetTypeIsRel();

void updateLabelText();

void ItalicizeLabelText();

void BoldenLabelText();

void ColorizeLabelText();

void PrintWarning();

void GetValueAndDisplayString();

void GetShortStringForValue();

void ReportMetricSize();

void getAttributeStatus();

void GetPropertyTextFont();

void GetPropertyColor();

class LayerInfo {
public:
    void FromLayer();

    void FromMutedLayerIdentifier();

    void GetIdentifier();

    void GetRealPath();

    void IsMuted();

    void GetOffset();

    void GetOffsetString();

    void GetOffsetTooltipString();

    void GetToolTipString();

    void GetHierarchicalDisplayString();
};

void addLayerTree();

void addLayerTreeWithMutedSubLayers();

void GetRootLayerStackInfo();

void PrettyFormatSize();

class Timer {
public:
    void Invalidate();

    void PrintTime();
};

class BusyContext {
public:
};

void InvisRootPrims();

void removeVisibilityRecursive();

void ResetSessionVisibility();

void HasSessionVis();

void GetEnclosingModelPrim();

void GetPrimLoadability();

void GetPrimsLoadability();

void GetFileOwner();

void GetAssetCreationTime();

void DumpMallocTags();

void GetInstanceIdForIndex();

void GetInstanceIndicesForIds();

void Drange();

class PrimNotFoundException : public std::exception {
public:
};

class PropertyNotFoundException : public std::exception {
public:
};

class FixableDoubleValidator : public QDoubleValidator {
public:
    void fixup(QString &input) const override {}
};