//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "selection_data_model.h"

void PrimSelection::clear() {
    for (const auto &selection : _selection) {
        _clearPrimPath(selection.first);
    }
}

void PrimSelection::removeMatchingPaths(const std::function<bool(const pxr::SdfPath &path)> &matches) {
    for (const auto &selection : _selection) {
        if (matches(selection.first)) {
            _clearPrimPath(selection.first);
        }
    }
}

void PrimSelection::addPrimPath(const pxr::SdfPath &path, int instance) {}

void PrimSelection::removePrimPath(const pxr::SdfPath &path, int instance) {}

void PrimSelection::togglePrimPath(const pxr::SdfPath &path, int instance) {}

std::vector<pxr::SdfPath> PrimSelection::getPrimPaths() {
    std::vector<pxr::SdfPath> paths;
    for (auto &selection : _selection) {
        paths.push_back(selection.first);
    }
    return paths;
}

void PrimSelection::getPrimPathInstances() {}

void PrimSelection::getDiff() {}

void PrimSelection::_clearPrimPath(const pxr::SdfPath &path) {}

void PrimSelection::_discardInstance(const pxr::SdfPath &path, int instance) {}

void PrimSelection::_allInstancesSelected(const pxr::SdfPath &path) {}

void PrimSelection::_noInstancesSelected(const pxr::SdfPath &path) {}

void PropSelection::clear() {}

void PropSelection::addPropPath(const pxr::SdfPath &primPath, const std::string &propName) {}

void PropSelection::removePropPath(const pxr::SdfPath &primPath, const std::string &propName) {}

void PropSelection::addTarget(const pxr::SdfPath &primPath, const std::string &propName, const pxr::SdfPath &target) {}

void PropSelection::removeTarget(const pxr::SdfPath &primPath, const std::string &propName, const pxr::SdfPath &target) {}

void PropSelection::getPropPaths() {}

void PropSelection::getTargets() {}

SelectionDataModel::SelectionDataModel(RootDataModel &rootDataModel)
    : _rootDataModel(rootDataModel),
      _computedPropFactory{ComputedPropertyFactory(_rootDataModel)} {
    _lcdPathSelection.push_back(pxr::SdfPath::AbsoluteRootPath());
}

void SelectionDataModel::clear() {
    clearPoint();
    clearPrims();
    clearProps();
}

void SelectionDataModel::clearPoint() {
    setPoint(pxr::GfVec3f());
}

void SelectionDataModel::setPoint(const pxr::GfVec3f &point) {
    _pointSelection = point;
}

pxr::GfVec3f &SelectionDataModel::getPoint() {
    return _pointSelection;
}

void SelectionDataModel::clearPrims() {
    _primSelection.clear();
    _primSelectionChanged();
}

void SelectionDataModel::addPrimPath(const pxr::SdfPath &path, int instance) {
    _ensureValidPrimPath(path);
    _validateInstanceIndexParameter(instance);

    _primSelection.addPrimPath(path, instance);
    _primSelectionChanged();
}

void SelectionDataModel::removePrimPath(const pxr::SdfPath &path, int instance) {
    _ensureValidPrimPath(path);
    _validateInstanceIndexParameter(instance);

    _primSelection.removePrimPath(path, instance);
    _primSelectionChanged();
}

void SelectionDataModel::togglePrimPath(const pxr::SdfPath &path, int instance) {
    _ensureValidPrimPath(path);
    _validateInstanceIndexParameter(instance);

    _primSelection.togglePrimPath(path, instance);
    _primSelectionChanged();
}

void SelectionDataModel::setPrimPath(const pxr::SdfPath &path, int instance) {
    clearPrims();
    addPrimPath(path, instance);
}

pxr::SdfPath SelectionDataModel::getFocusPrimPath() {
    _requireNotBatchingPrims();
    return _primSelection.getPrimPaths()[0];
}

std::vector<pxr::SdfPath> SelectionDataModel::getPrimPaths() {
    _requireNotBatchingPrims();
    return _primSelection.getPrimPaths();
}

std::vector<pxr::SdfPath> &SelectionDataModel::getLCDPaths() {
    _requireNotBatchingPrims();
    return _lcdPathSelection;
}

void SelectionDataModel::getPrimPathInstances() {
    _requireNotBatchingPrims();
    return _primSelection.getPrimPathInstances();
}

void SelectionDataModel::switchToPrimPath(const pxr::SdfPath &path, int instance) {
    //todo
}

void SelectionDataModel::addPrim(const pxr::UsdPrim &prim, int instance) {
    addPrimPath(prim.GetPath(), instance);
}

void SelectionDataModel::removePrim(const pxr::UsdPrim &prim, int instance) {
    removePrimPath(prim.GetPath(), instance);
}

void SelectionDataModel::togglePrim(const pxr::UsdPrim &prim, int instance) {
    togglePrimPath(prim.GetPath(), instance);
}

