#include <Windows.h>
#include <CommCtrl.h>
#include <thread>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Comctl32.lib")

static int progress = 0;
HBRUSH backgroundBrush = CreateSolidBrush(RGB(12, 12, 12));

// WIDGET EVENT HANDLER
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);

        FillRect(
            reinterpret_cast<HDC>(wParam),
            &rect,
            backgroundBrush
        );

        return 1;
    }
    case WM_TIMER:
        SendMessageW
        (
            GetDlgItem(hwnd, 100),
            PBM_SETPOS,
            progress++,
            0
        );

        if (progress > 100)
            KillTimer(hwnd, 1);

        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char* argv[])
{
    // INITIALIZE THE WINDOWS CONTROLS
    INITCOMMONCONTROLSEX initCommonControls{};
    initCommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    initCommonControls.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&initCommonControls);


    // CREATE THE WIDGET CLASS
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"VideoDecoderLoadingWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOWFRAME);
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
    // Title bar background
    COLORREF captionColor = RGB(30, 30, 30);
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));

    // Title bar text
    COLORREF textColor = RGB(255, 255, 255);
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));

    // CREATE THE PROGRESS BAR
    HWND progressBar = CreateWindowExW(
        0,
        PROGRESS_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST | PBS_SMOOTH,
        20, 50,
        340, 20,
        hwnd,
        reinterpret_cast<HMENU>(100),
        GetModuleHandleW(nullptr),
        nullptr
    );
    // SET THE PROGRESS BAR INTO THE WIDGET
    SendMessageW(progressBar, PBM_SETRANGE32, 0, 100);
    SendMessageW(progressBar, PBM_SETPOS, 0, 0);
    SendMessageW(progressBar, PBM_SETBARCOLOR, 0, RGB(255, 60, 60));
    SendMessageW(progressBar, PBM_SETBKCOLOR, 0, RGB(40, 40, 40));

    // Show & Update the widget
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 5, nullptr);

    // Handle the Windows messages
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}