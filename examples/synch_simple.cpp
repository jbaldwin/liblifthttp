#include <lift/Lift.h>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init {};

    lift::RequestPool request_pool;
    {
        auto request = request_pool.Produce("http://www.example.com");
        std::cout << "Requesting http://www.example.com" << std::endl;
        request->Perform();
        std::cout << request->GetResponseData() << std::endl;
        // when the request destructs it will return to the pool automatically
    }

    {
        // this request object will be the same one as above recycled through the pool
        auto request = request_pool.Produce("http://www.google.com");
        std::cout << "Requesting http://www.google.com" << std::endl;
        request->Perform();
        std::cout << request->GetResponseData() << std::endl;
    }

    return 0;
}
