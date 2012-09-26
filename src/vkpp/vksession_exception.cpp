#include <cb\vkpp.hpp>

namespace cb {
    namespace vkpp {
        vksession_exception::vksession_exception(result_code res_code): code_(res_code) {}
        const char* vksession_exception::what() {
            return get_error_string(code_);
        }
        result_code vksession_exception::code() {
            return code_;
        }
    }
}