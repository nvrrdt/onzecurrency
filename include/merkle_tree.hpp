#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <string>

#include <bits/stdc++.h>

using namespace std;

namespace crowd
{
    class merkle_tree
    {
    public:
        void create_user(string email, string password);
        void create_block();
    private:
        bool create_hash(const string& unhashed, string& hashed);
        void save_new_user(string&, string&);
        int two_hours_timer();
        bool is_empty(std::ifstream&);
        void calculate_root_hash(stack <string>);
    };
}

#endif /* MERKLE_TREE_H */