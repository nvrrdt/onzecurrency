- restructuring the merkle tree of users by removing all the accounts that weren't used in the last 2 years should be possible
- onzecoin: rewarding system like bitcoin: a few years 1000 coins you get when you're the chosen one and then a halving to 500 coins for the chosen one, so there's also a limited amount of coins
- create bool functions that can be allowed with an if statement like for authentication ans an example at the evp code in block.cpp
- maybe search for asynchronous daytime server asio

1) DONE create hash from user's emailadres and a hash from the password, then a hash from those two hashes , first the email hash and then the concatenated password hash
2) every hour a list with new users needs to be kept, that list will become a block, a merkle tree of the new users
3) // create ip_peers.json with the root hash (1) and with its ip in a dictionary || SOMETHING WRONG
