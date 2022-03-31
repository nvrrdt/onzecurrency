## Tackling the blockchain scaling problem in onzecurrency's proof-of-chosen-ones?

### Preface

Proof-of-chosen-ones works as follows:

The user_id is a hash of a constant coming from the login system, this system creates this constant based on face recognition and/or fingerprint recognition. The user_id is thus a 256 bit number (sha256).

There are a hundred chosen-ones which are being seen as validators. The chosen-ones divide the network of users into hundred (mostly almost) equal parts, when a message must be spread throughout the network they act as gatekeepers who do some validation of the message and then decide to distribute the message. Not all chosen-ones will distribute a message, so there's a probability playing.

The block_creation_delay is 1 minute, so every minute a block is being created only if there's a new user.  
There's a ddos prevention algorithm, so after every block_creation_delay more (preliminary) blocks are being created than strictly necessary because the hash of a block (also sha256) points to a different user's ip address, so more user's are getting a chance to become (final) chosen_one (and being rewarded).

To go from preliminary blocks to the final block, there's a sifting algorithm (creating preliminary blocks could be seen as mining) and the first received preliminary block whose validated correctly in the blockchain has a higher chance of becoming the final block. The final block is decided when there's only remaining one preliminary block after sifting.

The hash of a block is a number and the number of a user_id that follows on this hash of the block is the coordinator of the chosen-ones. Then based on this user_id of the coordinator the other chosen-ones can be calculated (approximately every 1/100th of all the users).

### The scaling problem

In centralized systems: the central server has a maximum throughput of transactions per second and can so serve comfortably a maximum amount of clients, meaning that for financial transactions they can only handle a maximum amount of transactions. As an example, Visa has practically maximum 1700 transactions per second.

In decentralized systems: Bitcoin with proof-of-work has 1 mb of transactions per approximately 10 minutes, or 4.6 transactions per second, and which is a compromise between throughput and network latency (competing miners must be updated) and confirmation time (till final block). In proof-of-stake the stake pool participants distribute their candidate block to all the users, the fastest blockchain wins and rewards are given to the stake pool participant who won. Here there's a lot of network activity. 500-1000 transactions per second is doable. In PBFT there's even much more network activity (all the users validate every block and communicate their result) which limits the size of the network tremendously.

In proof-of-chosen-ones v1: every minute a block is created which contains a maximum of 2048 transactions which gives theoretically 34 transactions per second.

### Solution (proof-of-chosen-ones v2)

**** Work in progress ****

The hash of a transaction leads to users whose first characters are the same as the first characters of this hash, these characters define the partitions of users. The transaction is sent to all these users.

When the block_creation_delay is due or if more than 2048 transactions arrived at these partitioned users, then one of these users is chosen (hash_block % hash_user[i] --> lowest remainder wins) to send all the transactions to the coordinator of the chosen_ones for this block. The coordinator sends these transactions to the chosen_ones. The other users send their hash of the block to all the chosen ones (they must be informed of the correct chosen_ones by the coordinator), just to compare. If a threshold of an amount of users is reached (3 for example), then these chosen_ones may inform their part of the network. 

Every partition has another start of the block_creation_delay resulting in a parallel distribution of transactions, which can than be processed thus also in parallel.

If a block is approx. 1 mb with 2048 transactions in and a block_creation_delay of 1 minute while all the users are distributed in 64 partitions, then every user must be able to process a block of 1 mb in 1 minute / 64 partitions = 0.9375 seconds/mb and the total throughput is 2048 x 64 or 131072 transactions per minute or 2184 transactions per second, theoretically.

There's also something fishy, block_creation delay is a minute, every second a new block, while sending the block to all the users takes 10 seconds at least
and the prev_hash is based on the previous block, block creation maybe a minute later and compared in the network? and thje transactions may also be delayed in the first step?

****
fair distribution of chosen_ones algorithm --> every shard must have approximately the same amount of users

tx % user_id = smallest for confirmation towards payer and payee

maximum 128 shards, maybe even more (?)

128 smallest user_id % h_block (= chosen_ones) must include prev_hash (rewards must also be included --> risk of becoming headless , what if new user goes online at same time, delayed going online?), wait so, then inform whole network based on h_block

time_synchronization, ha time is time he, block_creation_delay of 64 seconds, every shard fires every 64 / 128 second, they must wait for previous hash (congestion possible? but 0.5 seconds is a cautious number)

headless prevention algorithm

Make blockchain size smaller by distributing the transactions into shards, h_txs that leads to a certain shard (first characters of h_txs) should save the transactions, other users should save a hash
****

### Conclusion

**** Work in progress ****

This means that the traditional bottleneck in scaling blockchain transactions is solved. Although, on a sidenote, visa is at 1700 transactions per second which makes my calculations look unreal, so I would be glad if more than 1000 transactions per second is attainable.

It's also a sort of sharding I couldn't yet imagine.

In proof of stake this trick isn't applicable as the stake pool participants form the bottleneck, they all send their block proposal to all the users so there's more network congestion.

Also the creation of preliminary blocks (block proposals), as is now implemented in onzecurrency, can be removed. It was meant as ddos protection, but when every second a block is created, then this feature is not needed anymore.

#### PS If you're interested in this project, I can certainly use a set of good hands and minds. Feel free to participate, you're most welcome! Thanks anyway!