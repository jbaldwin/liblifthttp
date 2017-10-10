#include <lift/Lift.h>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

class UserData
{
public:
    explicit UserData(uint64_t request_id) : m_request_id(request_id) { }
    uint64_t m_request_id = 0;
};

class CompletedCtx : public lift::IRequestCb
{
public:
    auto OnComplete(lift::Request request) -> void override
    {
        std::cout << request->GetUrl() << std::endl;
        UserData* user_data = request->GetUserData<UserData>();
        if(user_data)
        {
            std::cout << "Request id " << user_data->m_request_id << " has completed." << std::endl;
            delete user_data;
        }
        else
        {
            std::cout << "user data was nullptr" << std::endl;
        }
    }
};

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    using namespace std::chrono_literals;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::EventLoop event_loop(std::make_unique<CompletedCtx>());
    auto& request_pool = event_loop.GetRequestPool();
    auto req1 = request_pool.Produce("http://www.example.com", 1000ms);
    req1->SetUserData(new UserData(1));
    event_loop.AddRequest(std::move(req1));

    auto req2 = request_pool.Produce("http://www.reddit.com", 1000ms);
    req2->SetUserData(new UserData(2));
    event_loop.AddRequest(std::move(req2));

    std::this_thread::sleep_for(500ms);

    while(event_loop.GetActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(10ms);
    }
    event_loop.Stop();

    lift::cleanup();

    return 0;
}
