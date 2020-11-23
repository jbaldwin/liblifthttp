#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("HTTP method to_string")
{
    using namespace lift::http;
    REQUIRE(to_string(method::get) == method_get);
    REQUIRE(to_string(method::head) == method_head);
    REQUIRE(to_string(method::post) == method_post);
    REQUIRE(to_string(method::put) == method_put);
    REQUIRE(to_string(method::delete_t) == method_delete);
    REQUIRE(to_string(method::connect) == method_connect);
    REQUIRE(to_string(method::options) == method_options);
    REQUIRE(to_string(method::patch) == method_patch);
    REQUIRE(to_string(method::unknown) == method_unknown);
    REQUIRE(to_string(static_cast<method>(1024)) == method_unknown);
}

TEST_CASE("HTTP version to_string")
{
    using namespace lift::http;
    REQUIRE(to_string(version::use_best) == version_use_best);
    REQUIRE(to_string(version::v1_0) == version_v1_0);
    REQUIRE(to_string(version::v1_1) == version_v1_1);
    REQUIRE(to_string(version::v2_0) == version_v2_0);
    REQUIRE(to_string(version::v2_0_tls) == version_v2_0_tls);
    REQUIRE(to_string(version::v2_0_only) == version_v2_0_only);
    REQUIRE(to_string(version::unknown) == version_unknown);
    REQUIRE(to_string(static_cast<version>(255)) == version_unknown);
}

