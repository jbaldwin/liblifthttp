#pragma once

#include <string>

#include <curl/curl.h>

namespace lift::http {

extern const std::string METHOD_UNKNOWN; // = "UNKNOWN"s;
extern const std::string METHOD_GET; // = "GET"s;
extern const std::string METHOD_HEAD; // = "HEAD"s;
extern const std::string METHOD_POST; // = "POST"s;
extern const std::string METHOD_PUT; // = "PUT"s;
extern const std::string METHOD_DELETE; // = "DELETE"s;
extern const std::string METHOD_CONNECT; // = "CONNECT"s;
extern const std::string METHOD_OPTIONS; // = "OPTIONS"s;
extern const std::string METHOD_PATCH; // = "PATCH"s;

enum class Method {
    UNKNOWN,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    PATCH
};

auto to_string(
    Method method) -> const std::string&;

// Some liberty is taken on the version strings where they don't match the specification.

extern const std::string VERSION_UNKNOWN; // = "HTTP/unknown"s;
extern const std::string VERSION_USE_BEST; // = "HTTP/Best"s;
extern const std::string VERSION_V1_0; // = "HTTP/1.0"s;
extern const std::string VERSION_V1_1; // = "HTTP/1.1"s;
extern const std::string VERSION_V2_0; // = "HTTP/2.0"s;
extern const std::string VERSION_V2_0_TLS; //= "HTTP/2.0-TLS"s;
extern const std::string VERSION_V2_0_ONLY; // = "HTTP/2.0-only"s;

enum class Version {
    UNKNOWN = -9999,
    /// Use the best version available.
    USE_BEST = CURL_HTTP_VERSION_NONE,
    /// Use HTTP 1.0.
    V1_0 = CURL_HTTP_VERSION_1_0,
    /// Use HTTP 1.1.
    V1_1 = CURL_HTTP_VERSION_1_1,
    /// Attempt HTTP 2 requests but fallback to 1.1 on failure.
    V2_0 = CURL_HTTP_VERSION_2_0,
    /// Attempt HTTP 2 over TLS (HTTPS) but fallback to 1.1 on failure.
    V2_0_TLS = CURL_HTTP_VERSION_2TLS,
    /// Use HTTP 2.0 non-TLS with no fallback to 1.1.
    V2_0_ONLY = CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE
};

auto to_string(
    Version version) -> const std::string&;

extern const std::string STATUS_CODE_HTTP_UNKNOWN; // = "UNKNOWN"s;
extern const std::string STATUS_CODE_HTTP_100_CONTINUE; // = "100 Continue"s;
extern const std::string STATUS_CODE_HTTP_101_SWITCHING_PROTOCOLS; // = "101 Switching Protocols"s;
extern const std::string STATUS_CODE_HTTP_102_PROCESSING; // = "102 Processing"s;
extern const std::string STATUS_CODE_HTTP_103_EARLY_HINTS; // = "103 Early Hints"s;

extern const std::string STATUS_CODE_HTTP_200_OK; // = "200 OK"s;
extern const std::string STATUS_CODE_HTTP_201_CREATED; // = "201 Created"s;
extern const std::string STATUS_CODE_HTTP_202_ACCEPTED; // = "202 Accepted"s;
extern const std::string STATUS_CODE_HTTP_203_NON_AUTHORITATIVE_INFORMATION; // = "203 Non-Authoritative Information"s;
extern const std::string STATUS_CODE_HTTP_204_NO_CONTENT; // = "204 No Content"s;
extern const std::string STATUS_CODE_HTTP_205_RESET_CONTENT; // = "205 Reset Content"s;
extern const std::string STATUS_CODE_HTTP_206_PARTIAL_CONTENT; // = "206 Partial Content"s;
extern const std::string STATUS_CODE_HTTP_207_MULTI_STATUS; // = "207 Multi-Status"s;
extern const std::string STATUS_CODE_HTTP_208_ALREADY_REPORTED; // = "208 Already Reported"s;
extern const std::string STATUS_CODE_HTTP_226_IM_USED; // = "226 IM Used"s;

extern const std::string STATUS_CODE_HTTP_300_MULTIPLE_CHOICES; // = "300 Multiple Choices"s;
extern const std::string STATUS_CODE_HTTP_301_MOVED_PERMANENTLY; // = "301 Moved Permanently"s;
extern const std::string STATUS_CODE_HTTP_302_FOUND; // = "302 Found"s;
extern const std::string STATUS_CODE_HTTP_303_SEE_OTHER; // = "303 See Other"s;
extern const std::string STATUS_CODE_HTTP_304_NOT_MODIFIED; // = "304 Not Modified"s;
extern const std::string STATUS_CODE_HTTP_305_USE_PROXY; // = "305 Use Proxy"s;
extern const std::string STATUS_CODE_HTTP_306_SWITCH_PROXY; // = "306 Switch Proxy"s;
extern const std::string STATUS_CODE_HTTP_307_TEMPORARY_REDIRECT; // = "307 Temporary Redirect"s;
extern const std::string STATUS_CODE_HTTP_308_PERMANENT_REDIRECT; // = "308 Permanent Redirect"s;

extern const std::string STATUS_CODE_HTTP_400_BAD_REQUEST; // = "400 Bad Request"s;
extern const std::string STATUS_CODE_HTTP_401_UNAUTHORIZED; // = "401 Unauthorized"s;
extern const std::string STATUS_CODE_HTTP_402_PAYMENT_REQUIRED; // = "402 Payment Required"s;
extern const std::string STATUS_CODE_HTTP_403_FORBIDDEN; // = "403 Forbidden"s;
extern const std::string STATUS_CODE_HTTP_404_NOT_FOUND; // = "404 Not Found"s;
extern const std::string STATUS_CODE_HTTP_405_METHOD_NOT_ALLOWED; // = "405 Method Not Allowed"s;
extern const std::string STATUS_CODE_HTTP_406_NOT_ACCEPTABLE; // = "406 Not Acceptable"s;
extern const std::string STATUS_CODE_HTTP_407_PROXY_AUTHENTICATION_REQUIRED; // = "407 Proxy Authentication Required"s;
extern const std::string STATUS_CODE_HTTP_408_REQUEST_TIMEOUT; // = "408 Request Timeout"s;
extern const std::string STATUS_CODE_HTTP_409_CONFLICT; // = "409 Conflict"s;
extern const std::string STATUS_CODE_HTTP_410_GONE; // = "410 Gone"s;
extern const std::string STATUS_CODE_HTTP_411_LENGTH_REQUIRED; // = "411 Length Required"s;
extern const std::string STATUS_CODE_HTTP_412_PRECONDITION_FAILED; // = "412 Precondition Failed"s;
extern const std::string STATUS_CODE_HTTP_413_PAYLOAD_TOO_LARGE; // = "413 Payload Too Large"s;
extern const std::string STATUS_CODE_HTTP_414_URI_TOO_LONG; // = "414 URI Too Long"s;
extern const std::string STATUS_CODE_HTTP_415_UNSUPPORTED_MEDIA_TYPE; // = "415 Unsupported Media Type"s;
extern const std::string STATUS_CODE_HTTP_416_RANGE_NOT_SATISFIABLE; // = "416 Range Not Satisfiable"s;
extern const std::string STATUS_CODE_HTTP_417_EXPECTATION_FAILED; // = "417 Expectation Failed"s;
extern const std::string STATUS_CODE_HTTP_418_IM_A_TEAPOT; // = "418 I'm a teapot"s;
extern const std::string STATUS_CODE_HTTP_421_MISDIRECTED_REQUEST; // = "421 Misdirected Request"s;
extern const std::string STATUS_CODE_HTTP_422_UNPROCESSABLE_ENTITY; // = "422 Unprocessable Entity"s;
extern const std::string STATUS_CODE_HTTP_423_LOCKED; // = "423 Locked"s;
extern const std::string STATUS_CODE_HTTP_424_FAILED_DEPENDENCY; // = "424 Failed Dependency"s;
extern const std::string STATUS_CODE_HTTP_425_TOO_EARLY; // = "425 Too Early"s;
extern const std::string STATUS_CODE_HTTP_426_UPGRADE_REQUIRED; // = "426 Upgrade Required"s;
extern const std::string STATUS_CODE_HTTP_428_PRECONDITION_REQUIRED; // = "428 Precondition Required"s;
extern const std::string STATUS_CODE_HTTP_429_TOO_MANY_REQUESTS; // = "429 Too Many Requests"s;
extern const std::string STATUS_CODE_HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE; // = "431 Request Header Fields Too Large"s;
extern const std::string STATUS_CODE_HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS; // = "451 Unavailable For Legal Reasons"s;

extern const std::string STATUS_CODE_HTTP_500_INTERNAL_SERVER_ERROR; // = "500 Internal Server Error"s;
extern const std::string STATUS_CODE_HTTP_501_NOT_IMPLEMENTED; // = "501 Not Implemented"s;
extern const std::string STATUS_CODE_HTTP_502_BAD_GATEWAY; // = "502 Bad Gateway"s;
extern const std::string STATUS_CODE_HTTP_503_SERVICE_UNAVAILABLE; // = "503 Service Unavailable"s;
extern const std::string STATUS_CODE_HTTP_504_GATEWAY_TIMEOUT; // = "504 Gateway Timeout"s;
extern const std::string STATUS_CODE_HTTP_505_HTTP_VERSION_NOT_SUPPORTED; // = "505 HTTP Version Not Supported"s;
extern const std::string STATUS_CODE_HTTP_506_VARIANT_ALSO_NEGOTIATES; // = "506 Variant Also Negotiates"s;
extern const std::string STATUS_CODE_HTTP_507_INSUFFICIENT_STORAGE; // = "507 Insufficient Storage"s;
extern const std::string STATUS_CODE_HTTP_508_LOOP_DETECTED; // = "508 Loop Detected"s;
extern const std::string STATUS_CODE_HTTP_510_NOT_EXTENDED; // = "510 Not Extended"s;
extern const std::string STATUS_CODE_HTTP_511_NETWORK_AUTHENTICATION_REQUIRED; // = "511 Network Authentication Required"s;

/**
 * https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 */
enum class StatusCode : int32_t {
    HTTP_UNKNOWN = 0,

