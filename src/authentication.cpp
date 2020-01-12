#include <iostream>
#include "authentication.hpp"

using namespace crowd;
using namespace std;

void authentication::auth()
{
    string email, password;

    cout << "Email adress: ";
    cin >> email;
    cout << "Password: ";
    cin >> password;  // credentials.hpp en cpp toevoegen, zenden naar create_hash()
}