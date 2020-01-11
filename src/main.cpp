#include "merkle_tree.hpp"
#include <iostream>
using namespace crowd;
using namespace std;
int main() 
{
    string email, password;

    cout << "Email adress: ";
    cin >> email;
    cout << "Password: ";
    cin >> password;  // credentials.hpp en cpp toevoegen, zenden naar create_hash()

    merkle_tree mt;
    mt.do_something();
    return 0;
}