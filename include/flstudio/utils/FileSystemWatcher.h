#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>

namespace fl {

class FileSystemWatcher {
public:
    using Callback = std::function<void(const std::wstring&)>;

    FileSystemWatcher(const std::wstring& path, Callback cb)
        : m_path(path), m_cb(std::move(cb)), m_running(false) {}

    void start();
    void stop();

private:
    std::wstring      m_path;
    Callback          m_cb;
    std::atomic<bool> m_running{false};
    std::thread       m_thread;
};

} // namespace fl