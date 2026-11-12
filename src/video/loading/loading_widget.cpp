#include "video/loading/loading_widget.hpp"
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#pragma region CONSTRUCTOR
LoadingWidget::LoadingWidget()
{
	createWidget();
	createProgressBar();
    update();
}
#pragma endregion

#pragma region DESTRUCTOR
LoadingWidget::~LoadingWidget()
{
    // Destroy the widget
    if (widget)
        DestroyWindow(widget);
}
#pragma endregion

#pragma region WIDGET
void LoadingWidget::createWidget()
{
    // Create the widget 
    widget = CreateWindowExW
    (
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
	// Set the dark colors if enabled
    if (inDarkMode)
    {
        // Colors
        captionColor = RGB(30, 30, 30);
        textColor = RGB(255, 255, 255);

		// Set the colors
        DwmSetWindowAttribute(widget, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor)); // Title bar background
        DwmSetWindowAttribute(widget, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor)); // Title bar text
    }
}

void LoadingWidget::update()
{
    // Show & Update the widget
    ShowWindow(widget, SW_SHOW);
    UpdateWindow(widget);
}
#pragma endregion

#pragma region PROGRESS BAR
void LoadingWidget::createProgressBar()
{
    // Create the progress bar
    progressBar = CreateWindowExW
    (
        0,
        PROGRESS_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 50,
        340, 20,
        widget,
        reinterpret_cast<HMENU>(100),
        GetModuleHandleW(nullptr),
        nullptr
    );
    // Set the dark colors if enabled
    if (inDarkMode)
    {
        // SET THE PROGRESS BAR INTO THE WIDGET
        SendMessageW(progressBar, PBM_SETRANGE32, 0, 100);
        SendMessageW(progressBar, PBM_SETPOS, 0, 0);
        SendMessageW(progressBar, PBM_SETBARCOLOR, 0, RGB(255, 60, 60));
        SendMessageW(progressBar, PBM_SETBKCOLOR, 0, RGB(40, 40, 40));
    }
}
#pragma endregion