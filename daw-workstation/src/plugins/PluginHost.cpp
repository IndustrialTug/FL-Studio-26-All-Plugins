#include "flstudio/plugins/PluginHost.h"
#include <iostream>

namespace fl {

PluginHost::PluginHost(int sampleRate, int blockSize)
    : m_sampleRate(sampleRate), m_blockSize(blockSize)
{}

bool PluginHost::LoadPlugin(const PluginDescriptor& desc)
{
    // FL Studio 25 all plugins free download support:
    // this stub simulates loading a VST/VST3 plugin
    if (desc.path.empty()) return false;
    m_loaded.push_back(desc);
    std::cout << "[PluginHost] Loaded: " << desc.name
              << " (" << (desc.isVST3 ? "VST3" : "VST2") << ")\n";
    return true;
}

void PluginHost::UnloadPlugin(const std::string& name)
{
    m_loaded.erase(
        std::remove_if(m_loaded.begin(), m_loaded.end(),
            [&](const PluginDescriptor& d) { return d.name == name; }),
        m_loaded.end());
}

void PluginHost::ProcessBlock(float* left, float* right, int numSamples)
{
    // Each loaded plugin processes the buffer in series
    (void)left; (void)right; (void)numSamples;
}

} // namespace fl