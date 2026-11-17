#include "video/loading/loading_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>

#pragma region CONSTRUCTOR
LoadingWidget::LoadingWidget(const std::string& videoName, QWidget* parent) : QDialog(parent)
{
    // Main Settings
    setWindowTitle("Video Decoder - Loading");
    setFixedSize(350, 120);
    setStyleSheet(
        R"(
            QDialog
            {
                background-color: #202020;
                color: #FFFFFF;
            }

            QLabel
            {
                color: #DDDDDD;
                font-size: 12px;
            }

            QProgressBar
            {
                border: 1px solid #404040;
                background-color: #151515;
                text-align: center;
                color: white;
                height: 12px;
            }

            QProgressBar::chunk
            {
                background-color: #E97034;
            }

            QPushButton
            {
                background-color: #353535;
                color: white;
                border: 1px solid #505050;
                border-radius: 4px;
                padding: 5px 15px;
            }

            QPushButton:hover
            {
                background-color: #454545;
            }
        )");
    // #E13C3C
    // #E97034
    // #3985DC

    // Deltatime
    elapsedTimer.start();

    // Timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoadingWidget::updateProgress);
    timer->start(16);

    // Initialize the text labels
    videoLabel = new QLabel(QString("<p align='center'>Loading \"%1\" cache</p>").arg(videoName), this);
    statusLabel = new QLabel("<p align='center'>Initializing...</p>", this);
    statusLabel->setStyleSheet(
        R"(
            QLabel
            {
                color: #FFEB96;
                font-size: 12px;
            }
        )");

    // Create the progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    // Cancel Button
    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setFixedSize(QSize(96, 22));
    connect(cancelButton, &QPushButton::clicked, this, &LoadingWidget::cancelRequested);

    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);

    // Main Layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(videoLabel);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addLayout(buttonLayout);

    // Cancel Signal
    connect(this, &LoadingWidget::cancelRequested, this, &LoadingWidget::cancelLoading);
    // Finish Signal
    connect(this, &LoadingWidget::finishRequested, this, &LoadingWidget::finishLoading);
}
#pragma endregion

#pragma region CANCEL
void LoadingWidget::cancelLoading()
{
    if (finished) return;

    finished = true;
    cancelButton->setEnabled(false);
    timer->stop();

    statusLabel->setText("<p align='center'>Cancelling...</p>");

    QTimer::singleShot(shutdownDelay * 1000, this, &QObject::deleteLater);
    emit cancelRequested();
}
#pragma endregion

#pragma region FINISH
void LoadingWidget::finishLoading()
{
    if (finished) return;

    finished = true;
    cancelButton->setEnabled(false);
    timer->stop();

    statusLabel->setText("<p align='center'>Finished...</p>");

    QTimer::singleShot(shutdownDelay * 1000, this, &QObject::deleteLater);
    emit finishRequested();
}
#pragma endregion

#pragma region STATUS
void LoadingWidget::setStatus(const QString& status)
{
    if (progress >= 100) return;

    statusLabel->setText(QString("<p align='center'>%1</p>").arg(status));
}
#pragma endregion

#pragma region PROGRESS
void LoadingWidget::addProgress(double p)
{
    if (progress >= 100) return;

    gainProgress += p;
}

void LoadingWidget::updateProgress()
{
    // Calculate the delta time
    float dt = elapsedTimer.restart() / 1000.f;
    double progressDelta = static_cast<double>(dt) * 50.0;

    // Add step by step the progress into the widget
    if (gainProgress > 0.f)
    {
        double step = std::min(gainProgress, progressDelta); // Prevent of using the actual deltatime when the gain progress is smaller

        gainProgress -= step;
        progress += step;
    }

    // Set the new value to the widget
    progress = std::min(progress, 100.0);
    progressBar->setValue(static_cast<int>(progress));

    // Finish the loading when the progress reach 100%
    if (progress >= 100 && !finished)
        finishRequested();
}
#pragma endregion