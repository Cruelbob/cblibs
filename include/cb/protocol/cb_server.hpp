#ifndef CB_SERVER_HPP
#define CB_SERVER_HPP

#include <boost\asio.hpp>
#include <cb_connection.hpp>
#include <cstdint>
#include <memory>
#include <boost\bind.hpp>

namespace cb {
    template<typename Connection>
    class server {
        boost::asio::io_service io_service_;
        boost::asio::ip::tcp::acceptor acceptor_;
        void start_accept();
        void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,const boost::system::error_code& err_code);
      public:
        server(uint16_t port);
        void run();
    };
}

namespace cb {
    template<typename Connection> 
    server<Connection>::server(uint16_t port):
        acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {
    }
    template<typename Connection> 
    void server<Connection>::start_accept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_service_);
        acceptor_.async_accept(*socket,
            boost::bind(&server<Connection>::handle_accept,
            this,
            socket,
            boost::asio::placeholders::error));
    }
    template<typename Connection> 
    void server<Connection>::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                           const boost::system::error_code& err_code) {
            if(!err_code) {
                std::shared_ptr<Connection>(new Connection(socket))->start();
            }
            start_accept();
    }
    template<typename Connection> 
    void server<Connection>::run() {
        start_accept();
        io_service_.run();
    }
}
#endif // CB_SERVER_HPP