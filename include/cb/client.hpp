#ifndef CB_CLIENT_HPP
#define CB_CLIENT_HPP

#include <boost/asio.hpp>
#include <string>
#include <cstdint>
#include <cb/connection.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace cb {
    using boost::asio::ip::tcp;
    template<typename ConnectionType>
    class client {
        typedef std::shared_ptr<tcp::socket> p_socket_type;
        boost::asio::io_service io_service_;
        tcp::resolver::iterator endpoint_it_;
        std::weak_ptr<ConnectionType> wp_con;
        void start_connection() {
            auto p_socket = std::make_shared<tcp::socket>(io_service_);
            boost::asio::async_connect(*p_socket,
                endpoint_it_,
                boost::bind(&client<ConnectionType>::handle_connect,
                this,
                p_socket,
                boost::asio::placeholders::error));
        }
        void handle_connect(p_socket_type p_socket,const boost::system::error_code& err_code) {
            if(!err_code) {
                auto p_con = create_connection(p_socket);
                wp_con = p_con;
                p_con->start();
            }
        }
      protected:
          virtual std::shared_ptr<ConnectionType> create_connection(p_socket_type& p_socket) {
              //return std::shared_ptr<ConnectionType>(new ConnectionType(p_socket));
              return std::make_shared<ConnectionType>(p_socket);
          }
      public:
        client(std::string host,uint16_t port):
            endpoint_it_(tcp::resolver(io_service_).resolve(tcp::resolver::query(host,boost::lexical_cast<std::string>(port)))){}
        virtual ~client() {}
        void run() {
            start_connection();
            io_service_.run();
        }
    };
}
#endif // CB_CLIENT_HPP
