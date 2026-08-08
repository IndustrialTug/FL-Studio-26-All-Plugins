#include "flstudio/utils/FileSystemWatcher.h"
#include <Windows.h>

namespace fl {

void FileSystemWatcher::start()
{
    m_running = true;
    m_thread = std::thread([this]() {
        HANDLE hDir = CreateFileW(m_path.c_str(), FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

        if (hDir == INVALID_HANDLE_VALUE) return;

        char buf[4096];
        DWORD bytesReturned = 0;
        while (m_running) {
            if (ReadDirectoryChangesW(hDir, buf, sizeof(buf), TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned, nullptr, nullptr))
            {
                auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buf);
                std::wstring name(info->FileName, info->FileNameLength / sizeof(wchar_t));
                m_cb(name);
            }
        }
        CloseHandle(hDir);
    });
}

void FileSystemWatcher::stop()
{
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

} // namespace fl