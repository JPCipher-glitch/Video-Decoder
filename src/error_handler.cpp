#include "error_handler.hpp"
#include <QEventLoop>
#include <QSpacerItem>
#include <QGridLayout>

#pragma region DESTRUCTOR
ErrorHandler::~ErrorHandler()
{
    if (activeMessageBox)
    {
        disconnect(activeMessageBox, nullptr, this, nullptr);
        delete activeMessageBox;
        activeMessageBox = nullptr;
    }
}
#pragma endregion

#pragma region PRIVATE
void ErrorHandler::createMessageBox(const MessageLogger& message)
{
    if (activeMessageBox) return; // Don't create another message box while the last one is not yet done

    // Create the message box
    activeMessageBox = new QMessageBox();
    activeMessageBox->setWindowTitle(QString::fromStdString(message.name));
    activeMessageBox->setText(QString::fromStdString(message.message));
    activeMessageBox->setAttribute(Qt::WA_DeleteOnClose);

    // Apply the icon & the buttons with the following type
    switch (message.type)
    {
        case MESSAGEBOX_TYPE::INFO:
        {
            activeMessageBox->setIcon(QMessageBox::Information);
            activeMessageBox->setStandardButtons(QMessageBox::Ok);
            break;
        }
        case MESSAGEBOX_TYPE::QUESTION:
        {
            activeMessageBox->setIcon(QMessageBox::Question);
            activeMessageBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            break;
        }
        case MESSAGEBOX_TYPE::WARNING:
        {
            activeMessageBox->setIcon(QMessageBox::Warning);
            activeMessageBox->setStandardButtons(QMessageBox::Ok);
            break;
        }
        case MESSAGEBOX_TYPE::CRITICAL:
        {
            activeMessageBox->setIcon(QMessageBox::Critical);
            activeMessageBox->setStandardButtons(QMessageBox::Ok);
            break;
        }
    }

    // Add a spacer to add minimal width to the widget
    QSpacerItem* spacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = qobject_cast<QGridLayout*>(activeMessageBox->layout());
    layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());

    // Handle the end life of the message box
    connect(activeMessageBox, &QMessageBox::finished, this,
        [this]()
        {
            activeMessageBox = nullptr;
            update();
        });

    // Show the message
    activeMessageBox->show();

    // Block the program while the message is blocking
    if (message.isBlocking)
    {
        QEventLoop loop;
        connect(activeMessageBox, &QMessageBox::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

void ErrorHandler::showMessage(const std::string& message, const std::string& name, const MESSAGEBOX_TYPE& type, bool isBlocking)
{
	// Push the message to display in the queue
	std::lock_guard<std::mutex> lock(mutex);
	messageQueue.push({ message, name, type, isBlocking });
}
#pragma endregion


#pragma region UPDATE
void ErrorHandler::update()
{
	// Display all the messages
	if (!activeMessageBox)
	{
		MessageLogger m;
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (messageQueue.empty())
				return;

			m = std::move(messageQueue.front());
			messageQueue.pop();
		}
		// Create the message box
		createMessageBox(m);
	}
}
#pragma endregion


#pragma region PUBLIC MESSAGES
void ErrorHandler::showInfo(const std::string& message, bool isBlocking)
{
    showMessage(message, "VideoDecoder - Info", MESSAGEBOX_TYPE::INFO, isBlocking);
}

void ErrorHandler::showQuestion(const std::string& message, bool isBlocking)
{
    showMessage(message, "VideoDecoder - Question", MESSAGEBOX_TYPE::QUESTION, isBlocking);
}

void ErrorHandler::showWarning(const std::string& message, bool isBlocking)
{
	showMessage(message, "VideoDecoder - Warning", MESSAGEBOX_TYPE::WARNING, isBlocking);
}

void ErrorHandler::showError(const std::string& message, bool isBlocking)
{
	showMessage(message, "VideoDecoder - Error", MESSAGEBOX_TYPE::CRITICAL, isBlocking);
}
#pragma endregion
