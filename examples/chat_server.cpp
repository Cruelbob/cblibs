#include <iostream>
#include <cb/server.hpp>
#include <list>

class server_methods {
  public:
    virtual void send_msg_to_all(std::string& msg) = 0;
};

class my_con: public cb::connection {
    server_methods *sm_;
    std::string nickname;
    void on_error(int err_code) override {
        std::cout << "err: " << err_code << std::endl;
    }
    void on_packet(id_type id,std::vector<uint8_t> data) override {
        if(id == 1) {
            std::string msg(data.begin(),data.end());
            sm_->send_msg_to_all(nickname+":"+msg);
        } else if(id == 2) {
            close();
        } else if(id == 3) {
            nickname.assign(data.begin(),data.end());
        }
    }
  public:
    void send_msg(const std::string& msg) {
        std::vector<uint8_t> data(msg.begin(),msg.end());
        send_packet(1,data);
    }
    my_con(p_socket_type& p_socket): cb::connection(p_socket) {
        std::cout << "new connection from: " << get_ip() << std::endl;
    }
    my_con(p_socket_type& p_socket,server_methods *sm): my_con(p_socket) {
        sm_ = sm;
    }
    ~my_con() override {
        std::cout << "disconnected " << get_ip() << " with up_time: " << get_up_time().count() << " seconds" << std::endl;
    }
};

class my_s: public cb::server<my_con>, public server_methods {
    std::list<std::weak_ptr<my_con>> con_list;
    std::shared_ptr<my_con> create_connection(p_socket_type& p_socket) override {
        con_list.push_back(std::weak_ptr<my_con>());
        auto it = con_list.end();
        --it;
        std::shared_ptr<my_con> p_con(new my_con(p_socket,dynamic_cast<server_methods*>(this)),
                                                 [it,this](my_con* con) {
                                                     delete con;
                                                     erase_con_from_list(it);
                                                 });
        con_list.back() = p_con;
        return p_con;
    }
    void erase_con_from_list(std::list<std::weak_ptr<my_con>>::const_iterator it) {
        con_list.erase(it);
    }
    void send_msg_to_all(std::string& msg) override {
        for(auto& it : con_list) {
            it.lock()->send_msg(msg);
        }
    }
  public:
    my_s(uint16_t port): cb::server<my_con>(port) {}
};

int _tmain(int argc, _TCHAR* argv[])
{
    my_s s1(2546);
    s1.run();
	return 0;
}

