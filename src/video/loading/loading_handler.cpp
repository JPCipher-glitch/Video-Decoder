#include "video/loading/loading_handler.hpp"

#pragma region EVENTS
HBRUSH backgroundBrush = CreateSolidBrush(RGB(12, 12, 12));
LRESULT CALLBACK WidgetEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);

        //FillRect(reinterpret_cast<HDC>(wParam), &rect, backgroundBrush);

        return 1;
    }
    case WM_DESTROY:
        //PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
#pragma endregion

#pragma region CONSTRUCTOR
LoadingHandler::LoadingHandler()
{
    // Initialize The Windows Controls
    initCommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    initCommonControls.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&initCommonControls);

	// Create The Widget Class
    widgetClass.cbSize = sizeof(WNDCLASSEXW);
    widgetClass.lpfnWndProc = WidgetEvents;
    widgetClass.hInstance = GetModuleHandleW(nullptr);
    widgetClass.lpszClassName = L"VideoDecoderLoadingWindow";
    widgetClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    widgetClass.hbrBackground = GetSysColorBrush(COLOR_WINDOWFRAME);
    RegisterClassExW(&widgetClass);

    // Create the task to manage the packet transfer
    task = std::jthread([this](std::stop_token stopToken)
    {
        handlerTask(stopToken);
    });
}
#pragma endregion

#pragma region DESTRUCTOR
LoadingHandler::~LoadingHandler()
{
    task.request_stop(); // Ask to the task to stop
	PostThreadMessageW(threadId, WM_QUIT, 0, 0); // Send a message to the task to stop the message loop
}
#pragma endregion


#pragma region TASK
void LoadingHandler::handlerTask(std::stop_token stopToken)
{
	threadId = GetCurrentThreadId(); // Get the thread ID of the task
    // Handle the Windows messages
    while (!stopToken.stop_requested())
    {
        if (GetMessageW(&messageHandler, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&messageHandler);
            DispatchMessageW(&messageHandler);
        }
    }
}
#pragma endregion