    HTTP_100_CONTINUE = 100,
    HTTP_101_SWITCHING_PROTOCOLS = 101,
    HTTP_102_PROCESSING = 102,
    HTTP_103_EARLY_HINTS = 103,

    HTTP_200_OK = 200,
    HTTP_201_CREATED = 201,
    HTTP_202_ACCEPTED = 202,
    HTTP_203_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_204_NO_CONTENT = 204,
    HTTP_205_RESET_CONTENT = 205,
    HTTP_206_PARTIAL_CONTENT = 206,
    HTTP_207_MULTI_STATUS = 207,
    HTTP_208_ALREADY_REPORTED = 208,
    HTTP_226_IM_USED = 226,

    HTTP_300_MULTIPLE_CHOICES = 300,
    HTTP_301_MOVED_PERMANENTLY = 301,
    HTTP_302_FOUND = 302,
    HTTP_303_SEE_OTHER = 303,
    HTTP_304_NOT_MODIFIED = 304,
    HTTP_305_USE_PROXY = 305,
    /*
     * HTTP status code 306 is unused and reserved per RFC 7231 (https://tools.ietf.org/html/rfc7231#section-6.4.6),
     * but originally meant 'switch proxy', so leaving for backwards compatibility.
     */
    HTTP_306_SWITCH_PROXY = 306,
    HTTP_307_TEMPORARY_REDIRECT = 307,
    HTTP_308_PERMANENT_REDIRECT = 308,

