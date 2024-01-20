//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QtCore/QObject>
#include <unordered_set>
#include <pxr/usd/sdf/path.h>
#include "root_data_model.h"
#include "custom_attributes.h"

static constexpr int ALL_INSTANCES = -1;

/// This class keeps track of the core data for prim selection: paths and
//    instances. The methods here can be called in any order required without
//    corrupting the path selection state.
class PrimSelection {
public:
    /// Clear the path selection.
    void clear();

    /// Remove any paths that pass the given predicate
    void removeMatchingPaths(const std::function<bool(const pxr::SdfPath &path)> &matches);

    /// Add a path to the selection. If an instance is given, then only add
    /// that instance. If all instances are selected when this happens then the
    /// single instance will become the only selected one.
    void addPrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Remove a path from the selection. If an instance is given, then only
    //  remove that instance. If all instances are selected when this happens,
    //  deselect all instances. If the target does not exist in the selection,
    //  do nothing.
    void removePrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Toggle the selection of a path. If an instance is given, only toggle
    ///  that instance's selection.
    void togglePrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Get a list of paths that are at least partially selected.
    std::vector<pxr::SdfPath> getPrimPaths();

    /// Get the full selection of paths and their corresponding selected
    ///  instances.
    void getPrimPathInstances();

    /// Get the prims added to or removed from the selection since the last
    ///  time getDiff() was called.
    void getDiff();

private:
    std::map<pxr::SdfPath, int> _selection;
    std::set<pxr::SdfPath> _added;
    std::set<pxr::SdfPath> _removed;

    /// Clears a path from the selection and updates the diff.
    void _clearPrimPath(const pxr::SdfPath &path);

    /// Discards an instance from the selection, then deletes the path from
    /// the selection if it has no more instances.
    void _discardInstance(const pxr::SdfPath &path, int instance);

    /// Returns True if all instances of a specified path are selected and
    ///  False otherwise.
    void _allInstancesSelected(const pxr::SdfPath &path);

    /// Returns True if all instances of a specified path are selected and
    ///  False otherwise.
    void _noInstancesSelected(const pxr::SdfPath &path);
};

class PropSelection {
public:
    /// Clears the property selection.
    void clear();

    /// Add a property to the selection.
    void addPropPath(const pxr::SdfPath &primPath, const std::string &propName);

    /// Remove a property from the selection.
    void removePropPath(const pxr::SdfPath &primPath, const std::string &propName);

    /// Add a target to the selection. Also add the target's property if it
    ///  is not already in the selection.
    void addTarget(const pxr::SdfPath &primPath, const std::string &propName, const pxr::SdfPath &target);

    /// Remove a target from the selection. If the target or its property are
    ///  not already in the selection, nothing is changed.
    void removeTarget(const pxr::SdfPath &primPath, const std::string &propName, const pxr::SdfPath &target);

    /// Get the list of properties.
    void getPropPaths();

    /// Get a dictionary which maps selected properties to a set of their
    /// selected targets or connections.
    void getTargets();

private:
    std::map<int, int> _selection;
};

/// Data model managing the current selection of prims and properties.
//  Please note that the owner of an instance of this class is
//  responsible for calling SelectionDataModel.removeUnpopulatedPrims() when
//  appropriate, lest methods like getPrims() return invalid prims.
class SelectionDataModel : QObject {
    Q_OBJECT;
signals:
    void signalPrimSelectionChanged();

    void signalPropSelectionChanged();

    void signalComputedPropSelectionChanged();

public:
    explicit SelectionDataModel(RootDataModel &rootDataModel);

#pragma region General Operations
    /// Clear all selections.
    void clear();

    void clearPoint();

    void setPoint(const pxr::GfVec3f &point);

    pxr::GfVec3f &getPoint();
#pragma endregion

#pragma region Prim Path Operations
    /// Clear the prim selection (same as path selection).
    void clearPrims();

