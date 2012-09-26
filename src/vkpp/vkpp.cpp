#include <cb\vkpp.hpp>

namespace cb {
    namespace vkpp {
        const char* get_error_string(result_code code) {
            switch(code) {
              case result_code::OK:
                return "";
              case result_code::APP_AUTH_ERROR:
                return "Error while app authorization.";
              case result_code::USER_AUTH_ERROR:
                return "Error while user authorization.";
              case result_code::CURL_INIT_ERROR:
                return "Error while initialize curl.";
              case result_code::CURL_ERROR:
                return "Look libcurl for more info.";
              case result_code::ALLOW_URL_NOT_FOUND:
                return "Allow url not found.";
              case result_code::ALLOW_REQUEST_ERROR:
                return "Request for allow failed.";
              case result_code::ACCESS_TOKEN_NOT_RECEIVED:
                return "Access token not received.";
              case result_code::CALL_REQUEST_ERROR:
                return "Call request error.";
              case result_code::ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED:
                return "Allocation memory for vksession_impl failed.";
              case result_code::API_AUTH_REQUEST_ERROR:
                return "API authorization request failed.";
              case result_code::COOKIES_NOT_FOUND:
                return "Cookies not found.";
              case result_code::REMIXSID_NOT_FOUND:
                return "remixsid not found in cookies.";
              default:
                throw std::exception("Invalid result code.");
            }
        }
    }
}