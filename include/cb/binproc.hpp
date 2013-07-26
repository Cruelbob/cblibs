#ifndef BINPROC_HPP
#define BINPROC_HPP

#include <cassert>
#include <type_traits>
#include <cstring>
#include <vector>
#include <tuple>
#include <string>
#include <typeinfo>

#include <cb/endian.hpp>

namespace cb
{
    namespace binproc
    {
        template<typename BinaryProcessor,typename T>
        void process_binary(BinaryProcessor &bp,T &t)
        {
            bp & t;
        }

        template<typename SizeType = size_t,typename EndianConv = cb::endian::as_is>
        class static_buffer_binary_reader
        {
            static_buffer_binary_reader(const static_buffer_binary_reader&);
            static_buffer_binary_reader(const static_buffer_binary_reader&&);
          public:
            typedef SizeType size_type;
          private:
            const uint8_t *data_;
            size_type size_;
            size_type get_pointer;
          public:
            static_buffer_binary_reader(void *data,size_type size_in_bytes,size_type read_from_byte = 0):
                 data_(reinterpret_cast<uint8_t*>(data)),
                 size_(size_in_bytes),
                 get_pointer(read_from_byte)
            {
                assert(data_);
                assert(size_); 
                assert(get_pointer <= size_);
            }
            template<typename T>
              static_buffer_binary_reader(const std::vector<T> &data,size_type read_from_byte = 0):
                data_(reinterpret_cast<const uint8_t*>(data.data())),
                size_(data.size()*sizeof(T)),
                get_pointer(read_from_byte)
            {
                assert(size_); 
                assert(get_pointer <= size_);
            }
            template<typename T>
              static_buffer_binary_reader& operator>>(T &arg)
            {
                process_binary(*this,arg);
                return *this;
            }
            template<typename T>
              typename std::enable_if<std::is_arithmetic<T>::value,static_buffer_binary_reader&>::type operator&(T &arg)
            {
                size_t arg_size = sizeof(arg);
                assert(get_pointer+arg_size <= size_);
                arg = EndianConv::convert(*reinterpret_cast<const T*>(&data_[get_pointer]));
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
            void read_raw(void *buf,size_type count)
            {
                assert(get_pointer+count <= size_);
                std::memcpy(buf,&data_[get_pointer],count);
                get_pointer+=count;
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
            const uint8_t *data()
            {
                return data_;
            }
            size_type size()
            {
                return size_;
            }
        };

        template<typename SizeType = size_t,typename EndianConv = cb::endian::as_is>
        class static_buffer_binary_writer

        {
            static_buffer_binary_writer(const static_buffer_binary_writer&);
            static_buffer_binary_writer(const static_buffer_binary_writer&&);
          public:
            typedef SizeType size_type;
          private:
            uint8_t *data_;
            size_type size_;
            size_type put_pointer;
          public:
            static_buffer_binary_writer(void *data,size_type size_in_bytes,size_type write_from_byte = 0):
                data_(reinterpret_cast<uint8_t*>(data)),
                size_(size_in_bytes),
                put_pointer(write_from_byte)
            {
                assert(data_);
                assert(size_);
                assert(put_pointer <= size_);
            }
            template<typename T>
              static_buffer_binary_writer(std::vector<T> &data,size_type write_from_byte = 0):
                data_(reinterpret_cast<uint8_t*>(data.data())),
                size_(data.size()*sizeof(T)),
                put_pointer(write_from_byte)
            {
                assert(size_);
                assert(put_pointer <= size_);
            }
            template<typename T>
              static_buffer_binary_writer& operator<<(const T& arg)
            {
                process_binary(*this,arg);
                return *this;
            }
            template<typename T>
              typename std::enable_if<std::is_arithmetic<T>::value,static_buffer_binary_writer&>::type operator&(const T& arg)
            {
                size_t arg_size = sizeof(arg);
                if(put_pointer+arg_size <= size_) {
                    *reinterpret_cast<T*>(&data_[put_pointer]) = EndianConv::convert(arg);
                    put_pointer+=arg_size;
                }
                return *this;
            }
            void write_raw(const void *buf,size_type count)
            {
                if (put_pointer+count <= size_) {
                    std::memcpy(&data_[put_pointer],buf,count);
                    put_pointer+=count;
                }
            }
            void skip(size_type count)
            {
                if(put_pointer+count <= size_) {
                    put_pointer+=count;
                }
            }
            template<typename T>
              void write(const T& arg) {
                operator<<(arg);
            }
            size_type bytes_written() const
            {
                return put_pointer;
            }
            uint8_t *data()
            {
                return data_;
            }
            size_type size()
            {
                return size_;
            }
        };
    
        template<typename SizeType = size_t,typename EndianConv = cb::endian::as_is>
        class dynamic_buffer_binary_writer {
            dynamic_buffer_binary_writer(const dynamic_buffer_binary_writer&);
            dynamic_buffer_binary_writer(const dynamic_buffer_binary_writer&&);
        public:
            typedef SizeType size_type;
        private:
            std::vector<uint8_t> data_;
        public:
            dynamic_buffer_binary_writer(size_type bytes_to_reserve = 0,size_type write_from_byte = 0) {
                data_.reserve(bytes_to_reserve);
                data_.resize(write_from_byte);
            }
            //binary_writer(std::vector<uint8_t>&& buf,size_type write_from_byte = 0):
            //  data_(std::move(buf))
            //{
            //    data_.resize(write_from_byte);
            //}

            template<typename T>
            dynamic_buffer_binary_writer& operator<<(const T& arg)
            {
                process_binary(*this,arg);
                return *this;
            }
            template<typename T>
            typename std::enable_if<std::is_arithmetic<T>::value,dynamic_buffer_binary_writer&>::type operator&(const T& arg)
            {
                size_t arg_size = sizeof(arg);
                size_t put_pointer = data_.size();
                data_.resize(data_.size()+arg_size);
                *reinterpret_cast<T*>(&data_[put_pointer]) = EndianConv::convert(arg);
                return *this;
            }
            void write_raw(const void *buf,size_type count)
            {
                size_t put_pointer = data_.size();
                data_.resize(data_.size()+count);
                std::memcpy(&data_[put_pointer],buf,count);
            }
            void skip(size_type count)
            {
                data_.resize(data_.size()+count);
            }
            template<typename T>
            void write(const T& arg) {
                operator<<(arg);
            }
            size_type bytes_written() const
            {
                return data_.size();
            }
            std::vector<uint8_t>& data()
            {
                return data_;
            }
        };

        typedef static_buffer_binary_reader<> simple_binary_reader;
        typedef dynamic_buffer_binary_writer<> simple_binary_writer;

        template<typename BinaryReader>
        void process_binary(BinaryReader &bp,std::string &str)
        {
            typename BinaryReader::size_type size;
            bp >> size;
            std::vector<char> temp(size);
            bp.read_raw(temp.data(),size);
            str.assign(temp.begin(),temp.end());
        }
        
        template<typename BinaryWriter>
        void process_binary(BinaryWriter &bp,const std::string& str) {
            bp << (typename BinaryWriter::size_type)str.size();
            bp.write_raw(str.c_str(),str.size());
        }


    }
}
#endif // BINPROC_HPP
