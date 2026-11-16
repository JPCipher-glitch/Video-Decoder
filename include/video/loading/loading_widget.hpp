#ifndef LOADING_WIDGET_HPP
#define LOADING_WIDGET_HPP

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QElapsedTimer>
#include <QTimer>

class LoadingWidget : public QDialog
{
    Q_OBJECT
private:
    bool finished{ false };

    float speed{ 200.f };
    float shutdownDelay{ 1.f };
    double progress{ 0.0 };
    double gainProgress{ 0.0 };

    QLabel* videoLabel{ nullptr };
    QLabel* statusLabel{ nullptr };

    QTimer* timer{ nullptr };
    QElapsedTimer elapsedTimer;

    QProgressBar* progressBar{ nullptr };
    QPushButton* cancelButton{ nullptr };

    void cancelLoading();
    void finishLoading();
    void updateProgress();
signals:
    void cancelRequested();
    void finishRequested();
public:
    explicit LoadingWidget(const std::string& videoName, QWidget* parent = nullptr);

    void addProgress(double progress);
    void setStatus(const QString& status);
};
#endif