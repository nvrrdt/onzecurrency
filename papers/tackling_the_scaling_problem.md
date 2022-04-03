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

The hash space (sha256 or from 0 to (2^256)-1) is divided in a dynamic binary amount of shards fluctuating from 1 shard to 128 shards.

When communicating transactions or blocks the first characters of the hash of a transaction or block leads to users (fair_user_id) with the same first characters, these characters define the shards and an amount of users (minimum 5) is present in each shard. So when minimum 5+5 users are reached in every shard then the shards are growing to the next binary number until a maximum of 128 shards (may be higher?) is reached, schrinking is also possible. Calculating the shards is a database operation!

The user_id is a hash coming from a face recognition and/or fingerprint recognition constant. These user_id's aren't fairly distributed over the 2^256 space, so what needs to be introduced is a fair_user_id, which is calculated by fair_user_id = (2^256 * order_nr_user_id / total_user_id's), every shard has so approximately an equal part of users and this results in a fairer distribution of rewards.

The hash of a transaction leads to a shard and all the shard's users should be informed of this transaction. (Might be sometime that a fairer transaction distribution algorithm is needed when there are too many users in a shard.) Only the shard's users are aware of this transaction and not all the users!  
Tx % fair_user_id = smallest fair_user_id confirms payer and payee of transaction being processed.

For blocks, when the block_creation_delay (of 1 minute) is due, then the first of maximum 128 (hash_block % fair_user_id_in_shard[i] --> 128 with lowest remainder) of the fair_users in a shard (chosen-ones) inform their part of the network, based on the order_nr of each fair_user_id in the shard starting from the first characters of the hash of the block.  
Every 60 seconds / total_shards a shard creates a block based on the order of the shards in de 2^256 space. This results in parallel processing of blocks and should improve the throughput tremendously.  
The chosen-ones must first communicate to establish the maximum amount of included transactions in the block.

If a block is approximatelly 1 mb with 2048 transactions in and a block_creation_delay of 1 minute while all the users are distributed in 128 shards, then must proof-of-chosen-ones be able to process a block of 1 mb in 1 minute / 128 partitions = 0.46875 seconds/mb and the total throughput is 2048 x 128 or 262144 transactions per minute or 4369 transactions per second, theoretically. It is every 0.46875 second a block for the whole system, while every shard has 1 minute to receive transactions, to create a block, and to inform the whole network of a block. So it might be that 1 minute is not enough if there are a lot of users (100^3 or so), informing the whole network is the bottleneck. There is no limit on block size, it eventually might be one day.

As the prev_hash in a block is not known at block creation, informing the network of a block takes more time then creating the blocks in parallel, a minute later the prev_hash is communicated with the next block and the block can be sealed then.

When a user goes online, this user must be added to the database at the start of the block_creation_delay, otherwise this user might upset the order of the chosen-ones.

There's also a risk of users becoming headless, that is when a block or prev_hash communication didn't happen. Pinging another user is the solution, who can then communicate a correct block/prev_hash.

Make blockchain size smaller by distributing the transactions into the correct shard, h_txs that leads to a certain shard (first characters of h_txs) should save the transactions, other users should save a hash.

### Conclusion

This means that the traditional bottleneck in scaling blockchain transactions might be solved. Although, on a sidenote, visa has 1700 transactions per second which makes my calculations look unreal, so I would be glad if more than 500 transactions per second is attainable.

In proof of stake this trick isn't applicable as the stake pool participants form the bottleneck, they all send their block proposal to all the users so there's more network congestion.

Also the creation of preliminary blocks (block proposals), as is now implemented in onzecurrency, can be removed. It was meant as ddos protection, but when every second a block is created, then this feature is not needed anymore.

#### PS If you're interested in this project, I can certainly use a set of good hands and minds. Feel free to participate, you're most welcome! Thanks anyway!