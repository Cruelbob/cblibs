#include <cb\vkpp.hpp>

namespace cb {
    namespace vkpp {
        vksession_exception::vksession_exception(result_code res_code):
          std::exception(get_error_string(res_code)),
          code_(res_code) {}
        result_code vksession_exception::code() const {
            return code_;
        }
    }
}