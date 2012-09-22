#include <cb\vkpp.hpp>
#include <vksession_impl.hpp>

namespace cb {
    namespace vkpp {
        vksession::vksession(const std::string& login,const std::string& password,const std::string& api_key):
          impl(new(std::nothrow) vksession_impl(login,password,api_key)) {
            if(!impl) {
                throw cb::error::error(err_codes::ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED,
                                           "Allocating memory for vksession_impl failed");
            }
        }
        vksession::session_info vksession::info() const {
            return impl->info();
        }
        std::string vksession::raw_call(const std::string& method,const parameter_list& param_list) {
            return impl->raw_call(method,param_list);
        }
    }
}