void SelectionDataModel::setPrim(const pxr::UsdPrim &prim, int instance) {
    setPrimPath(prim.GetPath(), instance);
}

pxr::UsdPrim SelectionDataModel::getFocusPrim() {
    return _rootDataModel.stage().value()->GetPrimAtPath(getFocusPrimPath());
}

std::vector<pxr::UsdPrim> SelectionDataModel::getPrims() {
    std::vector<pxr::UsdPrim> prims;
    for (const auto &path : getPrimPaths()) {
        prims.push_back(_rootDataModel.stage().value()->GetPrimAtPath(path));
    }

    return prims;
}

std::vector<pxr::UsdPrim> SelectionDataModel::getLCDPrims() {
    std::vector<pxr::UsdPrim> prims;
    for (const auto &path : getLCDPaths()) {
        prims.push_back(_rootDataModel.stage().value()->GetPrimAtPath(path));
    }

    return prims;
}

void SelectionDataModel::getPrimInstances() {
    // todo
}

void SelectionDataModel::switchToPrim(const pxr::UsdPrim &prim, int instance) {
    switchToPrimPath(prim.GetPath(), instance);
}

void SelectionDataModel::removeInactivePrims() {
    for (const auto &prim : getPrims()) {
        if (!prim.IsActive())
            removePrim(prim);
    }
}

void SelectionDataModel::removePrototypePrims() {
    for (const auto &prim : getPrims()) {
        if (prim.IsPrototype() || prim.IsInPrototype())
            removePrim(prim);
    }
}

void SelectionDataModel::removeAbstractPrims() {
    for (const auto &prim : getPrims()) {
        if (prim.IsAbstract())
            removePrim(prim);
    }
}

void SelectionDataModel::removeUndefinedPrims() {
    for (const auto &prim : getPrims()) {
        if (!prim.IsDefined())
            removePrim(prim);
    }
}

void SelectionDataModel::removeUnpopulatedPrims() {
    auto stage = _rootDataModel.stage().value();
    _primSelection.removeMatchingPaths([stage](const pxr::SdfPath &path) -> bool {
        return !stage->GetPrimAtPath(path);
    });
    _primSelectionChanged(false);
}

void SelectionDataModel::clearProps() {
    _propSelection.clear();
    _propSelectionChanged();
}

void SelectionDataModel::addPropPath(const pxr::SdfPath &path) {
    _ensureValidPropPath(path);

    auto primPath = path.GetPrimPath();
    const auto &propName = path.GetName();

    _propSelection.addPropPath(primPath, propName);
    _propSelectionChanged();
}

void SelectionDataModel::removePropPath(const pxr::SdfPath &path) {
    _ensureValidPropPath(path);

    auto primPath = path.GetPrimPath();
    const auto &propName = path.GetName();

    _propSelection.removePropPath(primPath, propName);
    _propSelectionChanged();
}

void SelectionDataModel::setPropPath(const pxr::SdfPath &path) {
    _ensureValidPropPath(path);

    clearProps();
    addPropPath(path);
}

void SelectionDataModel::addPropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath) {
    _ensureValidPropPath(path);
    _ensureValidTargetPath(targetPath);

    auto primPath = path.GetPrimPath();
    const auto &propName = path.GetName();

    _propSelection.addTarget(primPath, propName, targetPath);
    _propSelectionChanged();
}

void SelectionDataModel::removePropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath) {
    _ensureValidPropPath(path);
    _ensureValidTargetPath(targetPath);

    auto primPath = path.GetPrimPath();
    const auto &propName = path.GetName();

    _propSelection.removeTarget(primPath, propName, targetPath);
    _propSelectionChanged();
}

void SelectionDataModel::setPropTargetPath(const pxr::SdfPath &path, const pxr::SdfPath &targetPath) {
    clearProps();
    addPropTargetPath(path, targetPath);
}

void SelectionDataModel::getFocusPropPath() {}

void SelectionDataModel::getPropPaths() {}

void SelectionDataModel::getPropTargetPaths() {}

void SelectionDataModel::addProp(const pxr::UsdProperty &prop) {
    addPropPath(prop.GetPath());
}

void SelectionDataModel::removeProp(const pxr::UsdProperty &prop) {
    removePropPath(prop.GetPath());
}

void SelectionDataModel::setProp(const pxr::UsdProperty &prop) {
    setPropPath(prop.GetPath());
}

void SelectionDataModel::addPropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target) {
    addPropTargetPath(prop.GetPath(), target.GetPath());
}

void SelectionDataModel::removePropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target) {
    removePropTargetPath(prop.GetPath(), target.GetPath());
}

void SelectionDataModel::setPropTarget(const pxr::UsdProperty &prop, const pxr::UsdProperty &target) {
    removePropTargetPath(prop.GetPath(), target.GetPath());
}

void SelectionDataModel::getFocusProp() {}

void SelectionDataModel::getProps() {}

void SelectionDataModel::getPropTargets() {}

