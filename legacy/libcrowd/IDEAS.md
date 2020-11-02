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
- That's the xth element in the map. To show it really is a lottery becoming the chosen_one.
- A mail should be sent to the newly created user in order to verify the emailadres (given at user creation). Don't know yet how to implement this, but you should limit the amount of possible users per person, so also a deleting function shoudl be immplemented, deleting the user after 1 year of not using crowd.
- Do add a version to the various implementations and show this in the user interface.
- Maybe use a mnemonic as password
- When uploading the crowd object to amazon, libboost had to be installe on that server in order to execute the crowd object
- ip_adresses_peers.json should disappear, the ip_adresses of peers should be found in the map of online users
- if the chosen_one distributes its result, the chosen_one's place in the map of online users is the starting point to communicate to peers, it goes up in the map until a response comes, those peers do the same for the next online users in the map, until 51% of it's respondends confirm and those peers let the chosen_one that 51% confirms and if 51% of total responders confirms the result, the chosen_one remains silent if 51% is reached otherwise that chosen_one launches a new chosen_one by including his/hers hash in the block that didn't work out, and the following DDOS prevention point should also be activated
- preventing DDOS'ing: if there's no new user in the last second of the 2 hour timeframe for creating a block, then it's known who will be the chosen_one, and the ip of the chosen_one is known. To prevent DDOS multiple chosen_one's should be appointed by incorporating the previous chosen_one's hash in the block, which results in another chosen_one, and we keep creating chose_one's until 51% of online users has reacted positively to a certain chosen_one. A delay for appointing a new chosen_one may be included in order to not flood the network
- New idea to prevent maliciousness:
  + 10 chosen_ones [H((((data + chosen_one0) + chosen_one1) + chosen_one2) + ... + chosen_one9)] to lower the possibility of a malicious group to take over the blockchain, then >90% of users must be malicious
  + First chosen_one diffuses requests for verification to 9 following chosen_ones, they all must respond to the first chosen_one
  + Then, if all ok, the first chosen_one signs by adding a public key, and broadcasts the data to all online users
  + Then te next block kan be calculated
