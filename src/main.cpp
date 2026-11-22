#include "engine.hpp"
#include "instance_manager.hpp"

#include <QApplication>
#include <QIcon>
#include <array>

int main(int argc, char* argv[])
{
    // Check if another instance of this program was launched
    InstanceManager instanceManager;
    if (instanceManager.isAnotherInstanceRunning()) // Shutdown the program when another instance was found
        return 1;

    // Get the extra arguments
    std::array<std::string, 2> arguments;
    for (int i = 0; i < argc; i++)
        arguments[i] = argv[i];

    // Create the QT app
    QApplication app(argc, argv);
    app.setApplicationName("Video-Decoder");
    app.setWindowIcon(QIcon(":/icons/desk_icon"));
    app.setQuitOnLastWindowClosed(false);

    // Run the engine
    Engine engine(arguments[1]);
    engine.run();

    return 0;
}