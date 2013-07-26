#include <string>

#include <cb/protocol.hpp>

#include "packet_ids.hpp"

class my_s_con: public cb::protocol::connection {
  public:
    my_s_con(boost::asio::io_service& io_service):
      cb::protocol::connection(io_service)
    {
        set_on_packet(packet_ids::echo,&my_s_con::on_request,this);
    }
    void on_request(const std::string& str) {
        std::cout << str << " " << str.length() << std::endl;
        send(packet_ids::echo,str,static_cast<size_t>(str.length()));
    }
};

class my_s: public cb::protocol::server {
  public:
    my_s(uint16_t port): cb::protocol::server(port) {}
    std::shared_ptr<cb::protocol::connection> 
        create_connection(boost::asio::io_service& io_service) override
    {
        return std::shared_ptr<cb::protocol::connection>(
                                              new my_s_con(io_service));
    }
};

int main() {
    try {
        setlocale(LC_ALL,"");
        my_s server(1234);
        server.run();
    } catch (const boost::system::system_error& ex) {
        std::cout << ex.code() << std::endl;
    }
    return 0;
}
