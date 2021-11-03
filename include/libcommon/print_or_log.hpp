#pragma once

#include <iostream>

#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"

#include "globals.hpp"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace Common
{
    class Print_or_log
    {
    public:
        void init()
        {
            std::string path = "/onzecurrency/";
            if (boost::filesystem::exists(path))
            {
                plog::init(plog::verbose, "/onzecurrency/.config/onzehub/log/loggi", 2000000, 3); // Initialize the logger
            }
            else
            {
                plog::init(plog::verbose, ".config/onzehub/log/loggi", 2000000, 3); // Initialize the logger
            }
        }

        void handle_print_or_log(std::vector<std::string> message)
        {
            Common::Globals g;
            if (g.get_use_log())
            {
                PLOG_VERBOSE_IF(message.size() == 1) << message[0];
                PLOG_VERBOSE_IF(message.size() == 2) << message[0] << " " << message[1];
                PLOG_VERBOSE_IF(message.size() == 3) << message[0] << " " << message[1] << " " << message[2];
                PLOG_VERBOSE_IF(message.size() == 4) << message[0] << " " << message[1] << " " << message[2] << " " << message[3];
                PLOG_VERBOSE_IF(message.size() == 5) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4];
                PLOG_VERBOSE_IF(message.size() == 6) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4] << message[5];
                PLOG_VERBOSE_IF(message.size() == 7) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4] << message[5] << message[6];
                PLOG_VERBOSE_IF(message.size() == 8) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4] << message[5] << message[6] << message[7];
                PLOG_VERBOSE_IF(message.size() == 9) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4] << message[5] << message[6] << message[7] << message[8];
                PLOG_VERBOSE_IF(message.size() == 12) << message[0] << " " << message[1] << " " << message[2] << " " << message[3] << " " << message[4] << message[5] << message[6] << message[7] << message[8] << message[9] << message[10] << message[11];
            }
            else
            {
                for (auto& m: message)
                {
                    std::cout << m << " ";
                }
                std::cout << std::endl;
            }
        }
    private:
    };
}