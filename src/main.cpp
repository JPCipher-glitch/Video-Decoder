#include "engine.hpp"
#include <array>

int main(int argc, char* argv[])
{
    std::array<std::string, 2> arguments;

    for (int i = 0; i < argc; i++)
        arguments[i] = argv[i];

    Engine engine(arguments[1]);
    engine.run();

    return 0;
}
