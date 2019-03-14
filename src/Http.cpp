#include "lift/Http.h"

namespace lift::http {

static const std::string STATUS_CODE_HTTP_100_CONTINUE = "100 Continue";
static const std::string STATUS_CODE_HTTP_101_SWITCHING_PROTOCOLS = "101 Switching Protocols";
static const std::string STATUS_CODE_HTTP_102_PROCESSING = "102 Processing";
static const std::string STATUS_CODE_HTTP_103_EARLY_HINTS = "103 Early Hints";
static const std::string STATUS_CODE_HTTP_200_OK = "200 OK";
static const std::string STATUS_CODE_HTTP_201_CREATED = "201 Created";
static const std::string STATUS_CODE_HTTP_202_ACCEPTED = "202 Accepted";
static const std::string STATUS_CODE_HTTP_203_NON_AUTHORITATIVE_INFORMATION = "203 Non-Authoritative Information";
static const std::string STATUS_CODE_HTTP_204_NO_CONTENT = "204 No Content";
static const std::string STATUS_CODE_HTTP_205_RESET_CONTENT = "205 Reset Content";
static const std::string STATUS_CODE_HTTP_206_PARTIAL_CONTENT = "206 Partial Content";
static const std::string STATUS_CODE_HTTP_207_MULTI_STATUS = "207 Multi-Status";
static const std::string STATUS_CODE_HTTP_208_ALREADY_REPORTED = "208 Already Reported";
static const std::string STATUS_CODE_HTTP_226_IM_USED = "226 IM Used";
static const std::string STATUS_CODE_HTTP_300_MULTIPLE_CHOICES = "300 Multiple Choices";
static const std::string STATUS_CODE_HTTP_301_MOVED_PERMANENTLY = "301 Moved Permanently";
static const std::string STATUS_CODE_HTTP_302_FOUND = "302 Found";
static const std::string STATUS_CODE_HTTP_303_SEE_OTHER = "303 See Other";
static const std::string STATUS_CODE_HTTP_304_NOT_MODIFIED = "304 Not Modified";
static const std::string STATUS_CODE_HTTP_305_USE_PROXY = "305 Use Proxy";
static const std::string STATUS_CODE_HTTP_306_SWITCH_PROXY = "306 Switch Proxy";
static const std::string STATUS_CODE_HTTP_307_TEMPORARY_REDIRECT = "307 Temporary Redirect";
static const std::string STATUS_CODE_HTTP_308_PERMANENT_REDIRECT = "308 Permanent Redirect";
static const std::string STATUS_CODE_HTTP_400_BAD_REQUEST = "400 Bad Request";
static const std::string STATUS_CODE_HTTP_401_UNAUTHORIZED = "401 Unauthorized";
static const std::string STATUS_CODE_HTTP_402_PAYMENT_REQUIRED = "402 Payment Required";
static const std::string STATUS_CODE_HTTP_403_FORBIDDEN = "403 Forbidden";
static const std::string STATUS_CODE_HTTP_404_NOT_FOUND = "404 Not Found";
static const std::string STATUS_CODE_HTTP_405_METHOD_NOT_ALLOWED = "405 Method Not Allowed";
static const std::string STATUS_CODE_HTTP_406_NOT_ACCEPTABLE = "406 Not Acceptable";
static const std::string STATUS_CODE_HTTP_407_PROXY_AUTHENTICATION_REQUIRED = "407 Proxy Authentication Required";
static const std::string STATUS_CODE_HTTP_408_REQUEST_TIMEOUT = "408 Request Timeout";
static const std::string STATUS_CODE_HTTP_409_CONFLICT = "409 Conflict";
static const std::string STATUS_CODE_HTTP_410_GONE = "410 Gone";
static const std::string STATUS_CODE_HTTP_411_LENGTH_REQUIRED = "411 Length Required";
static const std::string STATUS_CODE_HTTP_412_PRECONDITION_FAILED = "412 Precondition Failed";
static const std::string STATUS_CODE_HTTP_413_PAYLOAD_TOO_LARGE = "413 Payload Too Large";
static const std::string STATUS_CODE_HTTP_414_URI_TOO_LONG = "414 URI Too Long";
static const std::string STATUS_CODE_HTTP_415_UNSUPPORTED_MEDIA_TYPE = "415 Unsupported Media Type";
static const std::string STATUS_CODE_HTTP_416_RANGE_NOT_SATISFIABLE = "416 Range Not Satisfiable";
static const std::string STATUS_CODE_HTTP_417_EXPECTATION_FAILED = "417 Expectation Failed";
static const std::string STATUS_CODE_HTTP_418_IM_A_TEAPOT_ = "418 I'm a teapot";
static const std::string STATUS_CODE_HTTP_421_MISDIRECTED_REQUEST = "421 Misdirected Request";
static const std::string STATUS_CODE_HTTP_422_UNPROCESSABLE_ENTITY = "422 Unprocessable Entity";
static const std::string STATUS_CODE_HTTP_423_LOCKED = "423 Locked";
static const std::string STATUS_CODE_HTTP_424_FAILED_DEPENDENCY = "424 Failed Dependency";
static const std::string STATUS_CODE_HTTP_426_UPGRADE_REQUIRED = "426 Upgrade Required";
static const std::string STATUS_CODE_HTTP_428_PRECONDITION_FAILED = "428 Precondition Required";
static const std::string STATUS_CODE_HTTP_429_TOO_MANY_REQUESTS = "429 Too Many Requests";
static const std::string STATUS_CODE_HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE = "431 Request Header Fields Too Large";
static const std::string STATUS_CODE_HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS = "451 Unavailable For Legal Reasons";
static const std::string STATUS_CODE_HTTP_500_INTERNAL_SERVER_ERROR = "500 Internal Server Error";
static const std::string STATUS_CODE_HTTP_501_NOT_IMPLEMENTED = "501 Not Implemented";
static const std::string STATUS_CODE_HTTP_502_BAD_GATEWAY = "502 Bad Gateway";
static const std::string STATUS_CODE_HTTP_503_SERVICE_UNAVAILABLE = "503 Service Unavailable";
static const std::string STATUS_CODE_HTTP_504_GATEWAY_TIMEOUT = "504 Gateway Timeout";
static const std::string STATUS_CODE_HTTP_505_HTTP_VERSION_NOT_SUPPORTED = "505 HTTP Version Not Supported";
static const std::string STATUS_CODE_HTTP_506_VARIANT_ALSO_NEGOTIATES = "506 Variant Also Negotiates";
static const std::string STATUS_CODE_HTTP_507_INSUFFICIENT_STORAGE = "507 Insufficient Storage";
static const std::string STATUS_CODE_HTTP_508_LOOP_DETECTED = "508 Loop Detected";
static const std::string STATUS_CODE_HTTP_510_NOT_EXTENDED = "510 Not Extended";
static const std::string STATUS_CODE_HTTP_511_NETWORK_AUTHENTICATION_REQUIRED = "511 Network Authentication Required";

static const std::string STATUS_CODE_HTTP_UNKNOWN = "UNKNOWN";

auto to_string(
    StatusCode code) -> const std::string&
{
    switch (code) {
    case StatusCode::HTTP_100_CONTINUE:
        return STATUS_CODE_HTTP_100_CONTINUE;
    case StatusCode::HTTP_101_SWITCHING_PROTOCOLS:
        return STATUS_CODE_HTTP_101_SWITCHING_PROTOCOLS;
    case StatusCode::HTTP_102_PROCESSING:
        return STATUS_CODE_HTTP_102_PROCESSING;
    case StatusCode::HTTP_103_EARLY_HINTS:
        return STATUS_CODE_HTTP_103_EARLY_HINTS;
    case StatusCode::HTTP_200_OK:
        return STATUS_CODE_HTTP_200_OK;
    case StatusCode::HTTP_201_CREATED:
        return STATUS_CODE_HTTP_201_CREATED;
    case StatusCode::HTTP_202_ACCEPTED:
        return STATUS_CODE_HTTP_202_ACCEPTED;
    case StatusCode::HTTP_203_NON_AUTHORITATIVE_INFORMATION:
        return STATUS_CODE_HTTP_203_NON_AUTHORITATIVE_INFORMATION;
    case StatusCode::HTTP_204_NO_CONTENT:
        return STATUS_CODE_HTTP_204_NO_CONTENT;
    case StatusCode::HTTP_205_RESET_CONTENT:
        return STATUS_CODE_HTTP_205_RESET_CONTENT;
    case StatusCode::HTTP_206_PARTIAL_CONTENT:
        return STATUS_CODE_HTTP_206_PARTIAL_CONTENT;
    case StatusCode::HTTP_207_MULTI_STATUS:
        return STATUS_CODE_HTTP_207_MULTI_STATUS;
    case StatusCode::HTTP_208_ALREADY_REPORTED:
        return STATUS_CODE_HTTP_208_ALREADY_REPORTED;
    case StatusCode::HTTP_226_IM_USED:
        return STATUS_CODE_HTTP_226_IM_USED;
    case StatusCode::HTTP_300_MULTIPLE_CHOICES:
        return STATUS_CODE_HTTP_300_MULTIPLE_CHOICES;
    case StatusCode::HTTP_301_MOVED_PERMANENTLY:
        return STATUS_CODE_HTTP_301_MOVED_PERMANENTLY;
    case StatusCode::HTTP_302_FOUND:
        return STATUS_CODE_HTTP_302_FOUND;
    case StatusCode::HTTP_303_SEE_OTHER:
        return STATUS_CODE_HTTP_303_SEE_OTHER;
    case StatusCode::HTTP_304_NOT_MODIFIED:
        return STATUS_CODE_HTTP_304_NOT_MODIFIED;
    case StatusCode::HTTP_305_USE_PROXY:
        return STATUS_CODE_HTTP_305_USE_PROXY;
    case StatusCode::HTTP_306_SWITCH_PROXY:
        return STATUS_CODE_HTTP_306_SWITCH_PROXY;
    case StatusCode::HTTP_307_TEMPORARY_REDIRECT:
        return STATUS_CODE_HTTP_307_TEMPORARY_REDIRECT;
    case StatusCode::HTTP_308_PERMANENT_REDIRECT:
        return STATUS_CODE_HTTP_308_PERMANENT_REDIRECT;
    case StatusCode::HTTP_400_BAD_REQUEST:
        return STATUS_CODE_HTTP_400_BAD_REQUEST;
    case StatusCode::HTTP_401_UNAUTHORIZED:
        return STATUS_CODE_HTTP_401_UNAUTHORIZED;
    case StatusCode::HTTP_402_PAYMENT_REQUIRED:
        return STATUS_CODE_HTTP_402_PAYMENT_REQUIRED;
    case StatusCode::HTTP_403_FORBIDDEN:
        return STATUS_CODE_HTTP_403_FORBIDDEN;
    case StatusCode::HTTP_404_NOT_FOUND:
        return STATUS_CODE_HTTP_404_NOT_FOUND;
    case StatusCode::HTTP_405_METHOD_NOT_ALLOWED:
        return STATUS_CODE_HTTP_405_METHOD_NOT_ALLOWED;
    case StatusCode::HTTP_406_NOT_ACCEPTABLE:
        return STATUS_CODE_HTTP_406_NOT_ACCEPTABLE;
    case StatusCode::HTTP_407_PROXY_AUTHENTICATION_REQUIRED:
        return STATUS_CODE_HTTP_407_PROXY_AUTHENTICATION_REQUIRED;
    case StatusCode::HTTP_408_REQUEST_TIMEOUT:
        return STATUS_CODE_HTTP_408_REQUEST_TIMEOUT;
    case StatusCode::HTTP_409_CONFLICT:
        return STATUS_CODE_HTTP_409_CONFLICT;
    case StatusCode::HTTP_410_GONE:
        return STATUS_CODE_HTTP_410_GONE;
    case StatusCode::HTTP_411_LENGTH_REQUIRED:
        return STATUS_CODE_HTTP_411_LENGTH_REQUIRED;
    case StatusCode::HTTP_412_PRECONDITION_FAILED:
        return STATUS_CODE_HTTP_412_PRECONDITION_FAILED;
    case StatusCode::HTTP_413_PAYLOAD_TOO_LARGE:
        return STATUS_CODE_HTTP_413_PAYLOAD_TOO_LARGE;
    case StatusCode::HTTP_414_URI_TOO_LONG:
        return STATUS_CODE_HTTP_414_URI_TOO_LONG;
    case StatusCode::HTTP_415_UNSUPPORTED_MEDIA_TYPE:
        return STATUS_CODE_HTTP_415_UNSUPPORTED_MEDIA_TYPE;
    case StatusCode::HTTP_416_RANGE_NOT_SATISFIABLE:
        return STATUS_CODE_HTTP_416_RANGE_NOT_SATISFIABLE;
    case StatusCode::HTTP_417_EXPECTATION_FAILED:
        return STATUS_CODE_HTTP_417_EXPECTATION_FAILED;
    case StatusCode::HTTP_418_IM_A_TEAPOT_:
        return STATUS_CODE_HTTP_418_IM_A_TEAPOT_;
    case StatusCode::HTTP_421_MISDIRECTED_REQUEST:
        return STATUS_CODE_HTTP_421_MISDIRECTED_REQUEST;
    case StatusCode::HTTP_422_UNPROCESSABLE_ENTITY:
        return STATUS_CODE_HTTP_422_UNPROCESSABLE_ENTITY;
    case StatusCode::HTTP_423_LOCKED:
        return STATUS_CODE_HTTP_423_LOCKED;
    case StatusCode::HTTP_424_FAILED_DEPENDENCY:
        return STATUS_CODE_HTTP_424_FAILED_DEPENDENCY;
    case StatusCode::HTTP_426_UPGRADE_REQUIRED:
        return STATUS_CODE_HTTP_426_UPGRADE_REQUIRED;
    case StatusCode::HTTP_428_PRECONDITION_FAILED:
        return STATUS_CODE_HTTP_428_PRECONDITION_FAILED;
    case StatusCode::HTTP_429_TOO_MANY_REQUESTS:
        return STATUS_CODE_HTTP_429_TOO_MANY_REQUESTS;
    case StatusCode::HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE:
        return STATUS_CODE_HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE;
    case StatusCode::HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS:
        return STATUS_CODE_HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS;
    case StatusCode::HTTP_500_INTERNAL_SERVER_ERROR:
        return STATUS_CODE_HTTP_500_INTERNAL_SERVER_ERROR;
    case StatusCode::HTTP_501_NOT_IMPLEMENTED:
        return STATUS_CODE_HTTP_501_NOT_IMPLEMENTED;
    case StatusCode::HTTP_502_BAD_GATEWAY:
        return STATUS_CODE_HTTP_502_BAD_GATEWAY;
    case StatusCode::HTTP_503_SERVICE_UNAVAILABLE:
        return STATUS_CODE_HTTP_503_SERVICE_UNAVAILABLE;
    case StatusCode::HTTP_504_GATEWAY_TIMEOUT:
        return STATUS_CODE_HTTP_504_GATEWAY_TIMEOUT;
    case StatusCode::HTTP_505_HTTP_VERSION_NOT_SUPPORTED:
        return STATUS_CODE_HTTP_505_HTTP_VERSION_NOT_SUPPORTED;
    case StatusCode::HTTP_506_VARIANT_ALSO_NEGOTIATES:
        return STATUS_CODE_HTTP_506_VARIANT_ALSO_NEGOTIATES;
    case StatusCode::HTTP_507_INSUFFICIENT_STORAGE:
        return STATUS_CODE_HTTP_507_INSUFFICIENT_STORAGE;
    case StatusCode::HTTP_508_LOOP_DETECTED:
        return STATUS_CODE_HTTP_508_LOOP_DETECTED;
    case StatusCode::HTTP_510_NOT_EXTENDED:
        return STATUS_CODE_HTTP_510_NOT_EXTENDED;
    case StatusCode::HTTP_511_NETWORK_AUTHENTICATION_REQUIRED:
        return STATUS_CODE_HTTP_511_NETWORK_AUTHENTICATION_REQUIRED;
    default:
        return STATUS_CODE_HTTP_UNKNOWN;
    }
}

auto to_enum(
    uint32_t code) -> StatusCode
{
    return static_cast<StatusCode>(code);
}

} // namespace lift::http
