#ifndef VKPP_HPP
#define VKPP_HPP

#include <string>
#include <list>
#include <utility>
#include <cstdint>
#include <memory>

namespace cb {
    namespace vkpp {
        enum class result_code: int {
            OK,
            ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED,
            CURL_INIT_ERROR,
            API_AUTH_REQUEST_ERROR,
            ALLOW_URL_NOT_FOUND,
            ALLOW_REQUEST_ERROR,
            ACCESS_TOKEN_NOT_RECEIVED,
            CALL_REQUEST_ERROR,
            CURL_ERROR,
            COOKIES_NOT_FOUND,
            REMIXSID_NOT_FOUND
        };
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
        const char* get_error_string(result_code code);
/**
@brief Structure for keeping session information.
*/
        struct session_info
        {
            std::string api_key; /**< Application ID */
            std::string remixsid; /**< VKontakte cookie session id */
            std::string user_id; /**< User ID */
            std::string access_token; /**< Token for access to using VK API */
            std::string expires_in; /**< Time in seconds to end of session */
        };
/**
@brief Class for using VKontakte API.
@detailed Provides high level interface for VK API. 
*/
        class vksession {
            class vksession_impl;

            std::shared_ptr<vksession_impl> impl; /**< Pointer to implementation class. */
          public:
            typedef std::pair<std::string,std::string> method_parameter; /**< VK API method parameter. */
            typedef std::list<method_parameter> parameter_list; /**< VK API method parameter list. */

/**
@brief Constructor.

@param login VKontakte login. Phone number or email.
@param password VKontakte password.
@param api_key Application ID.
@param scope Rights to which you want to access.

@throw vksession_exception
*/
            vksession(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope = rights::ALL);
/**
@brief Constructor.

@param code Result code.
@param login VKontakte login. Phone number or email.
@param password VKontakte password.
@param api_key Application ID.
@param scope Rights to which you want to access.
*/
            vksession(result_code& code,const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope = rights::ALL);
/**
@brief Reinitialization method. If session is open then closes it.

@param login VKontakte login. Phone number or email.
@param password VKontakte password.
@param api_key Application ID.
@param scope Rights to which you want to access.

@throw vksession_exception
*/
            void open(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope = rights::ALL);
/**
@brief Reinitialization method. If session is open then closes it.

@param code Result code.
@param login VKontakte login. Phone number or email.
@param password VKontakte password.
@param api_key Application ID.
@param scope Rights to which you want to access.
*/
            void open(result_code& code,const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope = rights::ALL);
/**
@brief Closes session.
*/
            void close();
/**
@brief Checks whether the session is open.
*/
            bool is_open();
/**
@brief Return session information.

@return Session information.
*/
            session_info info() const;
/**
@brief Calls VK API method and returns response in JSON UTF-8.

@param method VK API method name(Ex. "friends.get").
@param param_list List of method parameters.

@return Method response.

@throw vksession_exception
*/
            std::string raw_call(const std::string& method,const parameter_list& param_list = parameter_list());
/**
@brief Calls VK API method and returns response in JSON UTF-8.

@param code Result code.
@param method VK API method name(Ex. "friends.get").
@param param_list List of method parameters.

@return Method response.

@throw vksession_exception
*/
            std::string raw_call(result_code& code,const std::string& method,const parameter_list& param_list = parameter_list());
        };
        class vksession_exception: public std::exception {
          protected:
            result_code code_;
          public:
            explicit vksession_exception(result_code res_code);
            virtual result_code code() const;
        };
    };
};

#endif // VKPP_HPP