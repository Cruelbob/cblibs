#ifndef ENDIAN_HPP
#define ENDIAN_HPP

namespace cb
{
    namespace endian
    {
        class as_is
        {
          public:
            template<typename T>
            static T convert(T t)
            {
                return t;
            }
        };

        class endian_swap
        {
          public:
            template<typename T>
            static T convert(T be)
            {
                T le;
                char *p_be = reinterpret_cast<char*>(&be);
                char *p_le = reinterpret_cast<char*>(&le);
                for(size_t i = 0;i < sizeof(T);i++)
                {
                    p_le[i] = p_be[sizeof(T)-i-1];
                }
                return le;
            }
        };
    };
};

#endif // ENDIAN_HPP