#ifndef BINPROC_HPP
#define BINPROC_HPP

#include <cassert>
#include <type_traits>
#include <cstring>
#include <vector>

#include <cb\endian.hpp>

template<typename BinaryProcessor,typename T>
void process_binary(BinaryProcessor &bp,T &t)
{
    bp & t;
}

namespace cb
{
    namespace binproc
    {
        template<typename SizeType = size_t,typename EndianConv = cb::endian::as_is>
        class binary_reader
        {
            binary_reader(const binary_reader&);
            binary_reader(const binary_reader&&);
            binary_reader& operator=(const binary_reader&);
            binary_reader& operator=(const binary_reader&&);
          public:
            typedef SizeType size_type;
          private:
            char *data_;
            size_type size_;
            size_type get_pointer;
          public:
            binary_reader(void *data,size_type size_in_bytes,size_type read_from_byte = 0):
                 data_(reinterpret_cast<char*>(data)),
                 size_(size_in_bytes),
                 get_pointer(read_from_byte)
            {
                assert(data_);
                assert(size_); 
                assert(get_pointer <= size_);
            }
            binary_reader(std::vector<char> &data,size_type read_from_byte = 0):
                data_(data.data()),
                size_(data.size()),
                get_pointer(read_from_byte)
            {
                assert(size_); 
                assert(get_pointer <= size_);
            }
            template<typename T>
            binary_reader& operator>>(T &arg)
            {
                process_binary(*this,arg);
                return *this;
            }
            template<typename T>
            typename std::enable_if<std::is_arithmetic<T>::value,binary_reader&>::type operator&(T &arg)
            {
                size_t arg_size = sizeof(arg);
                assert(get_pointer+arg_size <= size_);
                arg = EndianConv::convert(*reinterpret_cast<T*>(&data_[get_pointer]));
                get_pointer+=arg_size;
                return *this;
            }
            template<typename T>
            T read()
            {
                T value;
                operator>>(value);
                return value;
            }
            void read_raw(const void *buf,size_type count)
             {
                 assert(get_pointer+count <= size_);
                 std::memcpy(buf,&data_[get_pointer],count);
             }
            void skip(size_type count)
            {
                assert(get_pointer+count <= size_);
                get_pointer+=count;
            }

            size_type bytes_read() const
            {
                return get_pointer;
            }
            char *data()
            {
                return data_;
            }
            size_type size()
            {
                return size_;
            }
        };

        template<typename SizeType = size_t,typename EndianConv = cb::endian::as_is>
        class binary_writer

        {
            binary_writer(const binary_writer&);
            binary_writer(const binary_writer&&);
            binary_writer& operator=(const binary_writer&);
            binary_writer& operator=(const binary_writer&&);
          public:
            typedef SizeType size_type;
          private:
            char *data_;
            size_type size_;
            size_type put_pointer;
          public:
            binary_writer(void *data,size_type size_in_bytes,size_type write_from_byte = 0):
                data_(reinterpret_cast<char*>(data)),
                size_(size_in_bytes),
                put_pointer(write_from_byte)
            {
                assert(data_);
                assert(size_);
                assert(put_pointer <= size_);
            }
            binary_writer(std::vector<char> &data,size_type write_from_byte = 0):
                data_(data.data()),
                size_(data.size()),
                put_pointer(write_from_byte)
            {
                assert(size_);
                assert(put_pointer <= size_);
            }
            template<typename T>
            binary_writer& operator<<(T &arg)
            {
                process_binary(*this,arg);
                return *this;
            }
            template<typename T>
            typename std::enable_if<std::is_arithmetic<T>::value,binary_writer&>::type operator&(T arg)
            {
                size_t arg_size = sizeof(arg);
                assert(put_pointer+arg_size <= size_);
                *reinterpret_cast<T*>(&data_[put_pointer]) = EndianConv::convert(arg);
                put_pointer+=arg_size;
                return *this;
            }
            void write_raw(const void *buf,size_type count)
            {
                assert(put_pointer+count <= size_);
                std::memcpy(&data_[put_pointer],buf,count);
            }
            void skip(size_type count)
            {
                assert(put_pointer+count <= size_);
                put_pointer+=count;
            }

            size_type bytes_written() const
            {
                return put_pointer;
            }
            char *data()
            {
                return data_;
            }
            size_type size()
            {
                return size_;
            }
        };
    
        typedef binary_reader<> simple_binary_reader;
        typedef binary_writer<> simple_binary_writer;
    };
};
#endif // BINPROC_HPP