//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "settings.h"

#include <utility>

StateSource::StateSource(StateSource *parent, std::string name)
    : _parentStateSource{parent}, _stateSourceName{std::move(name)} {
    // Register child source with the parent.
    if (_parentStateSource)
        _parentStateSource->_registerChildStateSource(this);
}

StateSource *StateSource::GetChildStateSource(const std::string &childName) {
    return _childStateSources.at(childName);
}

void StateSource::_registerChildStateSource(StateSource *child) {
    _childStateSources.insert(std::make_pair(child->_stateSourceName, child));
}

nlohmann::json &StateSource::_getState() {
    if (!_parentStateSource) {
        if (!_state.has_value())
            _state = nlohmann::json();
        return _state.value();
    } else {
        return _parentStateSource->_getChildState(_stateSourceName);
    }
}

nlohmann::json &StateSource::_getChildState(const std::string &childName) {
    auto &state = _getState();

    auto iter = state.find(childName);
    if (iter != state.end()) {
        return *iter;
    }

    // Create a new state dict for the child and save it in this source's
    // state dict.
    nlohmann::json childState{};
    state[childName] = childState;
    return state[childName];
}

void StateSource::_saveState() {}

void Settings::_loadState() {}

nlohmann::json &Settings::_getState() {
    return _stateBuffer;
}

Settings::Settings(int version, std::string stateFilePath)
    : StateSource(nullptr, ""),
      _version(version),
      _stateFilePath(std::move(stateFilePath)),
      _isEphemeral{_stateFilePath.empty()} {
}

void Settings::save() {}

void Settings::onSaveState(nlohmann::json &state) {}

ConfigManager::ConfigManager(std::string configDirPath)
    : _configDirPath(std::move(configDirPath)) {
}

void ConfigManager::loadSettings(const std::string &config, int version, bool isEphemeral) {
    _saveOnClose = !isEphemeral && config == defaultConfig;
    settings = std::make_unique<Settings>(version, _configPaths[config]);
}

void ConfigManager::getConfigs() {}

void ConfigManager::save(const std::string &newName) {
    settings->_stateFilePath = _configDirPath + newName + "." + EXTENSION;
    _saveOnClose = (newName == defaultConfig);
    settings->save();
}

void ConfigManager::close() const {
    if (_saveOnClose) {
        settings->save();
    }
}

void ConfigManager::_loadConfigPaths() {}