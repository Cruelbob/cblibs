#include <string>
#include <iostream>
#include <future>

#include <cb/protocol.hpp>

#include "packet_ids.hpp"

int main(int argc,char** argv) {
    boost::asio::io_service io_service;
    try {
        cb::protocol::client mc(io_service);
        mc.set_on_packet(packet_ids::echo,[&](const std::string& str,size_t length) {
            std::cout << "length of \"" << str << "\" is " << length << std::endl;
            mc.close();
        });
        auto con_res = mc.connect("127.0.0.1",1234);
        auto con_thread = std::async(std::launch::async,[&] {
            io_service.run();
        });
        if(con_res.get() == 0) {
            mc.send(packet_ids::echo,argc > 1 ? std::string(argv[1]) : 
                                                std::string("hello"));
        }
        con_thread.get();
    } catch(const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
