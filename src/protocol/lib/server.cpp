#include <cb/protocol/server.hpp>

namespace cb {
namespace protocol {
server::server(uint16_t port):
  acceptor_(io_service_,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
{
}
void server::run() {
    start_accept();
    io_service_.run();
}
std::shared_ptr<connection> server::create_connection(
                                        boost::asio::io_service& io_service)
{
    return std::make_shared<connection>(io_service);
}
void server::start_accept() {
    auto con = create_connection(io_service_);
    acceptor_.async_accept(con->socket_,
        [=](const boost::system::error_code& ec) {
            if(!ec) {
                connections_.emplace_back(con);
                con->start();
                auto it = connections_.end();
                --it;
                con->disconnect_handler_ = [it,this]() {      
                    auto p_con = *it;
                    io_service_.post([p_con]{
                    });
                    connections_.erase(it);
                };

                start_accept();
            }
        });
}
}
}
