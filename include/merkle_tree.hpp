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
    private:
        // string create_hash(const string str);
        bool create_hash(const string& unhashed, string& hashed);
    };
}

#endif /* MERKLE_TREE_H */