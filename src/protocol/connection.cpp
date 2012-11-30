#include <cb/connection.hpp>
#include <cb/binproc.hpp>
#include <boost/bind.hpp>
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
    connection::connection(p_socket_type& p_socket):
      p_socket_(p_socket),
      ip_(p_socket_->remote_endpoint().address().to_string()),
      start_time_(std::chrono::system_clock::now()),
      is_open(false) {
          std::atomic_init(&cn_waitings,0);
    }
    std::string connection::get_ip(){
        return ip_;
    }
    std::chrono::seconds connection::get_up_time() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    }
    void connection::send_packet(id_type id,std::vector<uint8_t> data) {
        if(!is_open) {
            throw std::exception("the connection is not fully opened.");
        }
        size_type data_size = static_cast<size_type>(data.size());
        size_t header_size = sizeof(id_type)+sizeof(size_type);
        output_data_.resize(data_size+header_size);
        binproc::simple_binary_writer packer(reinterpret_cast<char*>(output_data_.data()),output_data_.size());
        packer << id << data_size;
        if(data.size() != 0) {
            packer.write_raw(data.data(),data_size);
        }
        cn_waitings++;
        boost::asio::async_write(*p_socket_,
                                 boost::asio::buffer(output_data_,output_data_.size()),
                                 boost::bind(&connection::on_send_result,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
    }
    void connection::on_send_result(const boost::system::error_code& err_code) {
        cn_waitings--;
        if(err_code) {
            on_error(err_code.value());
        }
        if(!is_open && (cn_waitings == 0)) {
            p_socket_->close();
        }
    }
    void connection::on_receive_data(id_type id,const boost::system::error_code& err_code) {
        cn_waitings--;
        if(!err_code) {
            if(id == 0) {
                binproc::simple_binary_reader reader(reinterpret_cast<char*>(input_data_.data()),input_data_.size());
                id_type id;
                size_type size;
                reader >> id >> size;
                if(size == 0) {
                    on_packet(id,std::vector<uint8_t>());
                    size_t header_size = sizeof(id_type)+sizeof(size_type);
                    read_data(header_size,0);
                    return;
                }
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
        if(is_open) {
            cn_waitings++;
            input_data_.resize(size);
            boost::asio::async_read(*p_socket_,
                boost::asio::buffer(input_data_),
                boost::asio::transfer_exactly(size),
                boost::bind(&connection::on_receive_data,
                shared_from_this(),
                id,
                boost::asio::placeholders::error));
        }
    }
    void connection::start() {
        if(input_data_.empty()) {
            is_open = true;
            after_opening();
            size_t header_size = sizeof(id_type)+sizeof(size_type);
            read_data(header_size,0);
        }
    }
    void connection::close() {
        is_open = false;
        p_socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
        if(cn_waitings == 0) {
            p_socket_->close();
        }
    }
}