#ifndef SCARD_WATCHER_H_
#define SCARD_WATCHER_H_

#include <windows.h>
#include <winscard.h>
#include <winuser.h>
#include <dbt.h>

#include <map>
#include <string>
#include <optional>
#include <functional>

#define SW_INSERTION 0x12345678
#define SW_REMOVAL 0x87654321

namespace scard_watcher {

    // Smart Card Readers
    // Class = SmartCardReader
    // ClassGuid = {50dd5230-ba8a-11d1-bf5d-0000f805f530}
    // This class includes smart card readers.
    const GUID ScardReaderGUID = { 0x50dd5230, 0xba8a, 0x11d1, 0xbf, 0x5d, 0x00, 0x00, 0xf8, 0x05, 0xf5, 0x30 };

    class ScardWatcher {
    private:
        HWND hWnd;
        SCARDCONTEXT hSC;
        HDEVNOTIFY hDeviceNotify;
        UINT_PTR mTimer;
        ULONGLONG mDeviceChangedTimestamp;
        std::map<std::string, std::map<std::string, std::string>> mReaders;

        static void DeviceChangeChecker(HWND hwnd, UINT message, UINT_PTR timer, DWORD time);
        void PostInsertionEvent(std::string readerName);
        void PostRemovalEvent(std::string readerName);
    public:
        ScardWatcher();
        ~ScardWatcher();

        BOOL DoRegisterDeviceInterfaceToHwnd(HWND hwnd);
        void UpdateReadersState();
        std::map<std::string, std::map<std::string, std::string>> GetSmartCards();
        std::map<std::string, std::string> GetSmartCardDetails(std::string readerName);
        std::optional<LRESULT> DeviceChangeEventHandler(WPARAM event, PDEV_BROADCAST_HDR lpdb, std::function<void(std::string)> insertionCallback, std::function<void(std::string)> removalCallback);
    };

} // namespace scard_watcher

#endif  // SCARD_WATCHER_H_