void SelectionDataModel::clearComputedProps() {
    _computedPropSelection.clear();
    _computedPropSelectionChanged();
}

void SelectionDataModel::addComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName) {
    _ensureValidPrimPath(primPath);
    _validateComputedPropName(propName);

    _computedPropSelection.addPropPath(primPath, propName);
    _computedPropSelectionChanged();
}

void SelectionDataModel::removeComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName) {
    _ensureValidPrimPath(primPath);
    _validateComputedPropName(propName);

    _computedPropSelection.removePropPath(primPath, propName);
    _computedPropSelectionChanged();
}

void SelectionDataModel::setComputedPropPath(const pxr::SdfPath &primPath, const std::string &propName) {
    _ensureValidPrimPath(primPath);
    _validateComputedPropName(propName);

    clearComputedProps();
    addComputedPropPath(primPath, propName);
}

void SelectionDataModel::getFocusComputedPropPath() {}

void SelectionDataModel::getComputedPropPaths() {}

void SelectionDataModel::addComputedProp(const pxr::UsdProperty &prop) {
    addComputedPropPath(prop.GetPrimPath(), prop.GetName());
}

void SelectionDataModel::removeComputedProp(const pxr::UsdProperty &prop) {
    removeComputedPropPath(prop.GetPrimPath(), prop.GetName());
}

void SelectionDataModel::setComputedProp(const pxr::UsdProperty &prop) {
    setComputedPropPath(prop.GetPrimPath(), prop.GetName());
}

void SelectionDataModel::getFocusComputedProp() {}

void SelectionDataModel::getComputedProps() {}

void SelectionDataModel::_primSelectionChanged(bool value) {
    // If updates are suppressed, do not emit a signal or do any
    //  pre-processing.
    //        if batchPrimChanges.blocked():
    //            return
    //
    //  Make sure there is always at least one path selected.
    if (getPrimPaths().empty()) {
        _primSelection.addPrimPath(pxr::SdfPath::AbsoluteRootPath());
    }
    // Recalculate the LCD prims whenever the path selection changes.
    auto paths = _primSelection.getPrimPaths();
    if (paths.size() > 1) {
    }
    //            paths = [path for path in paths
    //                        if path != Sdf.Path.absoluteRootPath]
    pxr::SdfPath::RemoveDescendentPaths(&paths);

    //  Finally, emit the changed signal.
    //        added, removed = _primSelection.getDiff()
    //        if emitSelChangedSignal:
    //            signalPrimSelectionChanged.emit(added, removed)
}

void SelectionDataModel::_propSelectionChanged() {
    emit signalPropSelectionChanged();
}

void SelectionDataModel::_computedPropSelectionChanged() {
    emit signalComputedPropSelectionChanged();
}

void SelectionDataModel::_ensureValidPrimPath(const pxr::SdfPath &path) {
    if (path.IsAbsoluteRootOrPrimPath()) {
        // todo
    }
}

void SelectionDataModel::_validateInstanceIndexParameter(int instance) {
    auto validIndex = false;
    if (instance >= 0 || instance == ALL_INSTANCES) {
        validIndex = true;
    }

    if (!validIndex) {
        // todo
    }
}

void SelectionDataModel::_ensureValidPropPath(const pxr::SdfPath &path) {
    if (!path.IsPropertyPath()) {
        // todo
    }
}

void SelectionDataModel::_ensureValidTargetPath(const pxr::SdfPath &path) {
    if (!path.IsPrimPath() && !path.IsPropertyPath()) {
        // todo
    }
}

pxr::UsdProperty SelectionDataModel::_getPropFromPath(const pxr::SdfPath &path) {
    auto prim = _rootDataModel.stage().value()->GetPrimAtPath(path.GetPrimPath());
    return prim.GetProperty(path.GetNameToken());
}

pxr::UsdPrim SelectionDataModel::_getTargetFromPath(const pxr::SdfPath &path) {
    //    if (path.IsPropertyPath()) {
    //        return _getPropFromPath(path);
    //    } else {
    return _rootDataModel.stage().value()->GetPrimAtPath(path);
    //    }
}

void SelectionDataModel::_requireNotBatchingPrims() {}

void SelectionDataModel::_requireNotBatchingProps() {}

std::shared_ptr<CustomAttribute> SelectionDataModel::_getComputedPropFromPath(const pxr::SdfPath &primPath, const std::string &propName) {
    auto prim = _rootDataModel.stage().value()->GetPrimAtPath(primPath);
    return _computedPropFactory.getComputedProperty(prim, propName);
}

void SelectionDataModel::_requireNotBatchingComputedProps() {}

pxr::SdfPath SelectionDataModel::_buildPropPath(const std::string &primPath, const std::string &propName) {
    return pxr::SdfPath(primPath + "." + propName);
}

void SelectionDataModel::_validateComputedPropName(const std::string &propName) {}

void SelectionDataModel::_switchProps() {}