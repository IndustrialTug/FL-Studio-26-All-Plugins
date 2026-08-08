#include "PluginHost.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace FL {

// ---------------------------------------------------------------------------
// PluginInstance implementation
// ---------------------------------------------------------------------------

void PluginInstance::process(const float* input, float* output, FrameCount frames)
{
    if (!active)
    {
        // Bypass: copy input to output unchanged
        std::memcpy(output, input, sizeof(float) * frames * 2);
        return;
    }

    // Simplified processing: apply a unity-gain pass-through.
    // A real implementation would delegate to the plugin's processReplacing().
    std::memcpy(output, input, sizeof(float) * frames * 2);
}

float PluginInstance::getParameter(uint32_t index) const
{
    if (index < m_params.size())
        return m_params[index];
    return 0.0f;
}

void PluginInstance::setParameter(uint32_t index, float value)
{
    if (index >= m_params.size())
        m_params.resize(index + 1, 0.0f);
    m_params[index] = std::clamp(value, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// PluginHost implementation
// ---------------------------------------------------------------------------

PluginId PluginHost::loadPlugin(const PluginInfo& info)
{
    if (m_instances.size() >= kMaxPluginsPerHost)
        return 0;

    const PluginId id = nextId();
    auto inst     = std::make_unique<PluginInstance>();
    inst->id      = id;
    inst->info    = info;
    inst->active  = true;

    m_order.push_back(id);
    m_instances[id] = std::move(inst);
    return id;
}

void PluginHost::unloadPlugin(PluginId id)
{
    m_instances.erase(id);
    m_order.erase(std::remove(m_order.begin(), m_order.end(), id), m_order.end());
}

PluginInstance* PluginHost::getInstance(PluginId id)
{
    auto it = m_instances.find(id);
    return it != m_instances.end() ? it->second.get() : nullptr;
}

const PluginInstance* PluginHost::getInstance(PluginId id) const
{
    auto it = m_instances.find(id);
    return it != m_instances.end() ? it->second.get() : nullptr;
}

void PluginHost::processChain(const float* inputBuffer, float* outputBuffer, FrameCount frames)
{
    if (m_order.empty())
    {
        std::memcpy(outputBuffer, inputBuffer, sizeof(float) * frames * 2);
        return;
    }

    // Temp scratch buffer for the intermediate signal between plugins.
    std::vector<float> scratch(frames * 2);
    const float* src = inputBuffer;

    for (size_t i = 0; i < m_order.size(); ++i)
    {
        auto* inst = getInstance(m_order[i]);
        if (!inst) continue;

        float* dst = (i == m_order.size() - 1) ? outputBuffer : scratch.data();
        inst->process(src, dst, frames);
        src = dst;
    }
}

void PluginHost::forEach(VisitorFn fn)
{
    for (auto& id : m_order)
    {
        auto it = m_instances.find(id);
        if (it != m_instances.end())
            fn(*it->second);
    }
}

} // namespace FL
