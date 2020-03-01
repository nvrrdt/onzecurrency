#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <string>

#include <stack>
#include <memory>

using namespace std;

namespace crowd
{
    class merkle_tree
    {
    public:
        void create_user(string email, string password);
        void prep_block_creation();
    private:
        bool create_hash(const string& unhashed, string& hashed);
        void save_new_user(string&, string&);
        int two_hours_timer();
        bool is_empty(std::ifstream&);
        shared_ptr<stack<string>> calculate_root_hash(shared_ptr<stack<string>>);
        shared_ptr<stack<string>> pop_two_and_hash(shared_ptr<stack<string>>);
        void create_block();
    };
}

#endif /* MERKLE_TREE_H */