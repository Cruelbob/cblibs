#pragma once

#include <string>
#include <cstdint>
#include <future>

#include <boost/asio.hpp>

#include "connection.hpp"

namespace cb {
namespace protocol {
class client: public connection {
    client(const client&);
    client& operator=(const client&);
  public:
    client(boost::asio::io_service& io_service);

    std::future<int> connect(std::string host,uint16_t port);
};
}
}