#ifndef LOADING_HANDLER_HPP
#define LOADING_HANDLER_HPP

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <CommCtrl.h>
#include <thread>

#pragma comment(lib, "Comctl32.lib")

class LoadingHandler final
{
private:
	INITCOMMONCONTROLSEX initCommonControls;
	WNDCLASSEXW widgetClass{};
	MSG messageHandler{};

	std::jthread task;
	DWORD threadId {0};

	void handlerTask(std::stop_token stopToken);
	LRESULT CALLBACK loadingEvents(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	LoadingHandler();
	~LoadingHandler();
};

#endif