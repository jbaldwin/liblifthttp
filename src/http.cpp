#include "lift/http.hpp"

namespace lift::http
{
auto to_string(method m) -> const std::string&
{
    switch (m)
    {
        case method::get:
            return method_get;
        case method::head:
            return method_head;
        case method::post:
            return method_post;
        case method::put:
            return method_put;
        case method::delete_t:
            return method_delete;
        case method::connect:
            return method_connect;
        case method::options:
            return method_options;
        case method::patch:
            return method_patch;
        default:
            return method_unknown;
    }
}

auto to_string(version v) -> const std::string&
{
    switch (v)
    {
        case version::use_best:
            return version_use_best;
        case version::v1_0:
            return version_v1_0;
        case version::v1_1:
            return version_v1_1;
        case version::v2_0:
            return version_v2_0;
        case version::v2_0_tls:
            return version_v2_0_tls;
        case version::v2_0_only:
            return version_v2_0_only;
        default:
            return version_unknown;
    }
}

auto to_string(status_code code) -> const std::string&
{
    switch (code)
    {
        case status_code::http_unknown:
            return status_code_http_unknown;

        case status_code::http_100_continue:
            return status_code_http_100_continue;
        case status_code::http_101_switching_protocols:
            return status_code_http_101_switching_protocols;
        case status_code::http_102_processing:
            return status_code_http_102_processing;
        case status_code::http_103_early_hints:
            return status_code_http_103_early_hints;

        case status_code::http_200_ok:
            return status_code_http_200_ok;
        case status_code::http_201_created:
            return status_code_http_201_created;
        case status_code::http_202_accepted:
            return status_code_http_202_accepted;
        case status_code::http_203_non_authoritative_information:
            return status_code_http_203_non_authoritative_information;
        case status_code::http_204_no_content:
            return status_code_http_204_no_content;
        case status_code::http_205_reset_content:
            return status_code_http_205_reset_content;
        case status_code::http_206_partial_content:
            return status_code_http_206_partial_content;
        case status_code::http_207_multi_status:
            return status_code_http_207_multi_status;
        case status_code::http_208_already_reported:
            return status_code_http_208_already_reported;
        case status_code::http_226_im_used:
            return status_code_http_226_im_used;

        case status_code::http_300_multiple_choices:
            return status_code_http_300_multiple_choices;
        case status_code::http_301_moved_permanently:
            return status_code_http_301_moved_permanently;
        case status_code::http_302_found:
            return status_code_http_302_found;
        case status_code::http_303_see_other:
            return status_code_http_303_see_other;
        case status_code::http_304_not_modified:
            return status_code_http_304_not_modified;
        case status_code::http_305_use_proxy:
            return status_code_http_305_use_proxy;
        case status_code::http_306_switch_proxy:
            return status_code_http_306_switch_proxy;
        case status_code::http_307_temporary_redirect:
            return status_code_http_307_temporary_redirect;
        case status_code::http_308_permanent_redirect:
            return status_code_http_308_permanent_redirect;

        case status_code::http_400_bad_request:
            return status_code_http_400_bad_request;
        case status_code::http_401_unauthorized:
            return status_code_http_401_unauthorized;
        case status_code::http_402_payment_required:
            return status_code_http_402_payment_required;
        case status_code::http_403_forbidden:
            return status_code_http_403_forbidden;
        case status_code::http_404_not_found:
            return status_code_http_404_not_found;
        case status_code::http_405_method_not_allowed:
            return status_code_http_405_method_not_allowed;
        case status_code::http_406_not_acceptable:
            return status_code_http_406_not_acceptable;
        case status_code::http_407_proxy_authentication_required:
            return status_code_http_407_proxy_authentication_required;
        case status_code::http_408_request_timeout:
            return status_code_http_408_request_timeout;
        case status_code::http_409_conflict:
            return status_code_http_409_conflict;
        case status_code::http_410_gone:
            return status_code_http_410_gone;
        case status_code::http_411_length_required:
            return status_code_http_411_length_required;
        case status_code::http_412_precondition_failed:
            return status_code_http_412_precondition_failed;
        case status_code::http_413_payload_too_large:
            return status_code_http_413_payload_too_large;
        case status_code::http_414_uri_too_long:
            return status_code_http_414_uri_too_long;
        case status_code::http_415_unsupported_media_type:
            return status_code_http_415_unsupported_media_type;
        case status_code::http_416_range_not_satisfiable:
            return status_code_http_416_range_not_satisfiable;
        case status_code::http_417_expectation_failed:
            return status_code_http_417_expectation_failed;
        case status_code::http_418_im_a_teapot:
            return status_code_http_418_im_a_teapot;
        case status_code::http_421_misdirected_request:
            return status_code_http_421_misdirected_request;
        case status_code::http_422_unprocessable_entity:
            return status_code_http_422_unprocessable_entity;
        case status_code::http_423_locked:
            return status_code_http_423_locked;
        case status_code::http_424_failed_dependency:
            return status_code_http_424_failed_dependency;
        case status_code::http_425_too_early:
            return status_code_http_425_too_early;
        case status_code::http_426_upgrade_required:
            return status_code_http_426_upgrade_required;
        case status_code::http_428_precondition_required:
            return status_code_http_428_precondition_required;
        case status_code::http_429_too_many_requests:
            return status_code_http_429_too_many_requests;
        case status_code::http_431_request_header_fields_too_large:
            return status_code_http_431_request_header_fields_too_large;
        case status_code::http_451_unavailable_for_legal_reasons:
            return status_code_http_451_unavailable_for_legal_reasons;

        case status_code::http_500_internal_server_error:
            return status_code_http_500_internal_server_error;
        case status_code::http_501_not_implemented:
            return status_code_http_501_not_implemented;
        case status_code::http_502_bad_gateway:
            return status_code_http_502_bad_gateway;
        case status_code::http_503_service_unavailable:
            return status_code_http_503_service_unavailable;
        case status_code::http_504_gateway_timeout:
            return status_code_http_504_gateway_timeout;
        case status_code::http_505_http_version_not_supported:
            return status_code_http_505_http_version_not_supported;
        case status_code::http_506_variant_also_negotiates:
            return status_code_http_506_variant_also_negotiates;
        case status_code::http_507_insufficient_storage:
            return status_code_http_507_insufficient_storage;
        case status_code::http_508_loop_detected:
            return status_code_http_508_loop_detected;
        case status_code::http_510_not_extended:
            return status_code_http_510_not_extended;
        case status_code::http_511_network_authentication_required:
            return status_code_http_511_network_authentication_required;

        default:
            return status_code_http_unknown;
    }
}

auto to_enum(uint16_t code) -> status_code
{
    switch (code)
    {
        case 100:
            return status_code::http_100_continue;
        case 101:
            return status_code::http_101_switching_protocols;
        case 102:
            return status_code::http_102_processing;
        case 103:
            return status_code::http_103_early_hints;

        case 200:
            return status_code::http_200_ok;
        case 201:
            return status_code::http_201_created;
        case 202:
            return status_code::http_202_accepted;
        case 203:
            return status_code::http_203_non_authoritative_information;
        case 204:
            return status_code::http_204_no_content;
        case 205:
            return status_code::http_205_reset_content;
        case 206:
            return status_code::http_206_partial_content;
        case 207:
            return status_code::http_207_multi_status;
        case 208:
            return status_code::http_208_already_reported;
        case 226:
            return status_code::http_226_im_used;

        case 300:
            return status_code::http_300_multiple_choices;
        case 301:
            return status_code::http_301_moved_permanently;
        case 302:
            return status_code::http_302_found;
        case 303:
            return status_code::http_303_see_other;
        case 304:
            return status_code::http_304_not_modified;
        case 305:
            return status_code::http_305_use_proxy;
        case 306:
            return status_code::http_306_switch_proxy;
        case 307:
            return status_code::http_307_temporary_redirect;
        case 308:
            return status_code::http_308_permanent_redirect;

        case 400:
            return status_code::http_400_bad_request;
        case 401:
            return status_code::http_401_unauthorized;
        case 402:
            return status_code::http_402_payment_required;
        case 403:
            return status_code::http_403_forbidden;
        case 404:
            return status_code::http_404_not_found;
        case 405:
            return status_code::http_405_method_not_allowed;
        case 406:
            return status_code::http_406_not_acceptable;
        case 407:
            return status_code::http_407_proxy_authentication_required;
        case 408:
            return status_code::http_408_request_timeout;
        case 409:
            return status_code::http_409_conflict;
        case 410:
            return status_code::http_410_gone;
        case 411:
            return status_code::http_411_length_required;
        case 412:
            return status_code::http_412_precondition_failed;
        case 413:
            return status_code::http_413_payload_too_large;
        case 414:
            return status_code::http_414_uri_too_long;
        case 415:
            return status_code::http_415_unsupported_media_type;
        case 416:
            return status_code::http_416_range_not_satisfiable;
        case 417:
            return status_code::http_417_expectation_failed;
        case 418:
            return status_code::http_418_im_a_teapot;
        case 421:
            return status_code::http_421_misdirected_request;
        case 422:
            return status_code::http_422_unprocessable_entity;
        case 423:
            return status_code::http_423_locked;
        case 424:
            return status_code::http_424_failed_dependency;
        case 425:
            return status_code::http_425_too_early;
        case 426:
            return status_code::http_426_upgrade_required;
        case 428:
            return status_code::http_428_precondition_required;
        case 429:
            return status_code::http_429_too_many_requests;
        case 431:
            return status_code::http_431_request_header_fields_too_large;
        case 451:
            return status_code::http_451_unavailable_for_legal_reasons;

        case 500:
            return status_code::http_500_internal_server_error;
        case 501:
            return status_code::http_501_not_implemented;
        case 502:
            return status_code::http_502_bad_gateway;
        case 503:
            return status_code::http_503_service_unavailable;
        case 504:
            return status_code::http_504_gateway_timeout;
        case 505:
            return status_code::http_505_http_version_not_supported;
        case 506:
            return status_code::http_506_variant_also_negotiates;
        case 507:
            return status_code::http_507_insufficient_storage;
        case 508:
            return status_code::http_508_loop_detected;
        case 510:
            return status_code::http_510_not_extended;
        case 511:
            return status_code::http_511_network_authentication_required;

        case 0:
        default:
            return status_code::http_unknown;
    }
}

auto to_string(content_type ct) -> const std::string&
{
    switch (ct)
    {
        case content_type::no_content:
            return content_type_no_content;

        case content_type::text_css:
            return content_type_text_css;
        case content_type::text_csv:
            return content_type_text_csv;
        case content_type::text_html:
            return content_type_text_html;
        case content_type::text_plain:
            return content_type_text_plain;
        case content_type::text_xml:
            return content_type_text_xml;

        case content_type::image_gif:
            return content_type_image_gif;
        case content_type::image_jpeg:
            return content_type_image_jpeg;
        case content_type::image_png:
            return content_type_image_png;
        case content_type::image_tiff:
            return content_type_image_tiff;
        case content_type::image_x_icon:
            return content_type_image_x_icon;
        case content_type::image_svg_xml:
            return content_type_image_svg_xml;

        case content_type::video_mpeg:
            return content_type_video_mpeg;
        case content_type::video_mp4:
            return content_type_video_mp4;
        case content_type::video_x_flv:
            return content_type_video_x_flv;
        case content_type::video_webm:
            return content_type_video_webm;

        case content_type::multipart_mixed:
            return content_type_multipart_mixed;
        case content_type::multipart_alternative:
            return content_type_multipart_alternative;
        case content_type::multipart_related:
            return content_type_multipart_related;
        case content_type::multipart_form_data:
            return content_type_multipart_form_data;

        case content_type::audio_mpeg:
            return content_type_audio_mpeg;
        case content_type::audio_x_ms_wma:
            return content_type_audio_x_ms_wma;
        case content_type::audio_x_wav:
            return content_type_audio_x_wav;

        case content_type::application_javascript:
            return content_type_application_javascript;
        case content_type::application_octet_stream:
            return content_type_application_octet_stream;
        case content_type::application_ogg:
            return content_type_application_ogg;
        case content_type::application_pdf:
            return content_type_application_pdf;
        case content_type::application_xhtml_xml:
            return content_type_application_xhtml_xml;
        case content_type::application_x_shockwave_flash:
            return content_type_application_x_shockwave_flash;
        case content_type::application_json:
            return content_type_application_json;
        case content_type::application_ld_json:
            return content_type_application_ld_json;
        case content_type::application_xml:
            return content_type_application_xml;
        case content_type::application_zip:
            return content_type_application_zip;
        case content_type::application_x_www_form_urlencoded:
            return content_type_application_x_www_form_urlencoded;

        default:
            return content_type_unknown;
    }
}

auto to_string(connection_type ct) -> const std::string&
{
    switch (ct)
    {
        case connection_type::close:
            return connection_type_close;
        case connection_type::keep_alive:
            return connection_type_keep_alive;
        case connection_type::upgrade:
            return connection_type_upgrade;

        default:
            return connection_type_unknown;
    }
}

} // namespace lift::http
