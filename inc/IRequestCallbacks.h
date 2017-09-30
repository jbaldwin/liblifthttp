#pragma once

/**
 * Interface for the set of possible asynchronous request callbacks when
 * the request completes/times out/errors.
 */
class IRequestCallbacks
{
public:
    virtual ~IRequestCallbacks() = default;

    /**
     * Request callback on connection timeout.
     * @param request The request that failed to connect.
     */
    virtual auto OnConnectTimeout(
        std::unique_ptr<AsyncRequest> request
    ) -> void = 0;

    /**
     * Request callback on successful completion.
     * @param request The request that is finished.
     */
    virtual auto OnComplete(
        std::unique_ptr<AsyncRequest> request
    ) -> void = 0;

    /**
     * Request callback on timeout.  See Request::SetTimeoutMilliseconds().
     * @param request The request that timed out.
     */
    virtual auto OnTimeout(
        std::unique_ptr<AsyncRequest> request
    ) -> void = 0;

    /**
     * Request callback on error.
     * @param request The request that had an error while processing.
     */
    virtual auto OnError(
        std::unique_ptr<AsyncRequest> request
    ) -> void = 0;
};
