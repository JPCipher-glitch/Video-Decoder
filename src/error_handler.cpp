#include "error_handler.hpp"
#include "Windows.h"

#pragma region DESTRUCTOR
ErrorHandler::~ErrorHandler()
{
	shutdownMessages();
}
#pragma endregion


#pragma region PRIVATE
void ErrorHandler::showMessage(const std::wstring& message, const std::wstring& name, const uint32_t& iconTag, bool isBlocking)
{
	// If the message is non blocking then launch it in a separated task thread
	if (!isBlocking)
	{
		// Push the message to display in the queue
		{
			std::lock_guard<std::mutex> lock(mutex);
			messageQueue.push({ message, name, iconTag });
		}
		// Create the task once while the previous is not yet fisished
		if (!task.valid() || task.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			// Create the task
			std::function<void()> handler = [this]()
			{
				// Display all the messages
				while (true)
				{
					MessageWindow m;
					{
						std::lock_guard<std::mutex> lock(mutex);

						if (messageQueue.empty())
							break;

						m = std::move(messageQueue.front());
						messageQueue.pop();
					}

					MessageBoxW(nullptr, m.message.c_str(), m.name.c_str(), MB_OK | m.iconTag);
				}
			};
			task = std::async(std::launch::async, [handler = std::move(handler)] { handler(); });
		}
	}
	else
	{
		MessageBoxW(nullptr, message.c_str(), name.c_str(), MB_OK | iconTag);
	}
}
#pragma endregion


#pragma region STRING CONVERTER
std::wstring ErrorHandler::toWString(const std::string& str)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring result(size-1, L'\0');

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
	return result;
}
#pragma endregion

#pragma region PUBLIC MESSAGES
void ErrorHandler::showWarning(const std::wstring& message, bool isBlocking)
{
	showMessage(message, L"VideoDecoder - Warning", MB_ICONWARNING, isBlocking);
}

void ErrorHandler::showError(const std::wstring& message, bool isBlocking)
{
	showMessage(message, L"VideoDecoder - Error", MB_ICONERROR, isBlocking);
}
#pragma endregion

#pragma region SHUTDOWN
BOOL CALLBACK closeMessages(HWND hwnd, LPARAM)
{
	// List of all the message name
	std::unordered_set<std::wstring> messageNames = { L"VideoDecoder - Warning", L"VideoDecoder - Error" };

	wchar_t title[256]{};
	GetWindowTextW(hwnd, title, 256);

	if (messageNames.contains(title))
	{
		// Check if the messages are created from the VideoDecoder app
		DWORD processId = 0;
		GetWindowThreadProcessId(hwnd, &processId);

		if (processId == GetCurrentProcessId())
			PostMessageW(hwnd, WM_CLOSE, 0, 0);
	}

	return TRUE;
}

void ErrorHandler::shutdownMessages()
{
	// Close all the window popups from the decoder
	{
		std::lock_guard<std::mutex> lock(mutex);
		while (!messageQueue.empty())
			messageQueue.pop();
	}
	
	EnumWindows(closeMessages, 0);
}
#pragma endregion
