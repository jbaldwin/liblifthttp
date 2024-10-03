#pragma once

#include <string>
#include <cstdint>

#include <curl/curl.h>

namespace lift::http
{
inline const std::string method_unknown{"unknown"};
inline const std::string method_get{"GET"};
inline const std::string method_head{"HEAD"};
inline const std::string method_post{"POST"};
inline const std::string method_put{"PUT"};
inline const std::string method_delete{"DELETE"};
inline const std::string method_connect{"CONNECT"};
inline const std::string method_options{"OPTIONS"};
inline const std::string method_patch{"PATCH"};

enum class method : uint8_t
{
    unknown,
    get,
    head,
    post,
    put,
    delete_t,
    connect,
    options,
    patch
};

auto to_string(method m) -> const std::string&;

// Some liberty is taken on the version strings where they don't match the specification.

inline const std::string version_unknown{"HTTP/unknown"};
inline const std::string version_use_best{"HTTP/Best"};
inline const std::string version_v1_0{"HTTP/1.0"};
inline const std::string version_v1_1{"HTTP/1.1"};
inline const std::string version_v2_0{"HTTP/2.0"};
inline const std::string version_v2_0_tls{"HTTP/2.0-TLS"};
inline const std::string version_v2_0_only{"HTTP/2.0-only"};

enum class version : uint8_t
{
    unknown = 255,
    /// Use the best version available.
    use_best = CURL_HTTP_VERSION_NONE,
    /// Use HTTP 1.0.
    v1_0 = CURL_HTTP_VERSION_1_0,
    /// Use HTTP 1.1.
    v1_1 = CURL_HTTP_VERSION_1_1,
    /// Attempt HTTP 2 requests but fallback to 1.1 on failure.
    v2_0 = CURL_HTTP_VERSION_2_0,
    /// Attempt HTTP 2 over TLS (HTTPS) but fallback to 1.1 on failure.
    v2_0_tls = CURL_HTTP_VERSION_2TLS,
    /// Use HTTP 2.0 non-TLS with no fallback to 1.1.
    v2_0_only = CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE
};

auto to_string(version v) -> const std::string&;

inline const std::string status_code_http_unknown{"unknown"};

inline const std::string status_code_http_100_continue{"100 Continue"};
inline const std::string status_code_http_101_switching_protocols{"101 Switching Protocols"};
inline const std::string status_code_http_102_processing{"102 Processing"};
inline const std::string status_code_http_103_early_hints{"103 Early Hints"};

inline const std::string status_code_http_200_ok{"200 OK"};
inline const std::string status_code_http_201_created{"201 Created"};
inline const std::string status_code_http_202_accepted{"202 Accepted"};
inline const std::string status_code_http_203_non_authoritative_information{"203 Non-Authoritative Information"};
inline const std::string status_code_http_204_no_content{"204 No Content"};
inline const std::string status_code_http_205_reset_content{"205 Reset Content"};
inline const std::string status_code_http_206_partial_content{"206 Partial Content"};
inline const std::string status_code_http_207_multi_status{"207 Multi-Status"};
inline const std::string status_code_http_208_already_reported{"208 Already Reported"};
inline const std::string status_code_http_226_im_used{"226 IM Used"};

inline const std::string status_code_http_300_multiple_choices{"300 Multiple Choices"};
inline const std::string status_code_http_301_moved_permanently{"301 Moved Permanently"};
inline const std::string status_code_http_302_found{"302 Found"};
inline const std::string status_code_http_303_see_other{"303 See Other"};
inline const std::string status_code_http_304_not_modified{"304 Not Modified"};
inline const std::string status_code_http_305_use_proxy{"305 Use Proxy"};
inline const std::string status_code_http_306_switch_proxy{"306 Switch Proxy"};
inline const std::string status_code_http_307_temporary_redirect{"307 Temporary Redirect"};
inline const std::string status_code_http_308_permanent_redirect{"308 Permanent Redirect"};

inline const std::string status_code_http_400_bad_request{"400 Bad Request"};
inline const std::string status_code_http_401_unauthorized{"401 Unauthorized"};
inline const std::string status_code_http_402_payment_required{"402 Payment Required"};
inline const std::string status_code_http_403_forbidden{"403 Forbidden"};
inline const std::string status_code_http_404_not_found{"404 Not Found"};
inline const std::string status_code_http_405_method_not_allowed{"405 Method Not Allowed"};
inline const std::string status_code_http_406_not_acceptable{"406 Not Acceptable"};
inline const std::string status_code_http_407_proxy_authentication_required{"407 Proxy Authentication Required"};
inline const std::string status_code_http_408_request_timeout{"408 Request Timeout"};
inline const std::string status_code_http_409_conflict{"409 Conflict"};
inline const std::string status_code_http_410_gone{"410 Gone"};
inline const std::string status_code_http_411_length_required{"411 Length Required"};
inline const std::string status_code_http_412_precondition_failed{"412 Precondition Failed"};
inline const std::string status_code_http_413_payload_too_large{"413 Payload Too Large"};
inline const std::string status_code_http_414_uri_too_long{"414 URI Too Long"};
inline const std::string status_code_http_415_unsupported_media_type{"415 Unsupported Media Type"};
inline const std::string status_code_http_416_range_not_satisfiable{"416 Range Not Satisfiable"};
inline const std::string status_code_http_417_expectation_failed{"417 Expectation Failed"};
inline const std::string status_code_http_418_im_a_teapot{"418 I'm a teapot"};
inline const std::string status_code_http_421_misdirected_request{"421 Misdirected Request"};
inline const std::string status_code_http_422_unprocessable_entity{"422 Unprocessable Entity"};
inline const std::string status_code_http_423_locked{"423 Locked"};
inline const std::string status_code_http_424_failed_dependency{"424 Failed Dependency"};
inline const std::string status_code_http_425_too_early{"425 Too Early"};
inline const std::string status_code_http_426_upgrade_required{"426 Upgrade Required"};
inline const std::string status_code_http_428_precondition_required{"428 Precondition Required"};
inline const std::string status_code_http_429_too_many_requests{"429 Too Many Requests"};
inline const std::string status_code_http_431_request_header_fields_too_large{"431 Request Header Fields Too Large"};
inline const std::string status_code_http_451_unavailable_for_legal_reasons{"451 Unavailable For Legal Reasons"};

inline const std::string status_code_http_500_internal_server_error{"500 Internal Server Error"};
inline const std::string status_code_http_501_not_implemented{"501 Not Implemented"};
inline const std::string status_code_http_502_bad_gateway{"502 Bad Gateway"};
inline const std::string status_code_http_503_service_unavailable{"503 Service Unavailable"};
inline const std::string status_code_http_504_gateway_timeout{"504 Gateway Timeout"};
inline const std::string status_code_http_505_http_version_not_supported{"505 HTTP Version Not Supported"};
inline const std::string status_code_http_506_variant_also_negotiates{"506 Variant Also Negotiates"};
inline const std::string status_code_http_507_insufficient_storage{"507 Insufficient Storage"};
inline const std::string status_code_http_508_loop_detected{"508 Loop Detected"};
inline const std::string status_code_http_510_not_extended{"510 Not Extended"};
inline const std::string status_code_http_511_network_authentication_required{"511 Network Authentication Required"};

/**
 * https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 */
enum class status_code : uint16_t
{
    http_unknown = 0,

