#include "p2p.hpp"

using namespace Crowd;

/**
 * - version protocol
 * - download blockchain
 * - introduction new user
 * - hello --> online presence
 * - goodbye --> leaving the network
 * - ...
 * 
 * - only calculated users can access the peer, otherwise the upnp provider refuses
 * - lookup peer in map of users
 * - then go to its upnp provider if its upnp isn't enabled
 * - let the protocol work
 * 
 * - a nonce must be used for calculating the chosen_one
 * - otherwise there exists DDOS'ing
 * - if no nonce, the chosen_one is known, and its 'desciples' are also known
 * 
 * - hello: - lookup upnp provider above your own hash, get connected, if upnp provider offline lookup next until one online,
 *            this one gives you correct upnp provider
 *          - then update your map by asking the peer above you
 *          - then communicate to the whole network by which you divide the whole network by x divided by x,
 *            the corresponding users above the calculated hash will communicate to the whole network
 * - goodbye: - communicate to the whole network by which you divide the whole network by x divided by x,
 *              the corresponding users above the calculated hash will communicate to the whole network
 * - version: - just ask the version as first request
 * - download: - download the whole blockchain from 13 users who are the hashes of your hash
 *               or download the latest blocks from 13 users who are the hashes of your hash
 * - new_user: - just a few hardcoded and upnp enabled peers will are used for this case
 *             - the hardcoded peer above the hash of the new user will communicate the new user to all the peers
 */

boost::array<char, 32> Protocol::Request(boost::array<char, 32> r)
{
    //
}

boost::array<char, 32> Protocol::Response(boost::array<char, 32> r)
{
    //
}