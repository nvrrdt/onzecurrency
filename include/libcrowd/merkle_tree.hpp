#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <string>

#include <stack>
#include <memory>

#include "json.hpp"

using namespace std;

extern bool break_server_loop;

namespace Crowd
{
    class merkle_tree
    {
    public:
        void create_user(string email, string password);
        int prep_block_creation();
        void create_block(string&, string, nlohmann::json);
        string two_hours_timer();
        std::string time_now();
        shared_ptr<stack<string>> calculate_root_hash(shared_ptr<stack<string>>);
    private:
        void save_new_user(string&, string&);
        bool is_empty(std::ifstream&);
        shared_ptr<stack<string>> pop_two_and_hash(shared_ptr<stack<string>>);
        void create_genesis_block(string, nlohmann::json);
        void add_block_to_blockchain(string);
    };
}

#endif /* MERKLE_TREE_H */