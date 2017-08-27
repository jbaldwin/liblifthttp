#include "AsyncRequest.h"

#include <iostream>

AsyncRequest::AsyncRequest()
    :
        Request()
{

}

AsyncRequest::AsyncRequest(const std::string& url)
    :
        Request(url)
{

}

AsyncRequest::~AsyncRequest()
{

}
