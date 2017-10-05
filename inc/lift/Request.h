#pragma once

#include "lift/RequestHandle.h"

#include <memory>

namespace lift
{

class RequestPool;

class Request
{
    friend class RequestPool;
public:

    ~Request();
    Request(Request&& from) = default;
    auto operator = (Request&&) -> Request& = default;

    auto operator * () -> RequestHandle&;
    auto operator * () const -> const RequestHandle&;
    auto operator -> () -> RequestHandle*;
    auto operator -> () const -> const RequestHandle*;

private:
    Request(
        RequestPool& request_pool,
        std::unique_ptr<RequestHandle> request_handle
    );

    Request(const Request&) = delete;
    auto operator = (const Request&) = delete;

    RequestPool& m_request_pool;
    std::unique_ptr<RequestHandle> m_request_handle;
};

} // lift
