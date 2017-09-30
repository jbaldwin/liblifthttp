#include <lift/Lift.h>

#include <iostream>

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::Request request("http://www.example.com");
    request.Perform();
    std::cout << request.GetResponseData() << std::endl;

    request.Reset();
    request.SetUrl("http://www.google.com");
    request.Perform();
    std::cout << request.GetResponseData() << std::endl;

    lift::cleanup();

    return 0;
}
