#include "video/loading/loading_handler.hpp"

#pragma region CONSTRUCTOR
LoadingHandler::LoadingHandler(const std::string& name) : name{ name }
{
	// Create loading
	createLoadingWidget();
}
#pragma endregion

#pragma region DESTRUCTOR
LoadingHandler::~LoadingHandler()
{
	if (loadingWidget)
	{
		disconnect(loadingWidget, nullptr, this, nullptr);
		delete loadingWidget;
		loadingWidget = nullptr;
	}
}
#pragma endregion

#pragma region CANCELLED
bool LoadingHandler::isCancelled()
{
	return cancelled;
}
#pragma endregion


#pragma region CREATE LOADING
void LoadingHandler::createLoadingWidget()
{
	if (loadingWidget) return;

	// Create loading widget
	loadingWidget = new LoadingWidget(name);
	loadingWidget->show();

	// Cancel Signal
	connect(loadingWidget, &LoadingWidget::cancelRequested, this, &LoadingHandler::cancelledLoading);
	// Finish Signal
	connect(loadingWidget, &LoadingWidget::finishRequested, this, &LoadingHandler::finishedLoading);
}
#pragma endregion

#pragma region CANCEL LOADING
void LoadingHandler::cancelledLoading()
{
	// Cancel the loading and ask to the video to close
	cancelled = true;
	finishedLoading();
}

void LoadingHandler::cancelLoading()
{
	if (!loadingWidget) return;
	// Cancel the loading and ask to the video to close
	emit loadingWidget->finishRequested();
}
#pragma endregion

#pragma region FINISH LOADING
void LoadingHandler::finishedLoading()
{
	loadingWidget = nullptr;
}
#pragma endregion


#pragma region PROGRESS
void LoadingHandler::addProgress(double progress)
{
	if (!loadingWidget) return;

	// Push the new progress gain into the queue
	std::lock_guard<std::mutex> lock(mutex);
	progressQueue.push({ progress });
}
#pragma endregion

#pragma region STATUS
void LoadingHandler::setStatus(const std::string& s)
{
	if (!loadingWidget) return;

	// Change the status name
	std::lock_guard<std::mutex> lock(mutex);
	status = s;
}
#pragma endregion

#pragma region UPDATE
void LoadingHandler::update()
{
	if (!loadingWidget) return;

	// Apply all progress gain in the widget
	ProgressStuff stuff;
	{
		std::lock_guard<std::mutex> lock(mutex); // Lock the thread when using a queue element
		if (progressQueue.empty()) // Stop the process if the queue is empty
			return;

		// Get the progress stuff
		stuff = std::move(progressQueue.front());
		progressQueue.pop();

		// Apply the change in the widget
		loadingWidget->setStatus(QString::fromStdString(status));
		loadingWidget->addProgress(stuff.progress);
	}
}
#pragma endregion