    http_100_continue            = 100,
    http_101_switching_protocols = 101,
    http_102_processing          = 102,
    http_103_early_hints         = 103,

    http_200_ok                            = 200,
    http_201_created                       = 201,
    http_202_accepted                      = 202,
    http_203_non_authoritative_information = 203,
    http_204_no_content                    = 204,
    http_205_reset_content                 = 205,
    http_206_partial_content               = 206,
    http_207_multi_status                  = 207,
    http_208_already_reported              = 208,
    http_226_im_used                       = 226,

    http_300_multiple_choices  = 300,
    http_301_moved_permanently = 301,
    http_302_found             = 302,
    http_303_see_other         = 303,
    http_304_not_modified      = 304,
    http_305_use_proxy         = 305,
    /*
     * HTTP status code 306 is unused and reserved per RFC 7231 (https://tools.ietf.org/html/rfc7231#section-6.4.6),
     * but originally meant 'switch proxy', so leaving for backwards compatibility.
     */
    http_306_switch_proxy       = 306,
    http_307_temporary_redirect = 307,
    http_308_permanent_redirect = 308,

    http_400_bad_request                     = 400,
    http_401_unauthorized                    = 401,
    http_402_payment_required                = 402,
    http_403_forbidden                       = 403,
    http_404_not_found                       = 404,
    http_405_method_not_allowed              = 405,
    http_406_not_acceptable                  = 406,
    http_407_proxy_authentication_required   = 407,
    http_408_request_timeout                 = 408,
    http_409_conflict                        = 409,
    http_410_gone                            = 410,
    http_411_length_required                 = 411,
    http_412_precondition_failed             = 412,
    http_413_payload_too_large               = 413,
    http_414_uri_too_long                    = 414,
    http_415_unsupported_media_type          = 415,
    http_416_range_not_satisfiable           = 416,
    http_417_expectation_failed              = 417,
    http_418_im_a_teapot                     = 418,
    http_421_misdirected_request             = 421,
    http_422_unprocessable_entity            = 422,
    http_423_locked                          = 423,
    http_424_failed_dependency               = 424,
    http_425_too_early                       = 425, // https://tools.ietf.org/html/rfc8470#section-5.2
    http_426_upgrade_required                = 426,
    http_428_precondition_required           = 428, // https://tools.ietf.org/html/rfc6585
    http_429_too_many_requests               = 429,
    http_431_request_header_fields_too_large = 431,
    http_451_unavailable_for_legal_reasons   = 451,

