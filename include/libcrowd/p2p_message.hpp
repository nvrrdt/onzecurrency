#ifndef P2P_MESSAGE_HPP
#define P2P_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class p2p_message
{
public:
    enum
    {
        header_length = 5 // 4 for body length and 1 for eom_flag
    };
    enum
    {
        max_body_length = 512
    };

    p2p_message()
        : body_length_(0)
    {
    }

    const char *data() const
    {
        return data_;
    }

    char *data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return header_length + body_length_;
    }

    const char *body() const
    {
        return data_ + header_length;
    }

    char *body()
    {
        return data_ + header_length;
    }

    std::size_t body_length() const
    {
        return body_length_;
    }

    void body_length(std::size_t new_length)
    {
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header()
    {
        char header[header_length + 1] = "";
        std::strncat(header, data_, header_length);

        char body_l[4 + 1] = "";
        body_l[0] = header[0];
        body_l[1] = header[1];
        body_l[2] = header[2];
        body_l[3] = header[3];
        char eom_f[1 + 1] = "";
        eom_f[0] = header[4];

        body_length_ = std::atoi(body_l);
        if (body_length_ > max_body_length)
        {
            body_length_ = 0;
            return false;
        }

        eom_flag_ = std::atoi(eom_f);

        return true;
    }

    bool get_eom_flag()
    {
        return eom_flag_ ? true : false;        
    }

    int get_body_length()
    {
        return body_length_;
    }

    void encode_header(int eom_flag)
    {
        char header[header_length + 1] = "";
        std::sprintf(header, "%04d%1d", static_cast<int>(body_length_), eom_flag);
        std::memcpy(data_, header, header_length);
    }

private:
    char data_[header_length + max_body_length];
    std::size_t body_length_;
    bool eom_flag_;
};

#endif // P2P_MESSAGE_HPP