#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <QObject>
#include <QMessageBox>
#include <mutex>
#include <queue>

enum class MESSAGEBOX_TYPE
{
	INFO,
	QUESTION,
	WARNING,
	CRITICAL,
};

struct MessageLogger final
{
	std::string message; 
	std::string name; 
	MESSAGEBOX_TYPE type;
	bool isBlocking;
};

class ErrorHandler : public QObject
{
	Q_OBJECT
private:
	std::mutex mutex;
	std::queue<MessageLogger> messageQueue;
	QMessageBox* activeMessageBox { nullptr };

	void createMessageBox(const MessageLogger& message);
public:
	explicit ErrorHandler(QObject* parent = nullptr) {};
	~ErrorHandler();

	void showMessage(const std::string& message, const std::string& name, const MESSAGEBOX_TYPE& type, bool isBlocking);
	void showInfo(const std::string& message, bool isBlocking);
	void showQuestion(const std::string& message, bool isBlocking);
	void showWarning(const std::string& message, bool isBlocking);
	void showError(const std::string& message, bool isBlocking);
	void update();
};

#endif