    /// Add a path to the path selection. If an instance is given, only add that instance.
    void addPrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Remove a path from the path selection. If an instance is given, only
    /// remove that instance. If the target does not exist in the selection, do
    /// nothing.
    void removePrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Toggle a path in the path selection. If an instance is given, only
    //  that instance is toggled.
    void togglePrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Clear the prim selection then add a single prim path back to the
    //  selection. If an instance is given, only add that instance.
    void setPrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);

    /// Get the path currently in focus.
    pxr::SdfPath getFocusPrimPath();

    /// Get a list of all selected paths.
    std::vector<pxr::SdfPath> getPrimPaths();

    /// Get a list of paths from the selection who do not have an ancestor
    //  that is also in the selection. The "Least Common Denominator" paths.
    std::vector<pxr::SdfPath> &getLCDPaths();

    /// Get a dictionary which maps each selected prim to a set of its
    //  selected instances. If all of a path's instances are selected, the value
    //  is ALL_INSTANCES rather than a set.
    void getPrimPathInstances();

    /// Select only the given prim path. If only a single prim was selected
    //  before and all selected properties belong to this prim, select the
    //  corresponding properties on the new prim instead. If an instance is
    //  given, only select that instance.
    void switchToPrimPath(const pxr::SdfPath &path, int instance = ALL_INSTANCES);
#pragma endregion

#pragma region Prim Operations
    /// Add a prim's path to the path selection. If an instance is given, only add that instance.
    void addPrim(const pxr::UsdPrim &prim, int instance = ALL_INSTANCES);

    /// Remove a prim from the prim selection. If an instance is given, only
    /// remove that instance. If the target does not exist in the selection, do nothing.
    void removePrim(const pxr::UsdPrim &prim, int instance = ALL_INSTANCES);

    /// Toggle a prim's path in the path selection. If an instance is given,
    //  only that instance is toggled.
    void togglePrim(const pxr::UsdPrim &prim, int instance = ALL_INSTANCES);

    /// Clear the prim selection then add a single prim back to the
    //  selection. If an instance is given, only add that instance.
    void setPrim(const pxr::UsdPrim &prim, int instance = ALL_INSTANCES);

    /// Get the prim whose path is currently in focus.
    pxr::UsdPrim getFocusPrim();

    /// Get a list of all prims whose paths are selected.
    std::vector<pxr::UsdPrim> getPrims();

    /// Get a list of prims whose paths are both selected and do not have an
    //  ancestor that is also in the selection. The "Least Common Denominator" prims.
    std::vector<pxr::UsdPrim> getLCDPrims();

    /// Get a dictionary which maps each prim whose path is selected to a set
    //  of its selected instances. If all of a path's instances are selected,
    //  the value is ALL_INSTANCES rather than a set.
    void getPrimInstances();

    /// Select only the given prim. If only a single prim was selected before
    //  and all selected properties belong to this prim, select the
    //  corresponding properties on the new prim instead.
    void switchToPrim(const pxr::UsdPrim &prim, int instance = ALL_INSTANCES);
#pragma endregion

#pragma region Prim Group Removal Operations
    /// Remove all inactive prims
    void removeInactivePrims();

    /// Remove all prototype prims
    void removePrototypePrims();

    /// Remove all abstract prims
    void removeAbstractPrims();

    /// Remove all undefined prims
    void removeUndefinedPrims();

    /// Remove all prim paths whose corresponding prims do not currently
    //  exist on the stage.  It is the application's responsibility to
    //  call this method while it is processing changes to the stage,
    //  *before* querying this object for selections.  Because this is a
    //  synchronization operation rather than an expression of GUI state
    //  change, it does *not* perform any notifications/signals, which could
    //  cause reentrant application change processing.
    void removeUnpopulatedPrims();
#pragma endregion

#pragma region Property Path Operations
    /// Clear the property selection.
    void clearProps();

    /// Add a property to the selection.
    void addPropPath(const pxr::SdfPath &path);

    /// Remove a property from the selection.
    void removePropPath(const pxr::SdfPath &path);

    /// Clear the property selection, then add a single property path back to the selection.
    void setPropPath(const pxr::SdfPath &path);

    /// Select a property's target or connection.
    void addPropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath);

    /// Deselect a property's target or connection.
    void removePropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath);

    /// Clear the property selection, then add a single property path back to the selection with a target.
    void setPropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath);

    /// Get the focus property from the property selection.
    void getFocusPropPath();

    /// Get a list of all selected properties.
    void getPropPaths();

    /// Get a dictionary which maps selected properties to a set of their
    //  selected targets or connections.
    void getPropTargetPaths();
#pragma endregion

