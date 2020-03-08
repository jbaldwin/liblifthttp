#include "lift/Response.hpp"
#include "lift/Const.hpp"

namespace lift {

Response::Response()
{
    m_headers.reserve(HEADER_DEFAULT_MEMORY_BYTES);
    m_headers_idx.reserve(HEADER_DEFAULT_COUNT);
    m_data.reserve(HEADER_DEFAULT_MEMORY_BYTES);
}

} // namespace lift
