//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "common.h"

std::string to_constants(ClearColors value) {
    switch (value) {
        case ClearColors::BLACK: return "Black";
        case ClearColors::DARK_GREY: return "Grey (Dark)";
        case ClearColors::LIGHT_GREY: return "Grey (Light)";
        case ClearColors::WHITE: return "White";
    }
}

std::string to_constants(DefaultFontFamily value) {
    switch (value) {
        case DefaultFontFamily::FONT_FAMILY: return "Roboto";
        case DefaultFontFamily::MONOSPACE_FONT_FAMILY: return "Roboto Mono";
    }
}

std::string to_constants(HighlightColors value) {
    switch (value) {
        case HighlightColors::WHITE: return "White";
        case HighlightColors::YELLOW: return "Yellow";
        case HighlightColors::CYAN: return "Cyan";
    }
}

QBrush to_constants(UIBaseColors value) {
    switch (value) {

        case UIBaseColors::RED: return QBrush(QColor(230, 132, 131));
        case UIBaseColors::LIGHT_SKY_BLUE: return QBrush(QColor(135, 206, 250)); ;
        case UIBaseColors::DARK_YELLOW: return QBrush(QColor(222, 158, 46));
    }
}

QBrush to_constants(UIPrimTypeColors value) {
    switch (value) {
        case UIPrimTypeColors::HAS_ARCS: return to_constants(UIBaseColors::DARK_YELLOW);
        case UIPrimTypeColors::NORMAL: return QBrush(QColor(227, 227, 227));
        case UIPrimTypeColors::INSTANCE: return to_constants(UIBaseColors::LIGHT_SKY_BLUE);
        case UIPrimTypeColors::PROTOTYPE: return QBrush(QColor(118, 136, 217));
    }
}

QBrush to_constants(UIPrimTreeColors value) {
    switch (value) {
        case UIPrimTreeColors::SELECTED: return QBrush(QColor(189, 155, 84));
        case UIPrimTreeColors::SELECTED_HOVER: return QBrush(QColor(227, 186, 101));
        case UIPrimTreeColors::ANCESTOR_OF_SELECTED: return QBrush(QColor(189, 155, 84, 50));
        case UIPrimTreeColors::ANCESTOR_OF_SELECTED_HOVER: return QBrush(QColor(189, 155, 84, 100));
        case UIPrimTreeColors::UNSELECTED_HOVER: return QBrush(QColor(70, 70, 70));
    }
}

QBrush to_constants(UIPropertyValueSourceColors value) {
    switch (value) {
        case UIPropertyValueSourceColors::FALLBACK: return to_constants(UIBaseColors::DARK_YELLOW);
        case UIPropertyValueSourceColors::TIME_SAMPLE: return QBrush(QColor(177, 207, 153));
        case UIPropertyValueSourceColors::DEFAULT: return to_constants(UIBaseColors::LIGHT_SKY_BLUE);
        case UIPropertyValueSourceColors::NONE: return QBrush(QColor(140, 140, 140));
        case UIPropertyValueSourceColors::VALUE_CLIPS: return QBrush(QColor(230, 150, 230));
    }
}

QFont to_constants(UIFonts value) {
    constexpr int BASE_POINT_SIZE = 10;
    switch (value) {
        case UIFonts::OVER_PRIM:
        case UIFonts::ITALIC: {
            auto ITALIC = QFont();
            ITALIC.setWeight(QFont::Light);
            ITALIC.setItalic(true);
            return ITALIC;
        }
        case UIFonts::ABSTRACT_PRIM:
        case UIFonts::NORMAL: {
            auto NORMAL = QFont();
            NORMAL.setWeight(QFont::Normal);
        }
        case UIFonts::DEFINED_PRIM:
        case UIFonts::BOLD: {
            auto BOLD = QFont();
            BOLD.setWeight(QFont::Bold);
        }
        case UIFonts::BOLD_ITALIC: {
            auto BOLD_ITALIC = QFont();
            BOLD_ITALIC.setWeight(QFont::Bold);
            BOLD_ITALIC.setItalic(true);
        }
        case UIFonts::INHERITED: {
            auto INHERITED = QFont();
            INHERITED.setPointSize(BASE_POINT_SIZE * 0.8);
            INHERITED.setWeight(QFont::Normal);
            INHERITED.setItalic(true);
        }
    }
}

Qt::Key to_constants(KeyboardShortcuts value) {
    switch (value) {
        case KeyboardShortcuts::FramingKey: return Qt::Key_F;
    }
}

std::string to_constants(PropertyViewDataRoles value) {
    switch (value) {
        case PropertyViewDataRoles::ATTRIBUTE: return "Attr";
        case PropertyViewDataRoles::RELATIONSHIP: return "Rel";
        case PropertyViewDataRoles::ATTRIBUTE_WITH_CONNNECTIONS: return "Attr_";
        case PropertyViewDataRoles::RELATIONSHIP_WITH_TARGETS: return "Rel_";
        case PropertyViewDataRoles::TARGET: return "Tgt";
        case PropertyViewDataRoles::CONNECTION: return "Conn";
        case PropertyViewDataRoles::COMPOSED: return "Cmp";
    }
}

