#include <Windows.h>
#include <CommCtrl.h>
#include <thread>

#pragma comment(lib, "Comctl32.lib")

// WIDGET EVENT HANDLER
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char* argv[])
{
    // INITIALIZE THE WINDOWS CONTROLS
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);


    // CREATE THE WIDGET CLASS
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"VideoDecoderLoadingWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassExW(&wc);


    // CREATE THE WIDGET 
    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"VideoDecoderLoadingWindow",
        L"Video Decoder - Loading",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        400,
        150,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    // CREATE THE PROGRESS BAR
    HWND progressBar = CreateWindowExW(
        0,
        PROGRESS_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 50,
        340, 20,
        hwnd,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
    // SET THE PROGRESS BAR INTO THE WIDGET
    SendMessageW(progressBar, PBM_SETRANGE32, 0, 100);
    SendMessageW(progressBar, PBM_SETPOS, 50, 0);

    // Show & Update the widget
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Handle the Windows messages
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}