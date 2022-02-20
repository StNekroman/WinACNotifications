#define WIN32_LEAN_AND_MEAN
#define ELPP_NO_DEFAULT_LOG_FILE

#include <windows.h>
#include <shellapi.h>
#include <string>
#include "easylogging++.h"
#include "easylogging++.cc"
#include "MainApp.cpp"

INITIALIZE_EASYLOGGINGPP

std::string utf8_encode(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

/**
* @author StNekroman
*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    int argsCount;
    LPWSTR * args = CommandLineToArgvW(lpCmdLine , &argsCount);

    MainApp app = MainApp();

    el::Configurations conf;
    conf.setToDefault();

    for (int i = 0; i < argsCount; i++) {
        std::wstring arg = args[i];
        if (arg.compare(TEXT("--offline")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];
                app.setOfflineAction(arg);
            }
        } else if (arg.compare(TEXT("--online")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];
                app.setOnlineAction(arg);
            }
        } else if (arg.compare(TEXT("--runonce")) == 0) {
            app.setRunOnce(true);
        } else if (arg.compare(TEXT("--blocking")) == 0) {
            app.setBlockingMode(true);
        } else if (arg.compare(TEXT("--nosleep")) == 0) {
            app.setNoSleep(true);
        } else if (arg.compare(TEXT("--onstart")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];
                app.setActionOnStart(arg);
            }
        } else if (arg.compare(TEXT("--offlinetimeout")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];
                app.setOfflineTimeout(std::stoi(arg));
            }
        } else if (arg.compare(TEXT("--onlinetimeout")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];
                app.setOnlineTimeout(std::stoi(arg));
            }
        } else if (arg.compare(TEXT("--logfile")) == 0) {
            if (args[i + 1] != NULL) {
                arg = args[++i];

                conf.setGlobally(el::ConfigurationType::Filename, utf8_encode(arg));
                el::Loggers::reconfigureAllLoggers(conf);
            }
        } else {
            LOG(WARNING) << "Unknown argument received: " << arg;
        }
    }
    LocalFree(args);

    if (!app.isConfigValid()) {
        LOG(ERROR) << "Both --online and --offline are not set - nothing to do, exiting.";
        return 0;
    }

    WNDCLASSEX wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("dummywindow");
    wc.cbSize = sizeof(wc);

    if (!RegisterClassEx(&wc)) {
        LOG(ERROR) << "Could not register window class";
        return 0;
    }

    const HWND hwnd =  CreateWindow(TEXT("dummywindow"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        LOG(ERROR) << "Could not create window";
        return 0;
    }

    app.started();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_POWERBROADCAST && msg.hwnd == hwnd) {
            app.onACStatusChanged();
        }
 
        DispatchMessage(&msg);
    }

    LOG(INFO) << "Received exit message.";

    return (int) msg.wParam;
}
