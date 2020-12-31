#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "p2p.hpp"
#include "json.hpp"

using namespace Crowd;

using boost::asio::ip::tcp;

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        do_read();
    }

private:
    tcp_connection(boost::asio::io_context& io_context)
      : socket_(io_context)
    {
    }

    void do_read()
    {
        auto self(shared_from_this());

        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec) {
                    std::cout << "1: " << data_ << std::endl;
                    handle_read();
                    std::cout << "2: " << data_ << std::endl;
                    do_write(length);
                }
                std::fill_n(data_, length, 0);
            });

    }

    void handle_read()
    {
        std::string str_data_(data_);
        nlohmann::json data_j = nlohmann::json::parse(str_data_);
        if (data_j["msg"] == "register")
        {
            nlohmann::json data_resp_j;
            data_resp_j["register"] = "ack";
            std::string data_str_j = data_resp_j.dump();
            strncpy(data_, data_str_j.c_str(), sizeof(data_));
            data_[sizeof(data_) - 1] = 0;
        }
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                  do_read();
                }
            });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), 1975))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection =
          tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
              boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
          new_connection->start();
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int Tcp::server(std::string msg)
{
    try
    {
        std::cout << "11111" << std::endl;
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
