#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "p2p_message.hpp"
#include "p2p.hpp"

using namespace Crowd;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<p2p_message> p2p_message_queue;

//----------------------------------------------------------------------

class p2p_participant
{
public:
    virtual ~p2p_participant() {}
    virtual void deliver(const p2p_message &msg) = 0;
};

typedef std::shared_ptr<p2p_participant> p2p_participant_ptr;

//----------------------------------------------------------------------

class p2p_room
{
public:
    void join(p2p_participant_ptr participant)
    {
        participants_.insert(participant);
        for (auto msg : recent_msgs_)
            participant->deliver(msg);
    }

    void leave(p2p_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const p2p_message &msg)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for (auto participant : participants_)
            participant->deliver(msg);
    }

private:
    std::set<p2p_participant_ptr> participants_;
    enum
    {
        max_recent_msgs = 100
    };
    p2p_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class p2p_session
    : public p2p_participant,
      public std::enable_shared_from_this<p2p_session>
{
public:
    p2p_session(tcp::socket socket, p2p_room &room)
        : socket_(std::move(socket)),
          room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const p2p_message &msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), p2p_message::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec)
                                    {
                                        room_.deliver(read_msg_);
                                        do_read_header();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             std::cout << "sending response" << std::endl;
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         room_.leave(shared_from_this());
                                     }
                                 });
    }

    tcp::socket socket_;
    p2p_room &room_;
    p2p_message read_msg_;
    p2p_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class p2p_server
{
public:
    p2p_server(boost::asio::io_context &io_context,
                const tcp::endpoint &endpoint)
        : acceptor_(io_context, endpoint)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec)
                {
                    std::make_shared<p2p_session>(std::move(socket), room_)->start();
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    p2p_room room_;
};

//----------------------------------------------------------------------

int Tcp::server(std::string msg)
{
    try
    {
        boost::asio::io_context io_context;

        std::list<p2p_server> servers;
        tcp::endpoint endpoint(tcp::v4(), 1975);
        servers.emplace_back(io_context, endpoint);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}