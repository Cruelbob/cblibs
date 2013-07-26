#ifndef ENDIAN_HPP
#define ENDIAN_HPP

namespace cb {
    namespace endian {
        class as_is {
          public:
            template<typename T>
            static T convert(T t) {
                return t;
            }
        };

        class endian_swap {
          public:
            template<typename T>
            static T convert(T old_e) {
                T le;
                char *p_old_e = reinterpret_cast<char*>(&old_e);
                char *p_new_e = reinterpret_cast<char*>(&le);
                for(size_t i = 0;i < sizeof(T);i++) {
                    p_new_e[i] = p_old_e[sizeof(T)-i-1];
                }
                return le;
            }
        };

        class host_to_net {
          public:
            template<typename T>
              static T convert(T t) 
            {
                return t;
            }
        };

        class net_to_host {
        public:
            template<typename T>
            static T convert(T t) 
            {
                return t;
            }
        };
    }
}

#endif // ENDIAN_HPP
