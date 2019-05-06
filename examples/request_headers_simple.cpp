#include <lift/Lift.h>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init {};

    lift::RequestPool request_pool;
    auto request = request_pool.Produce("http://www.example.com");
    request->AddHeader("x-test-header", "test_value");
    request->AddHeader("x-test-header-2", "test_value-2");
    request->AddHeader("x-test-header-3", "test_value-3");

    for (auto& header : request->GetRequestHeaders()) {
        std::cout << header.GetHeader() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Requesting http://www.example.com" << std::endl;
    request->Perform();
    std::cout << request->GetResponseData() << std::endl;

    return 0;
}
