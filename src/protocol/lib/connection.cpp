#include <cb/protocol/connection.hpp>

namespace cb {
namespace protocol {
connection::connection(boost::asio::io_service& io_service):
  socket_(io_service)
{
}

void connection::start() {
    read_header();
    on_connect(0);
}

void connection::close() {
    on_close(0);
}

void connection::read_header() {
    auto buf = std::make_shared<buffer_t>(sizeof(id_type)+sizeof(size_type));
    boost::asio::async_read(socket_,boost::asio::buffer(*buf),
                            boost::asio::transfer_all(),
        [=](const boost::system::error_code& ec,
                   std::size_t bytes_read)
        {
            if(!ec) {
                    id_type packet_id;
                    size_type packet_size;
                    net_bin_reader(*buf) >> packet_id >> packet_size;
                    read_body(packet_id,packet_size);
            } else if(ec.value() != boost::asio::error::operation_aborted) {
                on_close(ec.value());
            }
        });
}

void connection::read_body(id_type packet_id,size_type packet_size) {
    auto buf = std::make_shared<buffer_t>(packet_size);
    boost::asio::async_read(socket_,boost::asio::buffer(*buf),
                            boost::asio::transfer_all(),
        [=](const boost::system::error_code& ec,
                             std::size_t bytes_read)
        {
            if(!ec) {
                read_header();
                auto handler = packets_.find(packet_id);
                if(handler != packets_.end()) {
                    //std::cout << "read | ";
                    //for(auto it: *buf) {
                    //    std::cout << std::hex << (int)it << " ";
                    //}
                    //std::cout << "|" << std::dec << std::endl;
                    handler->second(*buf);
                } else {
                    close();
                }
            } else if(ec.value() != boost::asio::error::operation_aborted) {
                on_close(ec.value());
            }
        });
}

void connection::send(const buffer_t& buf) {
    //std::cout << "send | ";
    //for(auto it: buf) {
    //    std::cout << std::hex << (int)it << " ";
    //}
    //std::cout << "|" << std::dec << std::endl;
    auto p_buf = std::make_shared<buffer_t>(buf);
    boost::asio::async_write(socket_,boost::asio::buffer(*p_buf),
                             boost::asio::transfer_all(),
        [this,p_buf](const boost::system::error_code& ec,
                     std::size_t bytes_sent)
        {
            if(ec && (ec.value() != boost::asio::error::operation_aborted)) {
                on_close(ec.value());
            }
        });
}

void connection::on_close(int reason) {
    if(disconnect_handler_) {
        disconnect_handler_();
    }
    on_disconnect(reason);
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
    socket_.close(ec);
}

void connection::remove_handler(id_type packet_id) {
    packets_.erase(packet_id);
}
}
}
