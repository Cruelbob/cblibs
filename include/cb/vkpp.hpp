#ifndef VKPP_HPP
#define VKPP_HPP

#include <string>
#include <list>
#include <utility>

#include <cb\error.hpp>

namespace cb {
    namespace vkpp {
        namespace err_codes {
            enum err_codes {
                ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED,
                CURL_INIT_ERROR,
                USER_AUTH_ERROR,
                APP_AUTH_ERROR
            };
        }
        class vksession {
            struct session_info
            {
                std::string api_key;
                std::string remixsid;
                std::string mid;
                std::string sid;
                std::string secret;
                std::string expire;
                std::string sig;
            };
            class vksession_impl;

            std::shared_ptr<vksession_impl> impl;
          public:
            typedef std::pair<std::string,std::string> method_parameter;
            typedef std::list<method_parameter> parameter_list;

            vksession(const std::string& login,const std::string& password,const std::string& api_key);
            session_info info() const;
            std::string raw_call(const std::string& method,const parameter_list& param_list = parameter_list());
        };
    };
};

#endif // VKPP_HPP