    http_500_internal_server_error           = 500,
    http_501_not_implemented                 = 501,
    http_502_bad_gateway                     = 502,
    http_503_service_unavailable             = 503,
    http_504_gateway_timeout                 = 504,
    http_505_http_version_not_supported      = 505,
    http_506_variant_also_negotiates         = 506,
    http_507_insufficient_storage            = 507,
    http_508_loop_detected                   = 508,
    http_510_not_extended                    = 510,
    http_511_network_authentication_required = 511
};

/**
 * @param code The status code to retrieve its string representation.
 * @return string
 */
auto to_string(status_code code) -> const std::string&;

/**
 * @param code The HTTP status code as an int.
 * @return StatusCode
 */
auto to_enum(uint16_t code) -> status_code;

inline const std::string content_type_unknown{"unknown"};

inline const std::string content_type_no_content{""};

inline const std::string content_type_text_css{"text/css"};
inline const std::string content_type_text_csv{"text/csv"};
inline const std::string content_type_text_html{"text/html"};
inline const std::string content_type_text_plain{"text/plain"};
inline const std::string content_type_text_xml{"text/xml"};

inline const std::string content_type_image_gif{"image/gif"};
inline const std::string content_type_image_jpeg{"image/jpeg"};
inline const std::string content_type_image_png{"image/png"};
inline const std::string content_type_image_tiff{"image/tiff"};
inline const std::string content_type_image_x_icon{"image/x-icon"};
inline const std::string content_type_image_svg_xml{"image/svg+xml"};

inline const std::string content_type_video_mpeg{"video/mpeg"};
inline const std::string content_type_video_mp4{"video/mp4"};
inline const std::string content_type_video_x_flv{"video/x-flv"};
inline const std::string content_type_video_webm{"video/webm"};

inline const std::string content_type_multipart_mixed{"multipart/mixed"};
inline const std::string content_type_multipart_alternative{"multipart/alternative"};
inline const std::string content_type_multipart_related{"multipart/related"};
inline const std::string content_type_multipart_form_data{"multipart/form-data"};

inline const std::string content_type_audio_mpeg{"audio/mpeg"};
inline const std::string content_type_audio_x_ms_wma{"audio/x-ms-wma"};
inline const std::string content_type_audio_x_wav{"audio/x-wav"};

inline const std::string content_type_application_javascript{"application/javascript"};
inline const std::string content_type_application_octet_stream{"application/octet-stream"};
inline const std::string content_type_application_ogg{"application/ogg"};
inline const std::string content_type_application_pdf{"application/pdf"};
inline const std::string content_type_application_xhtml_xml{"application/xhtml+xml"};
inline const std::string content_type_application_x_shockwave_flash{"application/x-shockwave-flash"};
inline const std::string content_type_application_json{"application/json"};
inline const std::string content_type_application_ld_json{"application/ld+json"};
inline const std::string content_type_application_xml{"application/xml"};
inline const std::string content_type_application_zip{"application/zip"};
inline const std::string content_type_application_x_www_form_urlencoded{"application/x-www-form-urlencoded"};

/**
 * HTTP 'Content-Type" types.  Can be extended to support
 * as many values as needed.  The design here is to reduce
 * string copies/manipulation.
 */
enum class content_type : uint16_t
{
    unknown,

    no_content,

    text_css,
    text_csv,
    text_html,
    text_plain,
    text_xml,

    image_gif,
    image_jpeg,
    image_png,
    image_tiff,
    image_x_icon,
    image_svg_xml,

    video_mpeg,
    video_mp4,
    video_x_flv,
    video_webm,

    multipart_mixed,
    multipart_alternative,
    multipart_related,
    multipart_form_data,

    audio_mpeg,
    audio_x_ms_wma,
    audio_x_wav,

    application_javascript,
    application_octet_stream,
    application_ogg,
    application_pdf,
    application_xhtml_xml,
    application_x_shockwave_flash,
    application_json,
    application_ld_json,
    application_xml,
    application_zip,
    application_x_www_form_urlencoded
};

auto to_string(content_type ct) -> const std::string&;

inline const std::string connection_type_unknown{"unknown"};
inline const std::string connection_type_close{"close"};
inline const std::string connection_type_keep_alive{"keep-alive"};
inline const std::string connection_type_upgrade{"upgrade"};

enum class connection_type : uint8_t
{
    close,
    keep_alive,
    upgrade
};

auto to_string(connection_type ct) -> const std::string&;

} // namespace lift::http
