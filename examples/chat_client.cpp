#include <cb/client.hpp>
#include <iostream>
#include <thread>

class my_con: public cb::connection {
    std::atomic_bool input_enable;
    std::thread user_input_thread;
    std::string nickname;
    void send_msg(const std::string& msg) {
        std::vector<uint8_t> data(msg.begin(),msg.end());
        send_packet(1,data);
    }
    void on_error(int err_code) override {
        std::cout << "err: " << err_code << std::endl;
    }
    void user_input_func() {
        std::string line;
        std::cout << "Enter nickname: ";
        std::getline(std::cin,nickname);
        if(nickname.empty()) {
            send_packet(2);
            close();
            return;
        }
        std::vector<uint8_t> data(nickname.begin(),nickname.end());
        send_packet(3,data);
        std::cout << ">";
        while(std::getline(std::cin,line) && input_enable) {
            if(line == "exit") {
                send_packet(2);
                close();
                return;
            }
            send_msg(line);
            std::cout << ">";
        }
    }
    void after_opening() override {
        input_enable = true;
        user_input_thread = std::thread(&my_con::user_input_func,this);
    }
    void on_packet(id_type id,std::vector<uint8_t> data) override {
        if(id == 1) {
            std::string msg(data.begin(),data.end());
            if(msg.substr(0,nickname.length()) != nickname) {
                std::cout << msg << std::endl;
            }
        }
        else if(id == 2) {
            input_enable = false;
            close();
        }
    }
  public:
    my_con(p_socket_type p_socket): cb::connection(p_socket) {}
    ~my_con() override {
        if(user_input_thread.joinable()) {
            user_input_thread.join();
        }
    }
};

int _tmain(int argc, _TCHAR* argv[])
{
    cb::client<my_con> c1("5.19.251.81",2546);
    c1.run();
	return 0;
}