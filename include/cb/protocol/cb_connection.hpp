#ifndef CB_CONNECTION_HPP
#define CB_CONNECTION_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <boost\asio.hpp>
#include <memory>

namespace cb {
    class connection: public std::enable_shared_from_this<connection> {
        typedef uint16_t id_type;
        typedef uint16_t size_type;
        std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
        std::string ip_;
        std::chrono::system_clock::time_point start_time_;
        std::vector<uint8_t> input_data_;
        void on_receive_result(uint16_t id,const boost::system::error_code& err_code);
        void on_send_result(const boost::system::error_code& err_code);
        void read_data(size_type size,id_type id);
        void send_packet(uint16_t id,std::vector<uint8_t> data);
      protected:
        virtual void on_packet(uint16_t id,std::vector<uint8_t> data) {
        }
        virtual void on_error(int err_code) {
        }
      public:
        void start();
        connection(std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
        virtual ~connection() {};
        std::string get_ip();
        std::chrono::seconds get_up_time();
    };
}

#endif // CB_CONNECTION_HPP