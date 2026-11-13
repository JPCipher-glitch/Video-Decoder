#include "engine.hpp"
#include <array>

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    std::array<std::string, 2> arguments;

    for (int i = 0; i < argc; i++)
        arguments[i] = argv[i];

    QApplication app(argc, argv);
    app.setApplicationName("Video-Decoder");
    app.setWindowIcon(QIcon(":/icons/desk_icon"));
    app.setQuitOnLastWindowClosed(false);

    Engine engine(arguments[1]);
    engine.run();

    return 0;
}