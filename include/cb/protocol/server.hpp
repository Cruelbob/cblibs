#pragma once

#include <memory>
#include <list>

#include "connection.hpp"

namespace cb {
namespace protocol {
class server {
    server(const server&);
    server& operator=(const server&);
  public:
    server(uint16_t port);
    virtual ~server() {}

    void run();
  protected:
    virtual std::shared_ptr<connection> 
                        create_connection(boost::asio::io_service& io_service);
  private:
    void start_accept();

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::list<std::shared_ptr<connection>> connections_;
};
}
}
