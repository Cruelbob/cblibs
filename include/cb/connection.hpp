#ifndef CB_CONNECTION_HPP
#define CB_CONNECTION_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <boost/asio.hpp>
#include <memory>
#include <atomic>

namespace cb {
    class connection: public std::enable_shared_from_this<connection> {
      public:
        typedef uint16_t id_type;
        typedef uint16_t size_type;
        typedef std::shared_ptr<boost::asio::ip::tcp::socket> p_socket_type;
      private:
        std::shared_ptr<boost::asio::ip::tcp::socket> p_socket_;
        std::string ip_;
        std::chrono::system_clock::time_point start_time_;
        std::vector<uint8_t> input_data_;
        std::vector<uint8_t> output_data_;
        bool is_open;
        std::atomic<int> cn_waitings;
        void on_receive_data(uint16_t id,const boost::system::error_code& err_code);
        //void on_receive_header(const boost::system::error_code& err_code);
        void on_send_result(const boost::system::error_code& err_code);
        void read_data(size_type size,id_type id);
        //void read_header();
      protected:
        virtual void on_packet(uint16_t id,std::vector<uint8_t> data) {
        }
        virtual void on_error(int err_code) {
        }
        virtual void after_opening() {}
        void send_packet(uint16_t id,std::vector<uint8_t> data = std::vector<uint8_t>());
      public:
        void start();
        void close();
        connection(p_socket_type& p_socket);
        virtual ~connection() {};
        std::string get_ip();
        std::chrono::seconds get_up_time();
    };
}

#endif // CB_CONNECTION_HPP
