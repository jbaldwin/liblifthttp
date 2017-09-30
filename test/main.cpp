#include "lift/Lift.h"

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    lift::initialize();

    lift::cleanup();

    return 0;
}
