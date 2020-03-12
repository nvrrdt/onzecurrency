- restructuring the merkle tree of users by removing all the accounts that weren't used in the last 2 years should be possible
- removal of users who don't use crowd for 2 years
- onzecoin: rewarding system like bitcoin: a few years 1000 coins you get when you're the chosen one and then a halving to 500 coins for the chosen one, so there's also a limited amount of coins
- create bool functions that can be allowed with an if statement like for authentication ans an example at the evp code in block.cpp
- maybe search for asynchronous daytime server asio
- TODO: block malicious users who in more than 51% of following verifications of the block's hash don't approve the block's hash in the case there's at least a 10-33% (amount of users decides, also amount of shards) users approving of the hash - please do not choke the system
- ZK solution: this describes a tx as data for in the block: a & b, userhash a & userhash b, end credit a & end credit b, begin credit a & begin crediet b, a hash from userhash & begin & end credit of a & the same for b & a hash from both hashes to include as data in the block, if a pays a new amount after the last tx, then only his userhash & prev end amount & prev begin amount is to be seen to c (a new b) with the hash from the prev b to verify the root hash from prev hash a & prev hash b; verification should also be done from the roothash in the crowd way of verification; what if the crowd verifier is malicious: then include the malicious one's hash in the root hash to form a new crowd verifier; there's the 2 last transactions that is to be known; the verifier should verify a correct tx and its hashes;;;; maybe it also works with a hash from userhash and end credit of both, while only communicating a tx amount to the verifier but not putting this amount in the blockchain, then there's only one tx shown in the data that the payee gets to see; if a malicious verifier is chosen by payer and payee, then the verifier should be included in the verifier of the block, so extra work for the verifier of the block who should verify every verifier of a tx;

1) DONE create hash from user's emailadres and a hash from the password, then a hash from those two hashes , first the email hash and then the concatenated password hash
2) every hour a list with new users needs to be kept, that list will become a block, a merkle tree of the new users
3) // create ip_peers.json with the root hash (1) and with its ip in a dictionary || SOMETHING WRONG

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

### What's next?
See the end of create_block in block.cpp, about the hash_table