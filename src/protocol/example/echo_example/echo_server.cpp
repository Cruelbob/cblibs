#include <string>
#include <algorithm>

#include <cb/protocol.hpp>

#include "packet_ids.hpp"

class my_s_con: public cb::protocol::connection {
  public:
    my_s_con(boost::asio::io_service& io_service):
      cb::protocol::connection(io_service)
    {
        set_on_packet(msgs::c2s::echo,&my_s_con::on_request,this);
    }
    void on_request(std::string& str) {
        std::reverse(str.begin(), str.end());
        send(msgs::s2c::echo,str);
        close();
    }
};

int main() {
    try {
        cb::protocol::server<my_s_con> server(1234);
        server.run();
    } catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