TEST_CASE("HTTP status_code to_string")
{
    // clang-format off
    using namespace lift::http;
    REQUIRE(to_string(status_code::http_unknown) == status_code_http_unknown);
    REQUIRE(to_string(static_cast<status_code>(9001)) == status_code_http_unknown);

    REQUIRE(to_string(status_code::http_100_continue) == status_code_http_100_continue);
    REQUIRE(to_string(status_code::http_101_switching_protocols) == status_code_http_101_switching_protocols);
    REQUIRE(to_string(status_code::http_102_processing) == status_code_http_102_processing);
    REQUIRE(to_string(status_code::http_103_early_hints) == status_code_http_103_early_hints);

    REQUIRE(to_string(status_code::http_200_ok) == status_code_http_200_ok);
    REQUIRE(to_string(status_code::http_201_created) == status_code_http_201_created);
    REQUIRE(to_string(status_code::http_202_accepted) == status_code_http_202_accepted);
    REQUIRE(to_string(status_code::http_203_non_authoritative_information) == status_code_http_203_non_authoritative_information);
    REQUIRE(to_string(status_code::http_204_no_content) == status_code_http_204_no_content);
    REQUIRE(to_string(status_code::http_205_reset_content) == status_code_http_205_reset_content);
    REQUIRE(to_string(status_code::http_206_partial_content) == status_code_http_206_partial_content);
    REQUIRE(to_string(status_code::http_207_multi_status) == status_code_http_207_multi_status);
    REQUIRE(to_string(status_code::http_208_already_reported) == status_code_http_208_already_reported);
    REQUIRE(to_string(status_code::http_226_im_used) == status_code_http_226_im_used);

    REQUIRE(to_string(status_code::http_300_multiple_choices) == status_code_http_300_multiple_choices);
    REQUIRE(to_string(status_code::http_301_moved_permanently) == status_code_http_301_moved_permanently);
    REQUIRE(to_string(status_code::http_302_found) == status_code_http_302_found);
    REQUIRE(to_string(status_code::http_303_see_other) == status_code_http_303_see_other);
    REQUIRE(to_string(status_code::http_304_not_modified) == status_code_http_304_not_modified);
    REQUIRE(to_string(status_code::http_305_use_proxy) == status_code_http_305_use_proxy);
    REQUIRE(to_string(status_code::http_306_switch_proxy) == status_code_http_306_switch_proxy);
    REQUIRE(to_string(status_code::http_307_temporary_redirect) == status_code_http_307_temporary_redirect);
    REQUIRE(to_string(status_code::http_308_permanent_redirect) == status_code_http_308_permanent_redirect);

    REQUIRE(to_string(status_code::http_400_bad_request) == status_code_http_400_bad_request);
    REQUIRE(to_string(status_code::http_401_unauthorized) == status_code_http_401_unauthorized);
    REQUIRE(to_string(status_code::http_402_payment_required) == status_code_http_402_payment_required);
    REQUIRE(to_string(status_code::http_403_forbidden) == status_code_http_403_forbidden);
    REQUIRE(to_string(status_code::http_404_not_found) == status_code_http_404_not_found);
    REQUIRE(to_string(status_code::http_405_method_not_allowed) == status_code_http_405_method_not_allowed);
    REQUIRE(to_string(status_code::http_406_not_acceptable) == status_code_http_406_not_acceptable);
    REQUIRE(to_string(status_code::http_407_proxy_authentication_required) == status_code_http_407_proxy_authentication_required);
    REQUIRE(to_string(status_code::http_408_request_timeout) == status_code_http_408_request_timeout);
    REQUIRE(to_string(status_code::http_409_conflict) == status_code_http_409_conflict);
    REQUIRE(to_string(status_code::http_410_gone) == status_code_http_410_gone);
    REQUIRE(to_string(status_code::http_411_length_required) == status_code_http_411_length_required);
    REQUIRE(to_string(status_code::http_412_precondition_failed) == status_code_http_412_precondition_failed);
    REQUIRE(to_string(status_code::http_413_payload_too_large) == status_code_http_413_payload_too_large);
    REQUIRE(to_string(status_code::http_414_uri_too_long) == status_code_http_414_uri_too_long);
    REQUIRE(to_string(status_code::http_415_unsupported_media_type) == status_code_http_415_unsupported_media_type);
    REQUIRE(to_string(status_code::http_416_range_not_satisfiable) == status_code_http_416_range_not_satisfiable);
    REQUIRE(to_string(status_code::http_417_expectation_failed) == status_code_http_417_expectation_failed);
    REQUIRE(to_string(status_code::http_418_im_a_teapot) == status_code_http_418_im_a_teapot);
    REQUIRE(to_string(status_code::http_421_misdirected_request) == status_code_http_421_misdirected_request);
    REQUIRE(to_string(status_code::http_422_unprocessable_entity) == status_code_http_422_unprocessable_entity);
    REQUIRE(to_string(status_code::http_423_locked) == status_code_http_423_locked);
    REQUIRE(to_string(status_code::http_424_failed_dependency) == status_code_http_424_failed_dependency);
    REQUIRE(to_string(status_code::http_425_too_early) == status_code_http_425_too_early);
    REQUIRE(to_string(status_code::http_426_upgrade_required) == status_code_http_426_upgrade_required);
    REQUIRE(to_string(status_code::http_428_precondition_required) == status_code_http_428_precondition_required);
    REQUIRE(to_string(status_code::http_429_too_many_requests) == status_code_http_429_too_many_requests);
    REQUIRE(to_string(status_code::http_431_request_header_fields_too_large) == status_code_http_431_request_header_fields_too_large);
    REQUIRE(to_string(status_code::http_451_unavailable_for_legal_reasons) == status_code_http_451_unavailable_for_legal_reasons);

    REQUIRE(to_string(status_code::http_500_internal_server_error) == status_code_http_500_internal_server_error);
    REQUIRE(to_string(status_code::http_501_not_implemented) == status_code_http_501_not_implemented);
    REQUIRE(to_string(status_code::http_502_bad_gateway) == status_code_http_502_bad_gateway);
    REQUIRE(to_string(status_code::http_503_service_unavailable) == status_code_http_503_service_unavailable);
    REQUIRE(to_string(status_code::http_504_gateway_timeout) == status_code_http_504_gateway_timeout);
    REQUIRE(to_string(status_code::http_505_http_version_not_supported) == status_code_http_505_http_version_not_supported);
    REQUIRE(to_string(status_code::http_506_variant_also_negotiates) == status_code_http_506_variant_also_negotiates);
    REQUIRE(to_string(status_code::http_507_insufficient_storage) == status_code_http_507_insufficient_storage);
    REQUIRE(to_string(status_code::http_508_loop_detected) == status_code_http_508_loop_detected);
    REQUIRE(to_string(status_code::http_510_not_extended) == status_code_http_510_not_extended);
    REQUIRE(to_string(status_code::http_511_network_authentication_required) == status_code_http_511_network_authentication_required);
    // clang-format on
}