    HTTP_400_BAD_REQUEST = 400,
    HTTP_401_UNAUTHORIZED = 401,
    HTTP_402_PAYMENT_REQUIRED = 402,
    HTTP_403_FORBIDDEN = 403,
    HTTP_404_NOT_FOUND = 404,
    HTTP_405_METHOD_NOT_ALLOWED = 405,
    HTTP_406_NOT_ACCEPTABLE = 406,
    HTTP_407_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_408_REQUEST_TIMEOUT = 408,
    HTTP_409_CONFLICT = 409,
    HTTP_410_GONE = 410,
    HTTP_411_LENGTH_REQUIRED = 411,
    HTTP_412_PRECONDITION_FAILED = 412,
    HTTP_413_PAYLOAD_TOO_LARGE = 413,
    HTTP_414_URI_TOO_LONG = 414,
    HTTP_415_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_416_RANGE_NOT_SATISFIABLE = 416,
    HTTP_417_EXPECTATION_FAILED = 417,
    HTTP_418_IM_A_TEAPOT = 418,
    HTTP_421_MISDIRECTED_REQUEST = 421,
    HTTP_422_UNPROCESSABLE_ENTITY = 422,
    HTTP_423_LOCKED = 423,
    HTTP_424_FAILED_DEPENDENCY = 424,
    HTTP_425_TOO_EARLY = 425, // https://tools.ietf.org/html/rfc8470#section-5.2
    HTTP_426_UPGRADE_REQUIRED = 426,
    HTTP_428_PRECONDITION_REQUIRED = 428, // https://tools.ietf.org/html/rfc6585
    HTTP_429_TOO_MANY_REQUESTS = 429,
    HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

