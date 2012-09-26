#include <cb\vkpp.hpp>
#include <vksession_impl.hpp>

namespace cb {
    namespace vkpp {
        vksession::vksession(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope) {
            open(login,password,api_key,scope);
        }
        vksession::vksession(result_code& code,const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope) {
            open(code,login,password,api_key,scope);
        }
        vksession::session_info vksession::info() const {
            return impl->info();
        }
        std::string vksession::raw_call(const std::string& method,const parameter_list& param_list) {
            result_code code = result_code::OK;
            std::string result = impl->raw_call(code,method,param_list);
            if(code == result_code::OK) {
                return result;
            }
            close();
            throw vksession_exception(code);
        }
        std::string vksession::raw_call(result_code& code,const std::string& method,const parameter_list& param_list) {
            std::string result = impl->raw_call(code,method,param_list);
            if(code == result_code::OK) {
                return result;
            }
            close();
            return "";
        }
        void vksession::open(result_code& code,const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope) {
            impl.reset(new(std::nothrow) vksession_impl(code,login,password,api_key,scope));
            if(!impl) {
                code = result_code::ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED;
            }
        }
        void vksession::open(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope) {
            result_code code = result_code::OK;
            open(code,login,password,api_key,scope);
            if(!impl) {
                throw vksession_exception(result_code::ALLOC_MEMORY_FOR_VKSESSION_IMPL_FAILED);
            }
            if(code != result_code::OK) {
                throw vksession_exception(code);
            }
        }
        void vksession::close() {
            impl.reset();
        }
        bool vksession::is_open() {
            return impl;
        }
    }
}