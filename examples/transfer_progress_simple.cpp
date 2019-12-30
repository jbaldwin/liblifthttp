#include <lift/Lift.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init{};

    lift::RequestPool request_pool;
    {
        auto request = request_pool.Produce("http://www.example.com");
        std::cout << "Requesting http://www.example.com" << std::endl;

        request->SetTransferProgressHandler(
            [](const lift::Request& r, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) -> bool {
                std::cout
                    << r.GetUrl() << " download_total_bytes:" << dltotal << " download_now_bytes: " << dlnow
                    << " upload_total_bytes: " << ultotal << " upload_now_bytes: " << ulnow << std::endl;

                return true; // continue the request
            });

        request->Perform();
        std::cout << request->GetResponseData() << std::endl;
        // when the request destructs it will return to the pool automatically
    }

    return 0;
}