TEST_CASE("HTTP status_code to_enum")
{
    using namespace lift::http;

    REQUIRE(to_enum(0) == status_code::http_unknown);
    REQUIRE(to_enum(9001) == status_code::http_unknown);

    REQUIRE(to_enum(100) == status_code::http_100_continue);
    REQUIRE(to_enum(101) == status_code::http_101_switching_protocols);
    REQUIRE(to_enum(102) == status_code::http_102_processing);
    REQUIRE(to_enum(103) == status_code::http_103_early_hints);

    REQUIRE(to_enum(200) == status_code::http_200_ok);
    REQUIRE(to_enum(201) == status_code::http_201_created);
    REQUIRE(to_enum(202) == status_code::http_202_accepted);
    REQUIRE(to_enum(203) == status_code::http_203_non_authoritative_information);
    REQUIRE(to_enum(204) == status_code::http_204_no_content);
    REQUIRE(to_enum(205) == status_code::http_205_reset_content);
    REQUIRE(to_enum(206) == status_code::http_206_partial_content);
    REQUIRE(to_enum(207) == status_code::http_207_multi_status);
    REQUIRE(to_enum(208) == status_code::http_208_already_reported);
    REQUIRE(to_enum(226) == status_code::http_226_im_used);

    REQUIRE(to_enum(300) == status_code::http_300_multiple_choices);
    REQUIRE(to_enum(301) == status_code::http_301_moved_permanently);
    REQUIRE(to_enum(302) == status_code::http_302_found);
    REQUIRE(to_enum(303) == status_code::http_303_see_other);
    REQUIRE(to_enum(304) == status_code::http_304_not_modified);
    REQUIRE(to_enum(305) == status_code::http_305_use_proxy);
    REQUIRE(to_enum(306) == status_code::http_306_switch_proxy);
    REQUIRE(to_enum(307) == status_code::http_307_temporary_redirect);
    REQUIRE(to_enum(308) == status_code::http_308_permanent_redirect);

    REQUIRE(to_enum(400) == status_code::http_400_bad_request);
    REQUIRE(to_enum(401) == status_code::http_401_unauthorized);
    REQUIRE(to_enum(402) == status_code::http_402_payment_required);
    REQUIRE(to_enum(403) == status_code::http_403_forbidden);
    REQUIRE(to_enum(404) == status_code::http_404_not_found);
    REQUIRE(to_enum(405) == status_code::http_405_method_not_allowed);
    REQUIRE(to_enum(406) == status_code::http_406_not_acceptable);
    REQUIRE(to_enum(407) == status_code::http_407_proxy_authentication_required);
    REQUIRE(to_enum(408) == status_code::http_408_request_timeout);
    REQUIRE(to_enum(409) == status_code::http_409_conflict);
    REQUIRE(to_enum(410) == status_code::http_410_gone);
    REQUIRE(to_enum(411) == status_code::http_411_length_required);
    REQUIRE(to_enum(412) == status_code::http_412_precondition_failed);
    REQUIRE(to_enum(413) == status_code::http_413_payload_too_large);
    REQUIRE(to_enum(414) == status_code::http_414_uri_too_long);
    REQUIRE(to_enum(415) == status_code::http_415_unsupported_media_type);
    REQUIRE(to_enum(416) == status_code::http_416_range_not_satisfiable);
    REQUIRE(to_enum(417) == status_code::http_417_expectation_failed);
    REQUIRE(to_enum(418) == status_code::http_418_im_a_teapot);
    REQUIRE(to_enum(421) == status_code::http_421_misdirected_request);
    REQUIRE(to_enum(422) == status_code::http_422_unprocessable_entity);
    REQUIRE(to_enum(423) == status_code::http_423_locked);
    REQUIRE(to_enum(424) == status_code::http_424_failed_dependency);
    REQUIRE(to_enum(425) == status_code::http_425_too_early);
    REQUIRE(to_enum(426) == status_code::http_426_upgrade_required);
    REQUIRE(to_enum(428) == status_code::http_428_precondition_required);
    REQUIRE(to_enum(429) == status_code::http_429_too_many_requests);
    REQUIRE(to_enum(431) == status_code::http_431_request_header_fields_too_large);
    REQUIRE(to_enum(451) == status_code::http_451_unavailable_for_legal_reasons);

    REQUIRE(to_enum(500) == status_code::http_500_internal_server_error);
    REQUIRE(to_enum(501) == status_code::http_501_not_implemented);
    REQUIRE(to_enum(502) == status_code::http_502_bad_gateway);
    REQUIRE(to_enum(503) == status_code::http_503_service_unavailable);
    REQUIRE(to_enum(504) == status_code::http_504_gateway_timeout);
    REQUIRE(to_enum(505) == status_code::http_505_http_version_not_supported);
    REQUIRE(to_enum(506) == status_code::http_506_variant_also_negotiates);
    REQUIRE(to_enum(507) == status_code::http_507_insufficient_storage);
    REQUIRE(to_enum(508) == status_code::http_508_loop_detected);
    REQUIRE(to_enum(510) == status_code::http_510_not_extended);
    REQUIRE(to_enum(511) == status_code::http_511_network_authentication_required);
}

