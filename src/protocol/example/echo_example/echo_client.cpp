#include <string>
#include <iostream>
#include <future>

#include <cb/protocol.hpp>

#include "packet_ids.hpp"

int main(int argc,char** argv) {
    boost::asio::io_service io_service;
    try {
        cb::protocol::client mc(io_service);
        mc.set_on_packet(msgs::s2c::echo,[&](const std::string& str) {
            std::cout << "from server: " << str << std::endl;
            mc.close();
        });
        auto con_res = mc.connect("127.0.0.1",1234);
        auto con_thread = std::async(std::launch::async,[&]()->size_t {
            return io_service.run();
        });
        if(con_res.get() == 0) {
            std::string str = argc > 1 ? argv[1] : "hello";
            std::cout << "to server: " << str << std::endl;
            mc.send(msgs::c2s::echo,str);
        }
        con_thread.get();
    } catch(const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
