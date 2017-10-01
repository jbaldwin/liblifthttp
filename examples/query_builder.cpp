#include <lift/Lift.h>

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::RequestPool request_pool;

    lift::QueryBuilder query_builder;
    query_builder
        .SetScheme("https")
        .SetHostname("www.example.com")
        .SetPort(443)
        .AppendPathPart("test")
        .AppendPathPart("path")
        .AppendPathPart("param1", "value1")
        .AppendPathPart("param2", "value2");

    auto url = query_builder.Build();
    auto request = request_pool.Produce(url);
    std::cout << "Requesting " << url << std::endl;
    request->Perform();
    std::cout << request->GetResponseData() << std::endl;
    request_pool.Return(std::move(request));

    /**
     * The query builder can be re-used after calling Build()
     */
    query_builder
        .SetScheme("https")
        .SetHostname("www.reddit.com")
        .SetPort(443)
        .AppendPathPart("r")
        .AppendPathPart("funny");
    url = query_builder.Build();
    request = request_pool.Produce(url);
    std::cout << "Requesting " << url << std::endl;
    request->Perform();
    std::cout << request->GetResponseData() << std::endl;
    request_pool.Return(std::move(request));

    lift::cleanup();

    return 0;
}
