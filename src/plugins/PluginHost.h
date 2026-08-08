#pragma once
#include "PluginScanner.h"
#include "../../include/FLTypes.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace FL {

// ---------------------------------------------------------------------------
// PluginInstance
// A thin handle over a loaded plugin.  In production this wraps the actual
// plugin object returned by the host's plugin loader.
// ---------------------------------------------------------------------------
struct PluginInstance {
    PluginId     id     = 0;
    PluginInfo   info;
    bool         active = true;

    // Process audio through this plugin.
    // input / output are interleaved stereo float buffers of `frames` frames.
    void process(const float* input, float* output, FrameCount frames);

    // Convenience: get/set a named parameter value in [0, 1].
    float  getParameter(uint32_t index) const;
    void   setParameter(uint32_t index, float value);

private:
    std::vector<float> m_params;  // cached parameter values
};

// ---------------------------------------------------------------------------
// PluginHost
// Owns a collection of loaded PluginInstances and drives their audio
// processing in insertion order.
// ---------------------------------------------------------------------------
class PluginHost {
public:
    PluginHost() = default;
    ~PluginHost() = default;

    PluginHost(const PluginHost&)            = delete;
    PluginHost& operator=(const PluginHost&) = delete;

    // Load a plugin from its PluginInfo descriptor.
    // Returns the assigned PluginId, or 0 on failure.
    PluginId loadPlugin(const PluginInfo& info);

    // Unload a plugin by ID.  No-op if the ID is unknown.
    void unloadPlugin(PluginId id);

    // Retrieve a loaded plugin instance.  Returns nullptr if not found.
    PluginInstance* getInstance(PluginId id);
    const PluginInstance* getInstance(PluginId id) const;

    // Number of currently loaded plugins.
    size_t pluginCount() const noexcept { return m_instances.size(); }

    // Process all active plugins in order, chaining audio from one to the next.
    // inputBuffer and outputBuffer are interleaved stereo float (frames * 2).
    void processChain(const float* inputBuffer, float* outputBuffer, FrameCount frames);

    // Iterate over all loaded instances.
    using VisitorFn = std::function<void(PluginInstance&)>;
    void forEach(VisitorFn fn);

private:
    PluginId nextId() noexcept { return ++m_idCounter; }

    std::unordered_map<PluginId, std::unique_ptr<PluginInstance>> m_instances;
    std::vector<PluginId>  m_order;   // insertion order for chained processing
    PluginId               m_idCounter = 0;
};

} // namespace FL
