#include <cb\vkpp.hpp>

#include <curl\curl.h>

namespace cb {
    namespace vkpp {
        class vksession::vksession_impl {
            std::shared_ptr<CURL> curl;
            session_info info_;
          public:
            vksession_impl(const std::string& login,const std::string& password,const std::string& api_key);
            ~vksession_impl();
            session_info info() const;
            bool auth_user(const std::string& login,const std::string& pass);
            bool auth_api(const std::string& api_key);
            std::string raw_call(const std::string& method,const parameter_list& param_list);
        };
    }
}