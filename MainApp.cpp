#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class MainApp {
private:
    std::wstring offlineAction;
    std::wstring onlineAction;
    std::wstring actionOnStart;
    bool runOnce = false;
    bool blockingMode = false;
    HANDLE hActiveJob = NULL;
public:
    void setOfflineAction(std::wstring offlineAction) {
        MainApp::offlineAction = offlineAction;
    }
    void setOnlineAction(std::wstring onlineAction) {
        MainApp::onlineAction = onlineAction;
    }
    void setActionOnStart(std::wstring actionOnStart) {
        MainApp::actionOnStart = actionOnStart;
    }
    void setRunOnce(bool runOnce) {
        MainApp::runOnce = runOnce;
    }
    void setBlockingMode(bool blockingMode) {
        MainApp::blockingMode = blockingMode;
    }

    bool isConfigValid() {
        return !offlineAction.empty() || !onlineAction.empty();
    }

    void runOnlineAction() {
        runAction(onlineAction);
    }

    void runOfflineAction() {
        runAction(offlineAction);
    }

    void started() {
        if (!actionOnStart.empty()) {
            if (actionOnStart.compare(TEXT("online")) == 0) {
                runOnlineAction();
            } else if (actionOnStart.compare(TEXT("online")) == 0) {
                runOfflineAction();
            } else {
                runAction(actionOnStart);
            }
        }
    }

    ~MainApp() {
        if (!runOnce) {
            CloseHandle(hActiveJob);
            hActiveJob = NULL;
        }
    }
private:
    void killCurrentJob() {
        if (blockingMode && hActiveJob != NULL) {
            CloseHandle(hActiveJob);
            hActiveJob = NULL;
        }
    }

    PROCESS_INFORMATION* runAction(std::wstring action) {
        killCurrentJob();
 
        if (action.empty()) {
            return NULL;
        }

        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;

        if (!CreateProcess(NULL, (LPWSTR)action.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &info, &processInfo)) {
            DWORD dw = GetLastError();

            LPWSTR lpMsgBuf = NULL;

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&lpMsgBuf,
                0, NULL);

            MessageBox(NULL, lpMsgBuf, NULL, MB_ICONERROR);

            return NULL;
        } else {
            if (blockingMode) {
                hActiveJob = CreateJobObject(NULL, TEXT("ACJobGroup"));
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = {};
                jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                SetInformationJobObject(hActiveJob, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo));
                AssignProcessToJobObject(hActiveJob, processInfo.hProcess);
            }

            if (runOnce) {
                PostQuitMessage(0);
            }
            return &processInfo;
        }
    }
};
