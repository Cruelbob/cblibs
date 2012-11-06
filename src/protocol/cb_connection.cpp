#include <cb_connection.hpp>
#include <cb\binproc.hpp>
#include <boost\bind.hpp>
#include <iostream>

namespace boost { 
    template<class T> const T* get_pointer(const std::shared_ptr<T>& ptr) 
    {
        return ptr.get();
    }

    template<class T> T* get_pointer(std::shared_ptr<T>& ptr)
    {
        return ptr.get();
    }
}

namespace cb {
    connection::connection(std::shared_ptr<boost::asio::ip::tcp::socket>& socket):
      socket_(socket),
      ip_(socket_->remote_endpoint().address().to_string()),
      start_time_(std::chrono::system_clock::now()) {
    }
    std::string connection::get_ip(){
        return ip_;
    }
    std::chrono::seconds connection::get_up_time() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    }
    void connection::send_packet(id_type id,std::vector<uint8_t> data) {
        size_type size = static_cast<size_type>(data.size());
        std::vector<char> packet(size+sizeof(id)+sizeof(size));
        binproc::simple_binary_writer packer(packet);
        packer << id << size;
        packer.write_raw(data.data(),size);
        boost::asio::async_write(*socket_,
                                 boost::asio::buffer(packet),
                                 boost::bind(&connection::on_send_result,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
    }
    void connection::on_send_result(const boost::system::error_code& err_code) {
        if(err_code) {
            on_error(err_code.value());
        }
    }
    void connection::on_receive_result(id_type id,const boost::system::error_code& err_code) {
        if(!err_code) {
            if(id == 0) {
                binproc::simple_binary_reader reader(reinterpret_cast<char*>(input_data_.data()),input_data_.size());
                id_type id;
                size_type size;
                reader >> id >> size;
                read_data(size,id);
            }
            else {
                on_packet(id,input_data_);
                size_t header_size = sizeof(id_type)+sizeof(size_type);
                read_data(header_size,0);
            }
        }
        else {
            on_error(err_code.value());
        }
    }
    void connection::read_data(size_type size,id_type id) {
        input_data_.resize(size);
        boost::asio::async_read(*socket_,
            boost::asio::buffer(input_data_),
            boost::asio::transfer_exactly(size),
            boost::bind(&connection::on_receive_result,
                        shared_from_this(),
                        id,
                        boost::asio::placeholders::error));
    }
    void connection::start() {
        if(input_data_.size() == 0) {
            size_t header_size = sizeof(id_type)+sizeof(size_type);
            read_data(header_size,0);
        }
    }
}