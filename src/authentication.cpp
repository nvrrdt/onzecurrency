#include <iostream>
#include "authentication.hpp"
#include "merkle_tree.hpp"

using namespace crowd;
using namespace std;

// TODO: use ncurses for proper authentication with integration of @ and . and ****
void authentication::auth()
{
    string email, password;

    cout << "Email adress: ";
    cin >> email;
    cout << "Password: ";
    cin >> password;

    merkle_tree mt;
    mt.create_user(email, password);
}