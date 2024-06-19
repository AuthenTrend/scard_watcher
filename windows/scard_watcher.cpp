#include "scard_watcher.h"

#include <sysinfoapi.h>

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace scard_watcher {

    ScardWatcher *self;

    ScardWatcher::ScardWatcher() {
        auto result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSC);
        if (result != SCARD_S_SUCCESS) {
            cout << "Failed SCardEstablishContext: " << (UINT)result << endl;
        }
    }

    ScardWatcher::~ScardWatcher() {
        SCardReleaseContext(hSC);
        if (NULL != hDeviceNotify) {
            UnregisterDeviceNotification(hDeviceNotify);
        }
    }

    map<string, map<string, string>> ScardWatcher::GetSmartCards() {
        map<string, map<string, string>> smartCards;
        for (auto &reader : mReaders) {
            auto details = reader.second;
            if (details["state"].compare("SCARD_ABSENT") != 0) {
                smartCards[reader.first] = details;
            }
        }
        return smartCards;
    }

    map<string, string> ScardWatcher::GetSmartCardDetails(string readerName) {
        if (mReaders.find(readerName) == mReaders.end()) {
            map<string, string> empty;
            return empty;
        }
        return mReaders[readerName];
    }

    void ScardWatcher::UpdateReadersState() {
        map<string, map<string, string>> readers;
        LPSTR pmszReaders = NULL;
        LPSTR pReader;
        DWORD cch = SCARD_AUTOALLOCATE;
        auto result = SCardListReadersA(hSC, NULL, (LPSTR)&pmszReaders, &cch);
        if (SCARD_E_SERVICE_STOPPED == result) {
            SCardReleaseContext(hSC);
            result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSC);
            if (result != SCARD_S_SUCCESS) {
                cout << "Failed SCardEstablishContext: " << (UINT)result << endl;
                return;
            }
            result = SCardListReadersA(hSC, NULL, (LPSTR)&pmszReaders, &cch);
        }
        if (SCARD_S_SUCCESS == result) {
            pReader = pmszReaders;
            while ('\0' != *pReader) {
                string readerName(pReader);
                SCARDHANDLE hCardHandle;
                DWORD dwAP;
                result = SCardConnectA(hSC, pReader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCardHandle, &dwAP);
                pReader = pReader + strlen(pReader) + 1;
                if (SCARD_S_SUCCESS != result) {
                    continue;
                }

                CHAR szReader[200];
                cch = 200;
                BYTE bAttr[32];
                DWORD cByte = 32;
                DWORD dwState, dwProtocol;
                result = SCardStatusA(hCardHandle, szReader, &cch, &dwState, &dwProtocol, (LPBYTE)&bAttr, &cByte);
                SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
                if (SCARD_S_SUCCESS != result) {
                    continue;
                }

                map<string, string> details;
                switch (dwState) {
                    case SCARD_ABSENT:
                        details["state"] = "SCARD_ABSENT";
                        break;
                    case SCARD_PRESENT:
                        details["state"] = "SCARD_PRESENT";
                        break;
                    case SCARD_SWALLOWED:
                        details["state"] = "SCARD_SWALLOWED";
                        break;
                    case SCARD_POWERED:
                        details["state"] = "SCARD_POWERED";
                        break;
                    case SCARD_NEGOTIABLE:
                        details["state"] = "SCARD_NEGOTIABLE";
                        break;
                    case SCARD_SPECIFIC:
                        details["state"] = "SCARD_SPECIFIC";
                        break;
                    default:
                        details["state"] = "UNKNOWN";
                        break;
                }
                readers[readerName] = details;
            }
            // Free the memory.
            SCardFreeMemory(hSC,pmszReaders);
        }

        for (auto &reader : readers) {
            if (mReaders.find(reader.first) == mReaders.end() || mReaders[reader.first]["state"].compare("SCARD_ABSENT") == 0) {
                if (reader.second["state"].compare("SCARD_ABSENT") != 0) {
                    PostInsertionEvent(reader.first);
                }
            } else if (mReaders[reader.first]["state"].compare("SCARD_ABSENT") != 0 && reader.second["state"].compare("SCARD_ABSENT") == 0) {
                PostRemovalEvent(reader.first);
            }
        }
        for (auto &reader : mReaders) {
            if (readers.find(reader.first) == readers.end()) {
                PostRemovalEvent(reader.first);
            }
        }
        mReaders = readers;
    }

    BOOL ScardWatcher::DoRegisterDeviceInterfaceToHwnd(HWND hwnd) {
        hWnd = hwnd;
        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
        ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
        NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        NotificationFilter.dbcc_classguid = ScardReaderGUID;

        if (NULL != hDeviceNotify) {
            UnregisterDeviceNotification(hDeviceNotify);
        }
        hDeviceNotify = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (NULL == hDeviceNotify) {
            return FALSE;
        }
        if (NULL != mTimer) {
            KillTimer(hWnd, mTimer);
        }
        self = this;
        SetTimer(hWnd, mTimer, 1000, DeviceChangeChecker);
        return TRUE;
    }

    void ScardWatcher::DeviceChangeChecker(HWND hwnd, UINT message, UINT_PTR timer, DWORD time) {
        ULONGLONG diff = GetTickCount64() - self->mDeviceChangedTimestamp;
        if (self->mDeviceChangedTimestamp == 0 || diff < 500ULL) {
            return;
        }

        self->mDeviceChangedTimestamp = 0;
        self->UpdateReadersState();
    }

    void ScardWatcher::PostInsertionEvent(string readerName) {
        size_t size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_A) + strlen(readerName.c_str());
        PDEV_BROADCAST_DEVICEINTERFACE_A lparam = (PDEV_BROADCAST_DEVICEINTERFACE_A)LocalAlloc(LMEM_FIXED, size);
        ZeroMemory(lparam, size);
        lparam->dbcc_size = (DWORD)size;
        lparam->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        lparam->dbcc_classguid = ScardReaderGUID;
        lstrcpyA(lparam->dbcc_name, readerName.c_str());
        PostMessageA(hWnd, WM_DEVICECHANGE, SW_INSERTION, (LPARAM)lparam);
    }

    void ScardWatcher::PostRemovalEvent(string readerName) {
        size_t size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_A) + strlen(readerName.c_str());
        PDEV_BROADCAST_DEVICEINTERFACE_A lparam = (PDEV_BROADCAST_DEVICEINTERFACE_A)LocalAlloc(LMEM_FIXED, size);
        ZeroMemory(lparam, size);
        lparam->dbcc_size = (DWORD)size;
        lparam->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        lparam->dbcc_classguid = ScardReaderGUID;
        lstrcpyA(lparam->dbcc_name, readerName.c_str());
        PostMessageA(hWnd, WM_DEVICECHANGE, SW_REMOVAL, (LPARAM)lparam);
    }

    std::optional<LRESULT> ScardWatcher::DeviceChangeEventHandler(WPARAM event, PDEV_BROADCAST_HDR lpdb, std::function<void(std::string)> insertionCallback, std::function<void(std::string)> removalCallback) {
        std::optional<LRESULT> result = std::nullopt;
        switch (event) {
            case DBT_DEVICEARRIVAL:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    PDEV_BROADCAST_DEVICEINTERFACE_A lpdbd = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
                    if (!IsEqualGUID(ScardReaderGUID, lpdbd->dbcc_classguid)) {
                        break;
                    }
                    mDeviceChangedTimestamp = GetTickCount64();
                }
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    PDEV_BROADCAST_DEVICEINTERFACE_A lpdbd = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
                    if (!IsEqualGUID(ScardReaderGUID, lpdbd->dbcc_classguid)) {
                        break;
                    }
                    mDeviceChangedTimestamp = GetTickCount64();
                }
                break;
            case DBT_DEVNODES_CHANGED: // card insertion/removal
                mDeviceChangedTimestamp = GetTickCount64();
                break;
            case SW_INSERTION:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    PDEV_BROADCAST_DEVICEINTERFACE_A lpdbd = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
                    if (!IsEqualGUID(ScardReaderGUID, lpdbd->dbcc_classguid)) {
                        break;
                    }
                    auto readerName = std::string(lpdbd->dbcc_name);
                    insertionCallback(readerName);
                    LocalFree((HLOCAL)lpdbd);
                    result = 0;
                }
                break;
            case SW_REMOVAL:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    PDEV_BROADCAST_DEVICEINTERFACE_A lpdbd = (PDEV_BROADCAST_DEVICEINTERFACE_A)lpdb;
                    if (!IsEqualGUID(ScardReaderGUID, lpdbd->dbcc_classguid)) {
                        break;
                    }
                    auto readerName = std::string(lpdbd->dbcc_name);
                    removalCallback(readerName);
                    LocalFree((HLOCAL)lpdbd);
                    result = 0;
                }
                break;
        }

        return result;
    }

} // namespace scard_watcher
