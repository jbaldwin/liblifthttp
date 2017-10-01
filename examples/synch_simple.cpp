#include <lift/Lift.h>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::RequestPool request_pool;
    {
        auto request = request_pool.Produce("http://www.example.com");
        std::cout << "Requesting http://www.example.com" << std::endl;
        request->Perform();
        std::cout << request->GetResponseData() << std::endl;
        request_pool.Return(std::move(request));
    }

    {
        auto request = request_pool.Produce("http://www.google.com");
        std::cout << "Requesting http://www.google.com" << std::endl;
        request->Perform();
        std::cout << request->GetResponseData() << std::endl;
        request_pool.Return(std::move(request));
    }

    lift::cleanup();

    return 0;
}
