#ifndef LOADING_HANDLER_HPP
#define LOADING_HANDLER_HPP

#include "video/loading/loading_widget.hpp"

#include <QObject>
#include <mutex>
#include <queue>

struct ProgressStuff
{
	double progress{ 0.0 };
};

class LoadingHandler : public QObject
{
	Q_OBJECT
private:
	bool cancelled{ false };

	std::string name;
	std::string status = "Initializing...";

	std::mutex mutex;
	std::queue<ProgressStuff> progressQueue;

	LoadingWidget* loadingWidget{ nullptr };

	void cancelledLoading();
	void finishedLoading();
public:
	LoadingHandler(const std::string& name);
	~LoadingHandler();

	double loadingWeightRatio{ 0.0 };
	bool isCancelled();
	void cancelLoading();

	void createLoadingWidget();
	void addProgress(double progress);
	void setStatus(const std::string& status);
	void update();
};

#endif