- How to continue development: You have to make it hard for trolls to do harm in de development process, and that's why we'd like to obtain critic developers, both from mainstream and from the margins. And so we'd like to introduce a development reward scheme that resembles a hanging cable, in the middle of the cable you receive nothing while to the edges you'll gain one onze for every time there's a new pull request to the repository, while the edges represent being right about the outcome of the new piece of code and on the other side being always wrong on the outcome. Off course more than 50% of the votes determine the outcome and all developers should vote, unless ..., but the unless is for this voting board of developers to improve this process of development over time.
- IMPORTANT idea to prevent a DOS attack: the block, including a nonce (!), is hashed and that hash leads to a disciple, which is again hashed (no nonce) with the disciple added leading to another disiple and that 1000 (?) times, the thousandth is the chosen_one and the chosen_one signs the whole block with a public key. The nonce should result in an amount of leading zero's in the first hash leading to a compute time of about 10 seconds on an average computer. It's not a race to find the first nonce because the chosen_one who gets some coins isn't known yet and it isn't possible to direct the chosen_one towards you. The 999 disciples and chosen_one exist to add some entropy in the chain. The hash, representing a user, above the calculated hash in the map is chosen to become disciple (= all nodes) or chosen_one (= fullnode), which also should add some entropy as users enter the map or leave. The 999 disciples and chosen_one will communicate each to ten remaining online users if all is ok with the block, otherwise the disiples and chosen_one will be informed and they verify and commmunicate accordingly. Needs can be calculated: 1000 * 1000 * 1000 = 1000000000 online users. Take care that the peers ^3 who got updated by the peers ^2 who got updated by the disciple ^1 need to know the region in the map of users where the communicating ^2 peer comes from, while the ^2 peer knows that it comes from a ^1 by peeking in the block. ^3 can 't peek in the block to ascertain itself of rightness of the communicating ^2. UPDATE: you don't need a nonce and you don't need to sign a block, the chosen_one and the disciples ust need to communicate with each other about the final hash, and to communicate the result to their own region of users (total_users_region) in the map of users, a total_users_region is a fixed division of total map (total sha256) divided by amount of chosen_one + disciples, and that's the region to which the containing users wil accept a message from. It's pure and simple mathematics! I like the simpleness. And the chosen_one and disciples are protected from DOS by their upnp server for their region (upnp_region). If more than 50% of disciples or the chosen_one doesn't react, then the result will be not sustainable and only then should a nonce ({"nonce": "0"}) should be incorporated in the hash of the block.
- to be able to do a compute by a simple smartphone and prevent peers from easily guessing the final hash so they're unable to point the final hash towards them: 10 verifiers, the final verifier creates the hash, every hash will be starting with a number of zeroes taking 30 seconds to compute on a smartphone done by adding a nonce when computing the hash. If you want the hash to every time (10 times) point to you, you need a lot of computing power. ... If a peer has 10 users (email adresses) then all users can be calculated at once, so this method is NOT TAMPER PROOF.
- next idea: every upnp_peer collects from all its online peers (all the peers before the upnp_peer's hash downwards to the next upnp_peer) a random number, multiplies them and the remainder will communicated to all the other upnp_peers, all remainders will be multiplied, the remainder of that multiplication will be the  nonce used to create the hash of the chosen_one with the block data included. We are effectively multiplying uint32_t. Is this tamper proof? A 51% rule should be obeyed.
- First SHARDING proposal. The starting character of your uint32 hash decides if the blockchain needs to be divided or merged. There should be a trigger to divide or merge, there also should be starting and ending blocks when dividing or merging. The trigger to divide is when in less time the maximum block size is reached than the block creation time, the trigger for merging is when 4x times more time is needed to reach maximum block size than block creation time. The maximum block size will be 10 MB according to careful calculations, only full nodes/peers need to be able to download them easily. 10 MB and a block creation time of less than 10 minutes will result in improved scalability compared to eg bitcoin. There's another trigger involved, that's when a certain time is reached, let's say a minute, and the maximum block size limit isn't reached yet. Don't know yet how to do starting and ending blocks though.
  + When dividing the blockchain each partial blockchain starts with a certain range of characters given in hex. Every tx with payer and payee will be hashed, the first character of that hash decides in which partial blockchain the tx will be included. All tx's form a block and sealing of the block is as follows: the block will be hashed including the nonce (the random number creation of all the peers in that partial blockchain leading to a unique nonce) and if the first character of the resulting hash is not in the range for the first character of that partial blockchain then 1 will be added to the nonce, if that doesn't give a correct range then 2 will added and so further. Then the block is sealed, the transactions done and on-chain sharding done. I don't really believe in off-chain sharding.
  + The dividing and merging blocks consists of: ...
- There should be a confirmation when a transaction is waiting in a preliminary block, maybe the peer above the hash from the tx with payer and payee, is able to give an ok or nok.
- In the case of the random number proposals of everyone for creating a random nonce: proof-of-verifiers should be introduced, the total uint32 spcae divided by total_online_peers^1/3 and they should ok everyone up to the next verifier if and only if all verifiers are ok.

### IOHK proposal
2.1.	Proof-of-chosen-ones:
All the peers in the network have a certain id which is produced by hashing (sha256 e.g.) some data. If there are 1.000.000 peers in a (e.g.) 3 layered network, then the first layer divides the network/peers in 1.000.000^⅓ = 100 equal parts, one of those 100 parts will also be divided in 100 parts in the second layer and the third layer divides again one part of layer 2 in 100 parts/peers. 
For now let’s consider the first layer and start at the beginning, for this we need a starting point for choosing the 100 peers of layer 1 (the chosen ones!) and we do that by taking the last previous_hash (for inclusion into the next block) as the starting point. And so we’re able to divide the sha256 space (=uint32) into 100 (approx.) equal parts. The peer closest to this 1/100th is seen as a layer 1 peer.
If all these peers create a pseudo random number and communicate them to each other, add all those numbers up (still uint32), then the result is a (I think true) random number. If this resulting number is then being verified by all the layer 1 peers, and if ok for all peers then this resulting number can be seen as a nonce for creating/sealing the next block.
The next step is to communicate this nonce from layer 1 to layer 2 and from layer 2 to layer 3.

If an issue should arise with ddos’ing the layer 1 peers, the first hash from the first starting point could be rehashed to create a new starting point (and so on if necessary).
In the ideal world all layer 1 peers should communicate an ok, maybe a threshold percentage of ok’s could suffice.
If in layer 1 there’s only 1 random number present (of 100 in this case, so 99 malicious peers) then a random number will be produced.
The only computation that is needed is producing a random number, which is not so difficult or intensive.

There is no mining and thus no mining fees.

Validation of this proposal can initially only be done in a testing environment where as many peers as  possible are created that are able to create blocks enabling measurement of maximum throughput. Although I don’t have an idea what’s realistically possible when setting up a test environment.

2.2.	Sharding and local sharding:
All the peers’ id’s consist of a hash of some data. Based on the first character of the hash you’re able to divide the peers into buckets which could present a partial blockchain.
In case of a financial transaction and block creation the following procedure is in place: the transaction will be hashed and the first character of that hash will decide the partial blockchain where this transaction will be included. Many transactions form a block, but it’s not sure that the first character of the hash of the block will be in the same blockchain as the transaction, which is why a simple structured nonce is introduced to create a correspondence between the transaction and the block (equal first character).
This correspondence enables the usage of multiple partial blockchains.
Every partial blockchain should also be able to shrink or expand based on the needs of the users. To enable this it might be needed to create extra blocks to announce the beginning and/or the end of an expanding/shrinking process.

Local sharding is what I like to introduce. The idea is that the network latency will become much better if you group partial blockchains geographically, right now network communication is performed globally. May be of less importance but local sharding has also improved energy efficiency.
Also most transactions happen in the same (overlapping) geographical region in my unmeasured opinion.

For validating this proposal a test environment should be set up. The goal is to test the shrinkability and expandability of the partial blockchains and to verify the correctness of all resulting hashes.

2.3.	Almost zero knowledge:
When sealing a block all transactions are verified by the chosen ones and to enable almost zero knowledge the (non hashed) data of every last transaction should then also be redundantly distributed over a few peers.

For every new transaction the first chosen one looks up at least two (for comparison) previous transactions (non hashed) from the distributed and redundant previous transactions and so she/he can verify the ability to e.g. fulfil a payment correctly.
A chain of next chosen ones should also exist to verify the (hashed) transactions and to prevent malicious peers from interfering with the blockchain, but that’s another story.

The last two transactions are there to be compared, older transactions should be deleted.

In a network of 1.000.000 peers there aren’t many peers who are knowledgeable about your latest transaction, just the first chosen one and the redundancy peers.

Also a testing environment should set up where the demanded functionality is tested.

### On a side note:
- futurewise: the os in the blockchain: couldn't installed programs or perhaps an entire operating system be hashed part by part in order to be compared with what peers have installed. The goal is to figure out if someone's system has been tampered with by someone else, a hacker perhaps. Cofiguration files remain an issue in this setup, how do you compare configuration files. Also software that isn't distributed or very scarce won't be comparable in this setup. The Compared Web - CW. To introduce entropy when hashing a whole computer a by consensus determined nonce-like number can create instantanious hashes of the whole system to be compared by peers, this to obstruct already fixed hashes made by a malicious peer.
- responsability for what's being communicated, like a registered letter, should be trivial in the blockchain world. Only privacy remains an issue.

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
- fairness (an open and fair prices policy to introduce to companies)
- dns
- monitor (for crowd and the apps)
- hair (an ipfs or swarm alternative)
- ...

### Amount of peers:
- (4^4)^4 = ~4 billion
- (5^5)^5 = a lot more
So 4 peers is too low en 5 is the right number. That is 5 peers and 5 peers^1 and 5 peers^2, o r every peer communicates with 5 other peers. Coincident peers is normal business?!