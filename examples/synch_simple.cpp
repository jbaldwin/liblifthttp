#include <lift/Lift.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init {};

    {
        lift::Request request { "http://www.example.com" };
        std::cout << "Requesting http://www.example.com" << std::endl;
        const auto& response = request.Perform();
        std::cout << response.Data() << std::endl;
    }

    {
        lift::Request request { "http://www.google.com" };
        std::cout << "Requesting http://www.google.com" << std::endl;
        const auto& response = request.Perform();
        std::cout << response.Data() << std::endl;

        for (const auto& header : response.Headers()) {
            std::cout << header.Name() << ": " << header.Value() << "\n";
        }
    }

    return 0;
}