    HTTP_500_INTERNAL_SERVER_ERROR = 500,
    HTTP_501_NOT_IMPLEMENTED = 501,
    HTTP_502_BAD_GATEWAY = 502,
    HTTP_503_SERVICE_UNAVAILABLE = 503,
    HTTP_504_GATEWAY_TIMEOUT = 504,
    HTTP_505_HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_506_VARIANT_ALSO_NEGOTIATES = 506,
    HTTP_507_INSUFFICIENT_STORAGE = 507,
    HTTP_508_LOOP_DETECTED = 508,
    HTTP_510_NOT_EXTENDED = 510,
    HTTP_511_NETWORK_AUTHENTICATION_REQUIRED = 511
};

/**
 * @param code The status code to retrieve its string representation.
 * @return string
 */
auto to_string(
    StatusCode code) -> const std::string&;

/**
 * @param code The HTTP status code as an int.
 * @return StatusCode
 */
auto to_enum(
    int32_t code) -> StatusCode;

extern const std::string CONTENT_TYPE_UNKNOWN; // = "UNKNOWN"s;

extern const std::string CONTENT_TYPE_NO_CONTENT; // = ""s;

extern const std::string CONTENT_TYPE_TEXT_CSS; // = "text/css"s;
extern const std::string CONTENT_TYPE_TEXT_CSV; // = "text/csv"s;
extern const std::string CONTENT_TYPE_TEXT_HTML; // = "text/html"s;
extern const std::string CONTENT_TYPE_TEXT_PLAIN; // = "text/plain"s;
extern const std::string CONTENT_TYPE_TEXT_XML; // = "text/xml"s;

extern const std::string CONTENT_TYPE_IMAGE_GIF; // = "image/gif"s;
extern const std::string CONTENT_TYPE_IMAGE_JPEG; // = "image/jpeg"s;
extern const std::string CONTENT_TYPE_IMAGE_PNG; // = "image/png"s;
extern const std::string CONTENT_TYPE_IMAGE_TIFF; // = "image/tiff"s;
extern const std::string CONTENT_TYPE_IMAGE_X_ICON; // = "image/x-icon"s;
extern const std::string CONTENT_TYPE_IMAGE_SVG_XML; // = "image/svg+xml"s;

extern const std::string CONTENT_TYPE_VIDEO_MPEG; // = "video/mpeg"s;
extern const std::string CONTENT_TYPE_VIDEO_MP4; // = "video/mp4"s;
extern const std::string CONTENT_TYPE_VIDEO_X_FLV; // = "video/x-flv"s;
extern const std::string CONTENT_TYPE_VIDEO_WEBM; // = "video/webm"s;

extern const std::string CONTENT_TYPE_MULTIPART_MIXED; // = "multipart/mixed"s;
extern const std::string CONTENT_TYPE_MULTIPART_ALTERNATIVE; // = "multipart/alternative"s;
extern const std::string CONTENT_TYPE_MULTIPART_RELATED; // = "multipart/related"s;
extern const std::string CONTENT_TYPE_MULTIPART_FORM_DATA; // = "multipart/form-data"s;

extern const std::string CONTENT_TYPE_AUDIO_MPEG; // = "audio/mpeg"s;
extern const std::string CONTENT_TYPE_AUDIO_X_MS_WMA; // = "audio/x-ms-wma"s;
extern const std::string CONTENT_TYPE_AUDIO_X_WAV; // = "audio/x-wav"s;

extern const std::string CONTENT_TYPE_APPLICATION_JAVASCRIPT; // = "application/javascript"s;
extern const std::string CONTENT_TYPE_APPLICATION_OCTET_STREAM; // = "application/octet-stream"s;
extern const std::string CONTENT_TYPE_APPLICATION_OGG; // = "application/ogg"s;
extern const std::string CONTENT_TYPE_APPLICATION_PDF; // = "application/pdf"s;
extern const std::string CONTENT_TYPE_APPLICATION_XHTML_XML; // = "application/xhtml+xml"s;
extern const std::string CONTENT_TYPE_APPLICATION_X_SHOCKWAVE_FLASH; // = "application/x-shockwave-flash"s;
extern const std::string CONTENT_TYPE_APPLICATION_JSON; // = "application/json"s;
extern const std::string CONTENT_TYPE_APPLICATION_LD_JSON; // = "application/ld+json"s;
extern const std::string CONTENT_TYPE_APPLICATION_XML; // = "application/xml"s;
extern const std::string CONTENT_TYPE_APPLICATION_ZIP; // = "application/zip"s;
extern const std::string CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED; // = "application/x-www-form-urlencoded"s;

/**
 * HTTP 'Content-Type" types.  Can be extended to support
 * as many values as needed.  The design here is to reduce
 * string copies/manipulation.
 */
enum class ContentType : uint64_t {
    UNKNOWN,

