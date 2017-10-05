#pragma once

#include <curl/curl.h>

#include <deque>
#include <mutex>

namespace lift
{

/**
 * Creating CURL* handles is expensive and can cause heavy CPU usage
 * in the kernel from spin locks on /dev/urandom etc.  This pool will create
 * additional CURL* handles as needed but otherwise will re-use CURL* handles
 * when they are not in use.
 *
 * The Request class's destructor will push the CURL* back into this pool to be
 * re-used.
 *
 * This class uses a mutex for cross thread safety when a CURL* pointer is being returned.
 */
class CurlPool
{
public:
    CurlPool() = default;
    ~CurlPool(); ///< Required to call curl_easy_cleanup() on every handle.

    CurlPool(const CurlPool& copy) = delete;                                ///< No copying
    CurlPool(CurlPool&& move) = default;                                    ///< Can move
    auto operator = (const CurlPool& copy_assign) -> CurlPool& = delete;    ///< No copy assign
    auto operator = (CurlPool&& move_assign) -> CurlPool& = default;        ///< Can move assign

    /**
     * Produces a CURL* handle.
     *
     * This function is thread safe.
     *
     * @return CURL*
     */
    auto Produce() -> CURL*;

    /**
     * Returns a CURL* handle to the pool.  This function is called when a Request
     * object destructs so the CURL* handle can be re-used.
     *
     * This function is thread safe.
     *
     * @param curl_handle CURL* handle to return to the pool.
     */
    auto Return(CURL* curl_handle) -> void;

private:
    std::mutex m_lock;                  ///< Used for thread safe calls.
    std::deque<CURL*> m_curl_handles;   ///< Pool of un-used CURL* handles.
};


} // lift
