#pragma once

#include <memory>
#include <list>

#include "connection.hpp"

namespace cb {
namespace protocol {
template<typename ConnType>
class server {
    server(const server&);
    server& operator=(const server&);
  public:
    server(uint16_t port);
    virtual ~server() {}

    void run();
  protected:
	virtual std::shared_ptr<ConnType>
                        create_connection(boost::asio::io_service& io_service);
  private:
    void start_accept();

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
	std::list<std::shared_ptr<ConnType>> connections_;
};

template<typename ConnType>
server<ConnType>::server(uint16_t port) :
	acceptor_(io_service_,
	boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

template<typename ConnType>
void server<ConnType>::run() {
	start_accept();
	io_service_.run();
}

template<typename ConnType>
std::shared_ptr<ConnType> server<ConnType>::create_connection(
	boost::asio::io_service& io_service)
{
	return std::make_shared<ConnType>(io_service);
}

template<typename ConnType>
void server<ConnType>::start_accept() {
	auto con = create_connection(io_service_);
	acceptor_.async_accept(con->socket_,
		[=](const boost::system::error_code& ec) {
			if (!ec) {
				connections_.emplace_back(con);
				con->start();
				auto it = connections_.end();
				--it;
				con->disconnect_handler_ = [it, this]() {
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
