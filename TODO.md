### Some ideas:
- restructuring the merkle tree of users by removing all the accounts that weren't used in the last 2 years should be possible
- removal of users who don't use crowd for 2 years
- onzecoin: rewarding system like bitcoin: a few years 1000 coins you get when you're the chosen one and then a halving to 500 coins for the chosen one, so there's also a limited amount of coins
- create bool functions that can be allowed with an if statement like for authentication ans an example at the evp code in block.cpp
- maybe search for asynchronous daytime server asio
- TODO: block malicious users who in more than 51% of following verifications of the block's hash don't approve the block's hash in the case there's at least a 10-33% (amount of users decides, also amount of shards) users approving of the hash - please do not choke the system
- Almost ZK solution: this describes a tx as data for in the block: a & b, userhash a & userhash b, end credit a & end credit b, begin credit a & begin crediet b, a hash from userhash & begin & end credit of a & the same for b & a hash from both hashes to include as data in the block, if a pays a new amount after the last tx, then only his userhash & prev end amount & prev begin amount is to be seen to c (a new b) with the hash from the prev b to verify the root hash from prev hash a & prev hash b; verification should also be done from the roothash in the crowd way of verification; what if the crowd verifier is malicious: then include the malicious one's hash in the root hash to form a new crowd verifier; there's the 2 last transactions that is to be known; the verifier should verify a correct tx and its hashes;;;; maybe it also works with a hash from userhash and end credit of both, while only communicating a tx amount to the verifier but not putting this amount in the blockchain, then there's only one tx shown in the data that the payee gets to see; if a malicious verifier is chosen by payer and payee, then the verifier should be included in the verifier of the block, so extra work for the verifier of the block who should verify every verifier of a tx;
- ATTENTION: there should be a sorted array for new_users_pool.json where differences of amount of users should lead to a different arrays of users (-1, 0, +1, or so) where more than 51% of online useres agrees on the final chosen_one
- when logging in with an email adress, people could flood crowd with many emailadresses to improve their chances at winning a reward (yes for the coin there's a reward system), that's why there must be quickly built a login system that makes use of a fingerprint or a facial recognition, maybe we should prevent logging in from a pc or so to prevent that flooding, anyone has a smartphone. For development we could make categories of developers who are written in de blockchain or so. Also peers must be able to see a videostream of you logging in to be able to create and verify the resulting hash, if this is doable.
- The website hook: <H1>THE NEXT BLOCKCHAIN FOR THE CHEAP.</H1><H2>No energy mungering monstrosity, no concentrated stakeholders, best decentralisation of the market. And rewards.</H2>
- The login system must be seriously improved, as someone can inject someone else's hashed credentials and no one would notice. A solution would be to let the chosen_one (resulting hash of email_h + passw_h) verify the non-hashed email and communicate this to a few peers who could verify in their turn.
- Another thing that's possible is, instead of pinging the whole blockchain to verify for a hash, just to ping the 100 online users after the chosen_one and see if 51% of those callees respond the same way.
- if a chosen_one is malacious found by some peers, who verify the chosen_one's hash, remember and make then a block for the last 4 hours, if in the meantime a new chosen_one is found and proves the prev chosen_one is not malaficious then those peers, who found the peer malacious, communicate to their user that their software is malicious. If the chose_one is indeed malicious found by the new chosen_one then again a new chosen_one is to be found for the block orf the last 4 hours
- you could also to the verifying by 100 successive online chosen_one's and if 51% confirms a certain hash from the block, then the block's hash is verified
- Something to take care off: or you wont put email_h and passw_h in the blockchain, only the resulting hash and the chosen one calculates the resulting hash from email_h and pass_h when the user is logging in, or the plain text email adress is going to be sent to the chosen one for verifying the resulting hashes. But I want email_h and passw_h in the blockchain to see if someone is using the same email adress with a different password to gain multiple logins to improve his/hers chances for rewards. Another possibility is to hash email twice, and to communicate the first email_h to the chosen one, and to keep the second email_hh in the blockchain, then you don't have to communicate the plain text email adress.
- The chosen_one should give a range of ip_adresses of online users (from the map), zo its peers can ping the next range of peers, and so on, until every online has been pinged. I have no clue yet about the quanities of people to ping at each stage. There's some mathematics involved. Key is the amount of online users. -- What about tor and vpn? A full node and a thin client?
- Please think about sharding! A certain thershold in the network shoul be measured in order to divide the network or a part of the network ... stuff to think about.

### The blueprint:
- check peers / assemble a peers list / update the peers list / kademlia
- | check de blockchain's integrity or download the blockchain if none existent
- if all ok: create/login
- search in all the blocks users (email and password!)
--> first search for a users (!, not block) root hash
--> if no email then create
--> if email existent check the password
- create is assembling in an hour all the new users
- the upper user seen from the root hash of this block of new users is the user who communicates the final hash of the block
( timestamp/timeframe (see below) + root hash block + hash chosen one + previous hash + the hashes from the user's credentials (data = email_hashed and password_hashed)) ==> hash communicated by the chosen one
- 51% of the currently online users should give their ok as assembled by the chosen one, the ok sayers communicate their ok to their list of ip's an expect a returned ok in 51% of their ip's / first layer peers, so a delayed ok towards the chosen one ...
- when ok for chosen one, the block is mined
--> racing between blocks: a certain amount of blocks must be copied from peers, until the hour has passes or the block size exceeds 1MB
- here derived apps can pick in, like for instance a coin or a smart contract system, later more ...

#### What's the genesis hourly timeframe's name? That's '0.0' an hour later then '1.0' ... , if divided in blocks of 1MB it's '34.1' followed by '34.2'

- communicating being online in the network, and leaving the network ... a hash table besides the blockchain to keep track of online presences
- what to do in the beginning when there are low online user presences? should 1 user be able to make the final hash without verifying peers? I think at least 2 online users ...

### What apps/tools to make?
- interface (main user interface for all apps)
- login (need to make a facial recognition library for all platforms, such that the resulting hash stays the same for every platform)
- coin (like bitcoin)
- contract (like ethereum)
- ego (like egoconf)
- link (like linkedin)
- chat (like slack or like twitter)
- lottery (tja)
- gitnip (like github)
- modestland (unprecedentantly trying to live modestly)
- coin exchange (coin vs different other currencies)
- stock exchange (stock valued in coin -- ico)
- file (service to share files)
- survey
- monitor (for crowd and the apps)
- ...

### Amount of peers:
- (4^4)^4 = ~4 billion
- (5^5)^5 = a lot more
So 4 peers is too low en 5 is the right number. That is 5 peers and 5 peers^1 and 5 peers^2, o r every peer communicates with 5 other peers. Coincident peers is normal business?!

### What's next?
See last TODO in the download_blockchain function from verification.cpp
See TODO in create_genesis_block() and then start add_block_to_blockchain() in block.cpp
