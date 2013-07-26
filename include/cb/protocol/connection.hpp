#pragma once

#include <cstdint>
#include <map>
#include <functional>
#include <vector>
#include <string>

#include <boost/asio.hpp>

#include <cb/binproc.hpp>

namespace cb {
namespace protocol {
class connection {
    connection(const connection&);
    connection& operator=(const connection&);
    friend class server;
    friend class client;
  public:
    typedef uint16_t id_type;
    typedef uint16_t size_type;
    typedef std::vector<uint8_t> buffer_t;
    typedef binproc::static_buffer_binary_reader
                                <size_type,endian::host_to_net> net_bin_reader;
    typedef binproc::dynamic_buffer_binary_writer
                                <size_type,endian::net_to_host> net_bin_writer;

    connection(boost::asio::io_service& io_service);
    virtual ~connection() {}
   
    void close();
    template<typename... Types>
     void send(id_type packet_id,const Types&... args);
    template<typename F>
      void set_on_packet(id_type packet_id,const F& f);
    template<typename... Types>
      void set_on_packet(id_type packet_id,void (*f)(Types...));
    template<typename C,typename... Types>
      void set_on_packet(id_type packet_id,
                         void (C::* method)(Types...),
                         C * obj);
    template<typename... Types>
      void set_on_packet(id_type packet_id,
                         const std::function<void (Types...)>& f);
	template<typename T1,typename T2>
	  void set_on_packet(id_type packet_id,
						 const std::function<void (T1,T2)>& f);
	template<typename T1, typename T2, typename T3>
	  void set_on_packet(id_type packet_id,
		  const std::function<void (T1,T2,T3)>& f);
    void remove_handler(id_type packet_id);
  protected:
    virtual void on_connect(int result) {}
    virtual void on_disconnect(int reason) {}
  private:
    template<typename T>
      void send(net_bin_writer& writer, const T& value);
    template<typename First,typename... Others>
      void send(net_bin_writer& writer,
                const First& value,
                const Others&... args);
    void send(const buffer_t& buf);
    void start();
    void read_header();
    void read_body(id_type packet_id,size_type packet_size);
    void on_close(int reason);

    boost::asio::ip::tcp::socket socket_;
    std::function<void ()> disconnect_handler_;
    std::map<id_type,
             std::function<void (const buffer_t&)>> packets_;
};

template<typename T>
void connection::send(net_bin_writer& writer, const T& value) {
    writer << value;
    binproc::static_buffer_binary_writer<size_type,endian::host_to_net>
                                    size_writer(writer.data(),sizeof(id_type));
    size_writer << size_type(writer.bytes_written()-sizeof(id_type)-sizeof(size_type));
    send(writer.data());
}

template<typename First,typename... Others>
void connection::send(net_bin_writer& writer,
                      const First& value,
                      const Others&... others)
{
    writer << value;
    send(writer,others...);
}

template<typename... Types>
void connection::send(id_type packet_id,const Types&... args) {
    net_bin_writer writer(100);
    writer << packet_id;
    writer.skip(sizeof(size_type));
    send(writer,args...);
}

namespace detail {
    template<typename T> 
    struct functor_to_function;
    template<typename C,typename ...Types> 
    struct functor_to_function<void (C::*)(Types...) const> {
        typedef std::function<void (Types...)> type;
    };
    struct pass {
        template<typename F,typename... Types>
        pass(const F& f,Types&&... args) {
            f(std::forward<Types>(args)...);
        }
        template<typename C,typename... Types,typename... Args>
        pass(void (C::* method)(Types...),
            C * obj,
            Args&&... args)
        {
            (obj->*method)(std::forward<Args>(args)...);
        }
    };
}

template<typename C,typename... Types>
void connection::set_on_packet(id_type packet_id,
                               void (C::* method)(Types...),
                               C * obj)
{
	set_on_packet(packet_id, [=](Types&&... args) {
		(obj->*method)(std::forward<Types>(args)...);
	});
    /*packets_[packet_id] = [=](const buffer_t& buf) {
        net_bin_reader reader(buf);
        detail::pass{method,obj,reader.read
            <typename std::remove_cv<typename std::remove_reference<Types>::type>::type>()...};
    };*/
}



template<typename... Types>
void connection::set_on_packet(id_type packet_id,void (*f)(Types...)) {
	set_on_packet(packet_id, [=](Types && ... args) {
		f(std::forward<Types>(args)...);
	});
    /*packets_[packet_id] = [=](const buffer_t& buf) {
        net_bin_reader reader(buf);
        detail::pass{f,reader.read
            <typename std::remove_cv<typename std::remove_reference<Types>::type>::type>()...};
    };*/
}

template<typename F>
void connection::set_on_packet(id_type packet_id,const F& f) {
    typename detail::functor_to_function<decltype(&F::operator())>::type functor = f;
    set_on_packet(packet_id,functor);
}

template<typename... Types>
void connection::set_on_packet(id_type packet_id,
	const std::function<void (Types...)>& f)
{
	packets_[packet_id] = [=](const buffer_t& buf) {
		net_bin_reader reader(buf);
		detail::pass{ f, reader.read
			<typename std::remove_cv<typename std::remove_reference<Types>::type>::type>()... };
	};
}

template<typename T1, typename T2>
void connection::set_on_packet(id_type packet_id,
							   const std::function<void (T1,T2)>& f)
{
	packets_[packet_id] = [=](const buffer_t& buf) {
		net_bin_reader reader(buf);
		T1 t1(reader.read
			<typename std::remove_cv<typename std::remove_reference<T1>::type>::type>());
		T2 t2(reader.read
			<typename std::remove_cv<typename std::remove_reference<T2>::type>::type>());
		f(t1, t2);
	};
}

template<typename T1, typename T2, typename T3>
void connection::set_on_packet(id_type packet_id,
							   const std::function<void (T1,T2,T3)>& f)
{
	packets_[packet_id] = [=](const buffer_t& buf) {
		net_bin_reader reader(buf);
		T1 t1(reader.read
			<typename std::remove_cv<typename std::remove_reference<T1>::type>::type>());
		T2 t2(reader.read
			<typename std::remove_cv<typename std::remove_reference<T2>::type>::type>());
		T3 t3(reader.read
			<typename std::remove_cv<typename std::remove_reference<T2>::type>::type>());
		f(t1,t2,t3);
	};
}
}
}
