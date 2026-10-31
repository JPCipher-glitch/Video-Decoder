#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <future>
#include <string>
#include <unordered_set>
#include <queue>

struct MessageWindow
{
	std::wstring message; 
	std::wstring name; 
	uint32_t iconTag;
};

class ErrorHandler
{
private:
	std::mutex mutex;
	std::future<void> task;
	std::queue<MessageWindow> messageQueue;

	void showMessage(const std::wstring& message, const std::wstring& name, const uint32_t& iconTag, bool isBlocking);
	void shutdownMessages();
public:
	ErrorHandler() = default;
	~ErrorHandler();

	std::wstring toWString(const std::string& str);
	void showWarning(const std::wstring& message, bool isBlocking);
	void showError(const std::wstring& message, bool isBlocking);
};

#endif