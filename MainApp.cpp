#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

#define IDT_TIMER_ID 100

enum class ACStatus {
    ONLINE,
    OFFLINE,
    UNKNOWN
};

/**
* @author StNekroman
*/
class MainApp {
private:
    std::wstring offlineAction;
    std::wstring onlineAction;
    std::wstring actionOnStart;
    int offlineTimeout = 0;
    int onlineTimeout = 0;
    bool runOnce = false;
    bool blockingMode = false;
    bool noSleep = false;
    HANDLE hActiveJob = NULL;
    UINT_PTR timerId = NULL;
    const std::wstring* lastAction = NULL;
    ACStatus currentStatus = ACStatus::UNKNOWN;
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
    void setOfflineTimeout(int offlineTimeout) {
        MainApp::offlineTimeout = offlineTimeout;
    }
    void setOnlineTimeout(int onlineTimeout) {
        MainApp::onlineTimeout = onlineTimeout;
    }
    void setRunOnce(bool runOnce) {
        MainApp::runOnce = runOnce;
    }
    void setBlockingMode(bool blockingMode) {
        MainApp::blockingMode = blockingMode;
    }
    void setNoSleep(bool noSleep) {
        MainApp::noSleep = noSleep;
    }

    bool isConfigValid() {
        return !offlineAction.empty() || !onlineAction.empty();
    }

    void onACStatusChanged() {
        ACStatus status = getACStatus();        
        if (status == ACStatus::ONLINE) {
            runAction(onlineAction, onlineTimeout);
        } else if (status == ACStatus::OFFLINE) {
            runAction(offlineAction, offlineTimeout);
        }
    }

    void started() {
        if (!actionOnStart.empty()) {
            if (actionOnStart.compare(TEXT("online")) == 0) {
                const ACStatus status = getACStatus();
                if (status == ACStatus::ONLINE) {
                    startProcess(onlineAction);
                }
            } else if (actionOnStart.compare(TEXT("offline")) == 0) {
                const ACStatus status = getACStatus();
                if (status == ACStatus::OFFLINE) {
                    startProcess(offlineAction);
                }
            } else {
                startProcess(actionOnStart);
            }
        }
        if (noSleep) {
            SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS | ES_AWAYMODE_REQUIRED);
        }
    }

    ~MainApp() {
        if (!runOnce) {
            CloseHandle(hActiveJob);
            hActiveJob = NULL;
        }
        if (noSleep) {
            SetThreadExecutionState(ES_CONTINUOUS);
        }
    }
private:
    static void timerproc(HWND hWND, UINT msgId, UINT_PTR timerId, DWORD systemTime);
    void runAction(const std::wstring& action, int timeout);
    
    void killCurrentJob() {
        if (blockingMode && hActiveJob != NULL) {
            CloseHandle(hActiveJob);
            hActiveJob = NULL;
        }
    }

    ACStatus getACStatus() {
        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus)) {
            if (powerStatus.ACLineStatus == 1) {
                return ACStatus::ONLINE;
            } else if (powerStatus.ACLineStatus == 0) {
                return ACStatus::OFFLINE;
            }
        }
        return ACStatus::UNKNOWN;
    }

    void killTimer() {
        if (timerId != NULL) {
            KillTimer(NULL, timerId);
            timerId = NULL;
        }
    }

    HANDLE startProcess(const std::wstring& action) {
        killTimer();

        const ACStatus status = getACStatus();
        if (status == currentStatus) {
            return NULL; // no need to do anything, already in this state
        }
 
        currentStatus = status;
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
            return processInfo.hProcess;
        }
    }
};

MainApp* globalRef = NULL;

void MainApp::runAction(const std::wstring& action, int timeout) {
    if (timeout == 0) {
        startProcess(action);
    } else {
        lastAction = &action;
        globalRef = this;
        timerId = SetTimer(NULL, IDT_TIMER_ID, timeout * 1000, MainApp::timerproc);
    }
}

void MainApp::timerproc(HWND hWND, UINT msgId, UINT_PTR timerId, DWORD systemTime) {
    if (msgId == WM_TIMER && timerId == globalRef->timerId) {
        globalRef->killTimer();
        globalRef->startProcess(*globalRef->lastAction);
    }
}
