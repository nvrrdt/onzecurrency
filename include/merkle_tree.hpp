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
        void create_block(string email, string password);
    private:
        string create_hash(const string str);
    };
}

#endif /* MERKLE_TREE_H */