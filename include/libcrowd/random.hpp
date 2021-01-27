#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>

#include <string>

using namespace boost::multiprecision;
using namespace boost::random;

typedef independent_bits_engine<mt19937, 512, uint512_t> generator512_type;

class Random
{
public:
    Random()
    {
        generator512_type gen512;
        number_ = gen512();
    }
    std::string get_random_number()
    {
        std::string str;
        std::stringstream stream;
        stream << std::hex << number_;
        str = stream.str();
        return str;
    }
private:
    uint512_t number_;
};