//  Copyright (c) 2023 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

/// Defines a state property on a StateSource object.
template<typename PropType>
struct StateProp {
    std::string name;
    PropType defaultValue;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StateProp, name, defaultValue)
};

/// An object which has some savable application state.
class StateSource {
public:
    explicit StateSource(StateSource *parent, std::string name);

    /// Validates and creates a new StateProp for this source. The property's
    //  value is returned so this method can be used during StateSource
    //  initialization.
    template<typename PropType>
    PropType stateProperty(const std::string &name, PropType defaultValue) {
        StateProp<PropType> prop{name, defaultValue};
        _stateSourceProperties[name] = prop;

        // Load the value from the state dict and validate it.
        auto &state = _getState();
        auto iter = state.find(name);
        if (iter != state.end()) {
            return *iter;
        } else {
            return defaultValue;
        }
    }

    /// Returns the child StateSource corresponding to childName, or None
    StateSource *GetChildStateSource(const std::string &childName);

    /// Save the source's state properties to a dict.
    virtual void onSaveState(nlohmann::json &state) = 0;

protected:
    StateSource *_parentStateSource{};
    std::unordered_map<std::string, StateSource *> _childStateSources;
    std::optional<nlohmann::json> _state;

    std::string _stateSourceName;
    nlohmann::json _stateSourceProperties;

    /// Registers a child StateSource with this source object.
    void _registerChildStateSource(StateSource *child);

    /// Get this source's state dict from its parent source.
    virtual nlohmann::json &_getState();

    /// Get a child source's state dict. This method guarantees that a dict
    // will be return but does not guarantee anything about the contents of
    // the dict.
    nlohmann::json &_getChildState(const std::string &childName);

    /// Saves the source's state to the settings object's state buffer.
    void _saveState();
};

/// An object which encapsulates saving and loading of application state to
//  a state file. When created, it loads state from a state file and stores it
//  in a buffer. Its children sources can fetch their piece of state from the
//  buffer. On save, this object tells its children to save their current
//  states, then saves the buffer back to the state file.
class Settings : public StateSource {
public:
    explicit Settings(int version, std::string stateFilePath = "");

    /// Inform all children to save their states, then write the state buffer
    //  back to the state file.
    void save();

    /// Settings object has no state properties.
    void onSaveState(nlohmann::json &state) override;

private:
    friend class ConfigManager;
    int _version;
    std::string _stateFilePath;
    nlohmann::json _stateBuffer;
    nlohmann::json _versionsStateBuffer;
    bool _isEphemeral;

    /// Loads and returns application state from a state file. If the file is
    //  not found, contains invalid JSON, does not contain a dictionary, an
    //  empty state is returned instead.
    void _loadState();

    /// Gets the buffered state rather than asking its parent for its state.
    nlohmann::json &_getState() override;
};

/// Class used to manage, read, and write the different saved settings that
/// represent the usdview application's current state.
class ConfigManager {
public:
    /// Creates the manager instance.
    explicit ConfigManager(std::string configDirPath);

    /// Loads the specified config. We wait to do this instead of loading in
    /// init to allow the manager to be created and read the list of available
    /// configs without actually doing the more expensive settings loading.
    void loadSettings(const std::string &config, int version, bool isEphemeral = false);

    /// Gets the list of config names
    void getConfigs();

    /// Saves the current state to the specified config
    void save(const std::string &newName);

    /// Signal that application is closing
    void close() const;

    std::string EXTENSION = "state.json";
    std::string defaultConfig;
    std::unique_ptr<Settings> settings;

private:
    std::string _configDirPath;
    bool _saveOnClose{false};
    std::unordered_map<std::string, std::string> _configPaths;

    /// Private method to load the config names and associated paths
    void _loadConfigPaths();
};