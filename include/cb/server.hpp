#ifndef CB_SERVER_HPP
#define CB_SERVER_HPP

#include <boost/asio.hpp>
#include <cb/connection.hpp>
#include <cstdint>
#include <memory>
#include <boost/bind.hpp>

namespace cb {
    template<typename ConnectionType>
    class server {
      protected:
        typedef std::shared_ptr<boost::asio::ip::tcp::socket> p_socket_type;
      private:
        boost::asio::io_service io_service_;
        boost::asio::ip::tcp::acceptor acceptor_;
        void start_accept() {
            auto p_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_service_);
            acceptor_.async_accept(*p_socket,
                boost::bind(&server<ConnectionType>::handle_accept,
                this,
                p_socket,
                boost::asio::placeholders::error));
        }
        void handle_accept(p_socket_type p_socket,const boost::system::error_code& err_code) {
            if(!err_code) {
                create_connection(p_socket)->start();
            }
            start_accept();
        }
      protected:
          virtual std::shared_ptr<ConnectionType> create_connection(p_socket_type& p_socket) {
              return std::make_shared<ConnectionType>(p_socket);
          }
      public:
        server(uint16_t port):
          acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {}
        void run() {
            start_accept();
            io_service_.run();
        }
    };
}
#endif // CB_SERVER_HPP