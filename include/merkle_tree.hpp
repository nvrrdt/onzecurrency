#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <string>

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
        void save_new_user(string&);
        int get_current_utc_time();
        void ticking_clock();

    };
}

#endif /* MERKLE_TREE_H */