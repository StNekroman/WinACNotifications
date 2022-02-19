#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string>
#include "MainApp.cpp"

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
        } else {
            WCHAR message[1024];
            wsprintf(message, TEXT("Unknown argument received:\n%s"), arg.c_str());
            MessageBox(NULL, message, NULL, MB_ICONERROR);
        }
    }

    LocalFree(args);

    if (!app.isConfigValid()) {
        MessageBox(NULL, TEXT("Both --online and --offline are not set - nothing to do, exiting."), NULL, MB_ICONERROR);
        return 0;
    }

    WNDCLASSEX wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("dummywindow");
    wc.cbSize = sizeof(wc);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Could not register window class"), NULL, MB_ICONERROR);
        return 0;
    }

    const HWND hwnd =  CreateWindow(TEXT("dummywindow"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBox(NULL, TEXT("Could not create window"), NULL, MB_ICONERROR);
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

    return (int) msg.wParam;
}
