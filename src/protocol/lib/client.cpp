#include <cb/protocol/client.hpp>

namespace cb{
namespace protocol {
client::client(boost::asio::io_service& io_service):
  connection(io_service)
{
}
std::future<int> client::connect(std::string host,uint16_t port) {
    boost::asio::ip::tcp::resolver resolver(socket_.get_io_service());
    boost::asio::ip::tcp::resolver::query query(host,std::to_string(port));
    auto endpoint_iterator = resolver.resolve(query);
    auto prms = std::make_shared<std::promise<int>>();
    auto future = prms->get_future();
    boost::asio::async_connect(socket_,endpoint_iterator,
    [=](boost::system::error_code ec,
        boost::asio::ip::tcp::resolver::iterator)
    {
        if(!ec) {
            start();
        }
        prms->set_value(ec.value());
    });
    return future;
}
}
}
