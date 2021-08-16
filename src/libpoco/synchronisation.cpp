#include "synchronisation.hpp"

using namespace Poco;

bool Synchronisation::break_block_creation_loops_ = false;

/**
 * synchronisation is necessary
 * 
 * 1st peer creation --> 2nd peer creation --> reward tx to 1st peer --> 3rd peer creation --> reward txs to first 2 peers
 * 
 * poco introduction for crowd first ...
 * then trying to make poco work for coin ...
 * 
 * calculating new blocks should be ceased and start over with a new vector after the block creation delay (here 10 seconds)
 */

void Synchronisation::get_sleep_and_create_block()
{
    // need to read the time in the block for full synchronisation
    // get_latest_block --> get datetime from block && get system time --> proceed when multiple of 10 or ???

    set_break_block_creation_loops(false); // break the loops in poco_crowd

    std::cout << "intro_msg_vec.size() in Poco: " << intro_msg_vec_.get_intro_msg_vec().size() << std::endl;
    std::cout << "transactions.size() in Coin: " << tx_.get_transactions().size() << std::endl;

    // chosen ones are being informed here
    std::thread t1(&Poco::PocoCrowd::create_and_send_block, pococr_);

    // and here too, but for coin then
    bmc_->add_received_block_vector_to_received_block_matrix();
    // std::thread xxxxxxxxxxxx pococo_.create_and_send_block_c();     // preliminary commented out

    std::thread t3(&Poco::Synchronisation::get_sleep_until, this);

    set_break_block_creation_loops(true);

    t1.join();
    t3.join();

    get_sleep_and_create_block();
}

void Synchronisation::get_sleep_until()
{
    // wait x seconds (infinite for loop + break) until datetime + 10, 20, 30s ... in latest_block

    // get datetime from latest block
    std::string datetime = get_latest_datetime();

    int yy, month, dd, hh, mm, ss;
    struct tm whenStart;
    const char *zStart = datetime.c_str();

    sscanf(zStart, "%4d/%02d/%02d %02d:%02d:%02d", &yy, &month, &dd, &hh, &mm, &ss);
    whenStart.tm_year = yy - 1900;
    whenStart.tm_mon = month - 1;
    whenStart.tm_mday = dd;
    whenStart.tm_hour = hh;
    whenStart.tm_min = mm;
    whenStart.tm_sec = ss;
    whenStart.tm_isdst = -1;

    std::time_t time = mktime(&whenStart);
    std::tm utc_tm_block = *gmtime(&time);
std::cout << "datetime block " << utc_tm_block.tm_year + 1900 << "/" << utc_tm_block.tm_mon + 1 << "/" << utc_tm_block.tm_mday << " " << utc_tm_block.tm_hour << ":" << utc_tm_block.tm_min << ":" << utc_tm_block.tm_sec << std::endl;
    for (;;)
    {
        // get system datetime
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm utc_tm = *gmtime(&tt);
        
        if (utc_tm.tm_sec % 10 == utc_tm_block.tm_sec % 10) break; // 10 = every 10 seconds
    }
}

std::string Synchronisation::get_latest_datetime()
{
    std::string latest_datetime;

    // read prev_hash file
    Crowd::ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/crowd");

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
            if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            {
                std::cout << p << " size is " << boost::filesystem::file_size(p) << '\n';
            }
            else if (boost::filesystem::is_directory(p))      // is p a directory?
            {
                std::cout << p << " is a directory containing:\n";

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                // for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                // {
                //     cout << "   " << *it << '\n';
                // }

                uint64_t n = v.size(); 

                std::ifstream stream(v[n-1].string(), std::ios::in | std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                nlohmann::json contents_j = nlohmann::json::parse(contents);

                latest_datetime = contents_j["starttime"];
            }
            else
            {
                std::cout << p << " exists, but is neither a regular file nor a directory\n";
            }
        }
        else
        {
            std::cout << p << " does not exist\n";
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        std::cout << ex.what() << '\n';
    }

    return latest_datetime;
}
