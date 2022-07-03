#include "synchronisation.hpp"

#include <boost/lexical_cast.hpp>
#include <math.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "print_or_log.hpp"
#include "globals.hpp"
#include "sharding.hpp"


using namespace Poco;
using namespace std::chrono;

std::string Synchronisation::datetime_ = "";

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

    Common::Print_or_log pl;
    pl.handle_print_or_log({"intro_msg_map.size() in Poco:", std::to_string(intro_msg_map_.get_intro_msg_map().size())});
    pl.handle_print_or_log({"transactions.size() in Coin:", std::to_string(tx_.get_transactions().size())});

    //std::thread t1(&Poco::Synchronisation::get_sleep_until, this);

    // chosen ones are being informed here
    std::thread t2(&Poco::PocoCrowd::create_prel_blocks, pococr_);

    // and here too, but for coin then
    // std::thread xxxxxxxxxxxx pococo_.create_prel_blocks_c();     // preliminary commented out

    get_sleep_until();

    //t1.join();
    t2.detach();

    get_sleep_and_create_block();
}

void Synchronisation::get_sleep_until()
{
    // wait x seconds (infinite for loop + break) until datetime + 10, 20, 30s ... in latest_block

    Common::Print_or_log pl;

    // get genesis datetime from latest block
    uint64_t genesis;
    std::istringstream iss(get_genesis_datetime()); // TODO make static variable genesis_datetime
    iss >> genesis;

    Common::Globals globals;
    Poco::DatabaseSharding dbs;

    uint32_t shard_time = (globals.get_block_time() * 1000 /* milliseconds */ ) / dbs.get_amount_of_shards();

    for (;;)
    {
        // get system datetime
        uint64_t now = std::chrono::system_clock::now().time_since_epoch().count();

// pl.handle_print_or_log({"____00000 gsu", std::to_string(shard_time), std::to_string(now - genesis)});
        if ((now - genesis) % shard_time == 0) // let every shard run in time
        {
            std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t(time);
            std::tm tm = *std::gmtime(&tt); //GMT (UTC)
            //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
            std::string format = "UTC: %Y-%m-%d %H:%M:%S";
            std::stringstream ss;
            ss << std::put_time( &tm, format.c_str() );
            pl.handle_print_or_log({"break: datetime block ", std::to_string(now), ss.str()});

            set_datetime_now(std::to_string(now));

            std::this_thread::sleep_for(std::chrono::milliseconds(shard_time));
            break;
        }
    }
}

std::string Synchronisation::get_genesis_datetime() // TODO make a static variable and set it in p2p.cpp
                                                    // otherwise this function is called too much
{
    std::string latest_datetime;

    // read prev_hash file
    Crowd::ConfigDir cd;
    boost::filesystem::path p (cd.GetConfigDir() + "blockchain/crowd");

    Common::Print_or_log pl;

    // if (!boost::filesystem::exists(p)) // --> should already exist --> intro_peer should send the genesis block
    // {
    //     try
    //     {
    //         boost::filesystem::create_directories(p);
    //     }
    //     catch (boost::filesystem::filesystem_error &e)
    //     {
    //         pl.handle_print_or_log({"error: path doesn't exist", e.what()});
    //     }
    // }

    try
    {
        if (boost::filesystem::exists(p))    // does p actually exist?
        {
            if (boost::filesystem::is_regular_file(p))        // is p a regular file?
            {
                pl.handle_print_or_log({p.string(), "size is", boost::lexical_cast<std::string>(boost::filesystem::file_size(p))});
            }
            else if (boost::filesystem::is_directory(p))      // is p a directory?
            {
                pl.handle_print_or_log({p.string(), "is a directory containing:"});

                typedef std::vector<boost::filesystem::path> vec;             // store paths,
                vec v;                                // so we can sort them later

                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));

                sort(v.begin(), v.end());             // sort, since directory iteration
                                                    // is not ordered on some file systems

                // for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                // {
                //     cout << "   " << *it << '\n';
                // }

                // uint64_t n = v.size(); 

                std::ifstream stream(v[0].string(), std::ios::in | std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
                nlohmann::json contents_j = nlohmann::json::parse(contents);

                latest_datetime = contents_j["starttime"];
            }
            else
            {
                pl.handle_print_or_log({p.string(), "exists, but is neither a regular file nor a directory"});
            }
        }
        else
        {
            pl.handle_print_or_log({p.string(), "does not exist"});
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        pl.handle_print_or_log({"error fs:", ex.what()});
    }

    return latest_datetime;
}

void Synchronisation::set_datetime_now(std::string datetime)
{
    datetime_ = datetime;
}

std::string Synchronisation::get_datetime_now()
{
    return datetime_;
}