std::string to_constants(RenderModes value) {
    switch (value) {
        case RenderModes::WIREFRAME: return "Wireframe";
        case RenderModes::WIREFRAME_ON_SURFACE: return "WireframeOnSurface";
        case RenderModes::SMOOTH_SHADED: return "Smooth Shaded";
        case RenderModes::FLAT_SHADED: return "Flat Shaded";
        case RenderModes::POINTS: return "Points";
        case RenderModes::GEOM_ONLY: return "Geom Only";
        case RenderModes::GEOM_FLAT: return "Geom Flat";
        case RenderModes::GEOM_SMOOTH: return "Geom Smooth";
        case RenderModes::HIDDEN_SURFACE_WIREFRAME: return "Hidden Surface Wireframe";
    }
}

std::string to_constants(ShadedRenderModes value) {
    switch (value) {
        case ShadedRenderModes::SMOOTH_SHADED: return to_constants(RenderModes::SMOOTH_SHADED);
        case ShadedRenderModes::FLAT_SHADED: return to_constants(RenderModes::FLAT_SHADED);
        case ShadedRenderModes::WIREFRAME_ON_SURFACE: return to_constants(RenderModes::WIREFRAME_ON_SURFACE);
        case ShadedRenderModes::GEOM_FLAT: return to_constants(RenderModes::GEOM_FLAT);
        case ShadedRenderModes::GEOM_SMOOTH: return to_constants(RenderModes::GEOM_SMOOTH);
    }
}

std::string to_constants(ColorCorrectionModes value) {
    switch (value) {
        case ColorCorrectionModes::DISABLED: return "disabled";
        case ColorCorrectionModes::SRGB: return "sRGB";
        case ColorCorrectionModes::OPENCOLORIO: return "openColorIO";
    }
}

std::string to_constants(PickModes value) {
    switch (value) {
        case PickModes::PRIMS: return "Select Prims";
        case PickModes::MODELS: return "Select Models";
        case PickModes::INSTANCES: return "Select Instances";
        case PickModes::PROTOTYPES: return "Select Prototypes";
    }
}

std::string to_constants(SelectionHighlightModes value) {
    switch (value) {
        case SelectionHighlightModes::NEVER: return "Never";
        case SelectionHighlightModes::ONLY_WHEN_PAUSED: return "Only when paused";
        case SelectionHighlightModes::ALWAYS: return "Always";
    }
}

std::string to_constants(CameraMaskModes value) {
    switch (value) {
        case CameraMaskModes::NONE: return "none";
        case CameraMaskModes::PARTIAL: return "partial";
        case CameraMaskModes::FULL: return "full";
    }
}

pxr::TfToken to_constants(IncludedPurposes value) {
    switch (value) {
        case IncludedPurposes::DEFAULT: return pxr::UsdGeomTokensType().default_;
        case IncludedPurposes::PROXY: return pxr::UsdGeomTokensType().proxy;
        case IncludedPurposes::GUIDE: return pxr::UsdGeomTokensType().guide;
        case IncludedPurposes::RENDER: return pxr::UsdGeomTokensType().render;
    }
}

void propTreeWidgetGetRole() {}

void propTreeWidgetTypeIsRel() {}

void updateLabelText() {}

void ItalicizeLabelText() {}

void BoldenLabelText() {}

void ColorizeLabelText() {}

void PrintWarning() {}

void GetValueAndDisplayString() {}

void GetShortStringForValue() {}

void ReportMetricSize() {}

void getAttributeStatus() {}

void GetPropertyTextFont() {}

void GetPropertyColor() {}

void LayerInfo::FromLayer() {}

void LayerInfo::FromMutedLayerIdentifier() {}

void LayerInfo::GetIdentifier() {}

void LayerInfo::GetRealPath() {}

void LayerInfo::IsMuted() {}

void LayerInfo::GetOffset() {}

void LayerInfo::GetOffsetString() {}

void LayerInfo::GetOffsetTooltipString() {}

void LayerInfo::GetToolTipString() {}

void LayerInfo::GetHierarchicalDisplayString() {}

void addLayerTree() {}

void addLayerTreeWithMutedSubLayers() {}

void GetRootLayerStackInfo() {}

void PrettyFormatSize() {}

void Timer::Invalidate() {}

void Timer::PrintTime() {}

void InvisRootPrims() {}

void removeVisibilityRecursive() {}

void ResetSessionVisibility() {}

void HasSessionVis() {}

void GetEnclosingModelPrim() {}

void GetPrimLoadability() {}

void GetPrimsLoadability() {}

void GetFileOwner() {}

void GetAssetCreationTime() {}

void DumpMallocTags() {}

void GetInstanceIdForIndex() {}

void GetInstanceIndicesForIds() {}

void Drange() {}