    NO_CONTENT,

    TEXT_CSS,
    TEXT_CSV,
    TEXT_HTML,
    TEXT_PLAIN,
    TEXT_XML,

    IMAGE_GIF,
    IMAGE_JPEG,
    IMAGE_PNG,
    IMAGE_TIFF,
    IMAGE_X_ICON,
    IMAGE_SVG_XML,

    VIDEO_MPEG,
    VIDEO_MP4,
    VIDEO_X_FLV,
    VIDEO_WEBM,

    MULTIPART_MIXED,
    MULTIPART_ALTERNATIVE,
    MULTIPART_RELATED,
    MULTIPART_FORM_DATA,

    AUDIO_MPEG,
    AUDIO_X_MS_WMA,
    AUDIO_X_WAV,

    APPLICATION_JAVASCRIPT,
    APPLICATION_OCTET_STREAM,
    APPLICATION_OGG,
    APPLICATION_PDF,
    APPLICATION_XHTML_XML,
    APPLICATION_X_SHOCKWAVE_FLASH,
    APPLICATION_JSON,
    APPLICATION_LD_JSON,
    APPLICATION_XML,
    APPLICATION_ZIP,
    APPLICATION_X_WWW_FORM_URLENCODED
};

auto to_string(
    ContentType content_type) -> const std::string&;

extern const std::string CONNECTION_TYPE_UNKNOWN; // = "UNKNOWN"s;
extern const std::string CONNECTION_TYPE_CLOSE; // = "close"s;
extern const std::string CONNECTION_TYPE_KEEP_ALIVE; // = "keep-alive"s;
extern const std::string CONNECTION_TYPE_UPGRADE; // = "upgrade"s;

enum class ConnectionType : uint64_t {
    CLOSE,
    KEEP_ALIVE,
    UPGRADE
};

auto to_string(
    ConnectionType connection_type) -> const std::string&;

} // lift
