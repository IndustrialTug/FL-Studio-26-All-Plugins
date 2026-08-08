#pragma once
#include "../../include/FLTypes.h"
#include <string>
#include <vector>
#include <functional>

namespace FL {

// ---------------------------------------------------------------------------
// PluginInfo
// Metadata gathered by PluginScanner for one discovered plugin file.
// ---------------------------------------------------------------------------
struct PluginInfo {
    std::string  filePath;
    std::string  name;
    std::string  vendor;
    std::string  version;
    PluginFormat format      = PluginFormat::Unknown;
    uint32_t     numInputs   = 0;
    uint32_t     numOutputs  = 0;
    bool         isInstrument = false;
    bool         isSynth     = false;
};

// ---------------------------------------------------------------------------
// PluginScanner
// Walks a directory tree and collects VST2/VST3 plugin descriptors.
// Scanning is synchronous; call scan() from a background thread if needed.
// ---------------------------------------------------------------------------
class PluginScanner {
public:
    // Callback invoked once for each successfully parsed plugin during a scan.
    using FoundCallback = std::function<void(const PluginInfo&)>;

    PluginScanner() = default;
    ~PluginScanner() = default;

    // Add a root directory to the scan path list.  Duplicates are silently ignored.
    void addSearchPath(std::string path);

    // Remove a previously added search path.
    void removeSearchPath(const std::string& path);

    const std::vector<std::string>& searchPaths() const noexcept { return m_searchPaths; }

    // Scan all registered directories.  Clears any previously found results.
    // Returns the number of plugins found.
    size_t scan(FoundCallback cb = nullptr);

    // Results from the last scan() call.
    const std::vector<PluginInfo>& results() const noexcept { return m_results; }

    // Clear cached scan results without touching the search paths.
    void clearResults() { m_results.clear(); }

    // Filter results by format.
    std::vector<PluginInfo> filterByFormat(PluginFormat fmt) const;

    // Filter results by instrument / effect type.
    std::vector<PluginInfo> filterInstruments() const;
    std::vector<PluginInfo> filterEffects() const;

private:
    // Recursively walk a directory and probe each candidate file.
    void walkDirectory(const std::string& dir, FoundCallback& cb);

    // Attempt to parse plugin metadata from a file path.
    // Returns false if the file is not a recognised plugin.
    bool probeFile(const std::string& filePath, PluginInfo& out) const;

    PluginFormat detectFormat(const std::string& filePath) const noexcept;

    std::vector<std::string>  m_searchPaths;
    std::vector<PluginInfo>   m_results;
};

} // namespace FL
