#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <memory>

#include "json.hpp"

namespace Crowd
{
    class CreateBlock
    {
    public:
        CreateBlock(std::vector<nlohmann::json> &message_j_vec);
        nlohmann::json get_block_j();
        std::string get_hash_of_new_block();
    private:
        void set_hash_of_new_block(std::string block);
    private:
        std::vector<nlohmann::json> message_j_vec_;
        nlohmann::json block_j_;
        std::shared_ptr<std::stack<std::string>> s_shptr_ = std::make_shared<std::stack<std::string>>();
        std::string hash_of_block_;
    };
}