#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <string>

#include <stack>
#include <memory>

#include "json.hpp"

using namespace std;

namespace crowd
{
    class merkle_tree
    {
    public:
        void create_user(string email, string password);
        int prep_block_creation();
        bool create_hash(const string& unhashed, string& hashed);
    private:
        void save_new_user(string&, string&);
        string two_hours_timer();
        bool is_empty(std::ifstream&);
        shared_ptr<stack<string>> calculate_root_hash(shared_ptr<stack<string>>);
        shared_ptr<stack<string>> pop_two_and_hash(shared_ptr<stack<string>>);
        void create_block(string&, string, nlohmann::json);
        void create_genesis_block(string);
        void add_block_to_blockchain(string);
    };
}

#endif /* MERKLE_TREE_H */