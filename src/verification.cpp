#include "verification.hpp"

Verification::Verification()
{
    // - hello network!
    // - blockchain folder is_empty; download
    // - blockchain folder not_empty; what is latest block
    // - latest block not_latest; update
    // - new user in network
    // - goodbye network
    
    //std::cout << "blockchain_folder_path: " << blockchain_folder_path << std::endl;
    boost::system::error_code c;
    boost::filesystem::path path(blockchain_folder_path);

    if (!boost::filesystem::exists(path))
    {
        boost::filesystem::create_directory(path);
        Verification::Download();
    }
    else
    {
        bool isEmpty = boost::filesystem::is_empty(path);
        if (isEmpty)
        {
            Verification::Download();
        }
    }

    Verification::Update();
}
bool Verification::Download()
{
    // Add 1 to own user_id and search for next chosen_one in leveldb that is online
}
bool Verification::Update()
{
    // check for the latest block in blockchain folder and verify if that's the latest in the chain
    // if not the latest: update
}