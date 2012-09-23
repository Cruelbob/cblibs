#ifndef VKPP_HPP
#define VKPP_HPP

#include <string>
#include <list>
#include <utility>
#include <cstdint>

#include <cb\error.hpp>

namespace cb {
    namespace vkpp {
        namespace err_codes {
            enum err_codes {
                ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED,
                CURL_INIT_ERROR,
                USER_AUTH_ERROR,
                APP_AUTH_ERROR,
                API_AUTH_REQUEST_ERROR,
                ALLOW_URL_NOT_FOUND,
                ALLOW_REQUEST_ERROR,
                ACCESS_TOKEN_NOT_RECEIVED,
                CALL_REQUEST_ERROR
            };
        }
        namespace rights {
            enum rights {
                NOTIFY = 1,
                FRIENDS = 2,
                PHOTOS = 4,
                AUDIO = 8,
                VIDEO = 16,
                DOCS = 32,
                NOTES = 64,
                PAGES = 128,
                STATUS = 256,
                OFFERS = 512,
                QUESTIONS = 1024,
                WALL = 2048,
                GROUPS = 4096,
                MESSAGES = 8192,
                NOTIFICATIONS = 16384,
                STATS = 32768,
                ADS = 65536,
                OFFLINE = 131072,
                NOHTTPS = 262144,
                /* "ALL" not includes "NOHTTPS" and "ADS" scope*/
                ALL = NOTIFY | FRIENDS | PHOTOS | AUDIO | VIDEO|
                      DOCS | NOTES | PAGES | STATUS | OFFERS |
                      QUESTIONS | WALL | GROUPS | MESSAGES | NOTIFICATIONS |
                      STATS | ADS | OFFLINE
            };
        }
        class vksession {
            struct session_info
            {
                std::string api_key;
                std::string remixsid;
                std::string user_id;
                std::string access_token;
                std::string expires_in;
            };
            class vksession_impl;

            std::shared_ptr<vksession_impl> impl;
          public:
            typedef std::pair<std::string,std::string> method_parameter;
            typedef std::list<method_parameter> parameter_list;

            vksession(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope = rights::ALL);
            session_info info() const;
            std::string raw_call(const std::string& method,const parameter_list& param_list = parameter_list());
        };
    };
};

#endif // VKPP_HPP