TEST_CASE("HTTP content_type to_string")
{
    // clang-format off
    using namespace lift::http;
    REQUIRE(to_string(content_type::unknown) == content_type_unknown);
    REQUIRE(to_string(static_cast<content_type>(45678)) == content_type_unknown);

    REQUIRE(to_string(content_type::no_content) == content_type_no_content);

    REQUIRE(to_string(content_type::text_css) == content_type_text_css);
    REQUIRE(to_string(content_type::text_csv) == content_type_text_csv);
    REQUIRE(to_string(content_type::text_html) == content_type_text_html);
    REQUIRE(to_string(content_type::text_plain) == content_type_text_plain);
    REQUIRE(to_string(content_type::text_xml) == content_type_text_xml);

    REQUIRE(to_string(content_type::image_gif) == content_type_image_gif);
    REQUIRE(to_string(content_type::image_jpeg) == content_type_image_jpeg);
    REQUIRE(to_string(content_type::image_png) == content_type_image_png);
    REQUIRE(to_string(content_type::image_tiff) == content_type_image_tiff);
    REQUIRE(to_string(content_type::image_x_icon) == content_type_image_x_icon);
    REQUIRE(to_string(content_type::image_svg_xml) == content_type_image_svg_xml);

    REQUIRE(to_string(content_type::video_mpeg) == content_type_video_mpeg);
    REQUIRE(to_string(content_type::video_mp4) == content_type_video_mp4);
    REQUIRE(to_string(content_type::video_x_flv) == content_type_video_x_flv);
    REQUIRE(to_string(content_type::video_webm) == content_type_video_webm);

    REQUIRE(to_string(content_type::multipart_mixed) == content_type_multipart_mixed);
    REQUIRE(to_string(content_type::multipart_alternative) == content_type_multipart_alternative);
    REQUIRE(to_string(content_type::multipart_related) == content_type_multipart_related);
    REQUIRE(to_string(content_type::multipart_form_data) == content_type_multipart_form_data);

    REQUIRE(to_string(content_type::audio_mpeg) == content_type_audio_mpeg);
    REQUIRE(to_string(content_type::audio_x_ms_wma) == content_type_audio_x_ms_wma);
    REQUIRE(to_string(content_type::audio_x_wav) == content_type_audio_x_wav);

    REQUIRE(to_string(content_type::application_javascript) == content_type_application_javascript);
    REQUIRE(to_string(content_type::application_octet_stream) == content_type_application_octet_stream);
    REQUIRE(to_string(content_type::application_ogg) == content_type_application_ogg);
    REQUIRE(to_string(content_type::application_pdf) == content_type_application_pdf);
    REQUIRE(to_string(content_type::application_xhtml_xml) == content_type_application_xhtml_xml);
    REQUIRE(to_string(content_type::application_x_shockwave_flash) == content_type_application_x_shockwave_flash);
    REQUIRE(to_string(content_type::application_json) == content_type_application_json);
    REQUIRE(to_string(content_type::application_ld_json) == content_type_application_ld_json);
    REQUIRE(to_string(content_type::application_xml) == content_type_application_xml);
    REQUIRE(to_string(content_type::application_zip) == content_type_application_zip);
    REQUIRE(to_string(content_type::application_x_www_form_urlencoded) == content_type_application_x_www_form_urlencoded);
    // clang-format on
}

TEST_CASE("HTTP connection_type to_string")
{
    using namespace lift::http;
    REQUIRE(to_string(connection_type::close) == connection_type_close);
    REQUIRE(to_string(connection_type::keep_alive) == connection_type_keep_alive);
    REQUIRE(to_string(connection_type::upgrade) == connection_type_upgrade);
    REQUIRE(to_string(static_cast<connection_type>(3839)) == connection_type_unknown);
}