#pragma region Property Operations
    /// Add a property to the selection.
    void addProp(const pxr::UsdProperty &prop);

    /// Remove a property from the selection.
    void removeProp(const pxr::UsdProperty &prop);

    /// Clear the property selection, then add a single property back to the selection.
    void setProp(const pxr::UsdProperty &prop);

    /// Select a property's target or connection.
    void addPropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target);

    /// Deselect a property's target or connection.
    void removePropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target);

    /// Clear the property selection, then add a single property back to the selection with a target.
    void setPropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target);

    /// Get the focus property from the property selection.
    void getFocusProp();

    /// Get a list of all selected properties.
    void getProps();

    /// Get a dictionary which maps selected properties to a set of their selected targets or connections.
    void getPropTargets();
#pragma endregion

#pragma region Computed Property Path Operations
    /// Clear the computed property selection.
    void clearComputedProps();

    /// Add a computed property to the selection.
    void addComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName);

    /// Remove a computed property from the selection.
    void removeComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName);

    /// Clear the computed property selection, then add a single computed
    //  property path back to the selection.
    void setComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName);

    /// Get the focus computed property from the property selection.
    void getFocusComputedPropPath();

    /// Get a list of all selected computed properties.
    void getComputedPropPaths();
#pragma endregion

#pragma region Computed Property Operations
    /// Add a computed property to the selection.
    void addComputedProp(const pxr::UsdProperty &prop);

    /// Remove a computed property from the selection.
    void removeComputedProp(const pxr::UsdProperty &prop);

    /// Clear the computed property selection, then add a single computed
    //  property back to the selection.
    void setComputedProp(const pxr::UsdProperty &prop);

    /// Get the focus computed property from the property selection.
    void getFocusComputedProp();

    /// Get a list of all selected computed properties.
    void getComputedProps();
#pragma endregion

private:
    RootDataModel &_rootDataModel;
    ComputedPropertyFactory _computedPropFactory;
    //    Blocker batchPrimChanges;
    //    Blocker batchPropChanges;
    //    Blocker batchComputedPropChanges;
    pxr::GfVec3f _pointSelection{};
    std::vector<pxr::SdfPath> _lcdPathSelection;
    PrimSelection _primSelection;
    PropSelection _propSelection;
    PropSelection _computedPropSelection;

#pragma region Internal Operations
    /// Should be called whenever a change is made to _primSelection. Some
    //  final work is done then the prim selection changed signal is emitted.
    void _primSelectionChanged(bool value = true);

    /// Should be called whenever a change is made to _propSelection
    void _propSelectionChanged();

    /// Should be called whenever a change is made to _computedPropSelection.
    void _computedPropSelectionChanged();

    /// Validate an input path. If it is a string path, convert it to an Sdf.Path object.
    static void _ensureValidPrimPath(const pxr::SdfPath &path);

    /// Validate an instance used as a parameter. This can be any positive int or ALL_INSTANCES.
    void _validateInstanceIndexParameter(int instance);

    /// Validate a property.
    void _ensureValidPropPath(const pxr::SdfPath &path);

    /// Validate a property target or connection.
    void _ensureValidTargetPath(const pxr::SdfPath &path);

    /// Get a Usd property object from a property path.
    pxr::UsdProperty _getPropFromPath(const pxr::SdfPath &path);

    /// Get the Usd object from a target path. It can be either a Usd prim or Usd property.
    pxr::UsdPrim _getTargetFromPath(const pxr::SdfPath &path);

    /// Raise an error if we are currently batching prim selection changes.
    /// We don't want to allow reading prim selection state in the middle of a
    /// batch.
    void _requireNotBatchingPrims();

    /// Raise an error if we are currently batching prop selection changes.
    //  We don't want to allow reading prop selection state in the middle of a
    //  batch.
    void _requireNotBatchingProps();

    /// Raise an error if we are currently batching prop selection changes.
    //  We don't want to allow reading prop selection state in the middle of a
    //  batch.
    void _requireNotBatchingComputedProps();

    /// Get a CustomAttribute object from a prim path and property name.
    //  Raise an error if the property name does not match any known
    //  CustomAttribute.
    std::shared_ptr<CustomAttribute> _getComputedPropFromPath(const pxr::SdfPath &path, const std::string &propName);

    /// Build a new property path from a prim path and a property name.
    static pxr::SdfPath _buildPropPath(const std::string &primPath, const std::string &propName);

    /// Validate a computed property name.
    void _validateComputedPropName(const std::string &propName);

    /// Switch all selected properties from one prim to another. Only do this
    //   if all properties currently belong to the "from" prim.
    void _switchProps();
#pragma endregion
};