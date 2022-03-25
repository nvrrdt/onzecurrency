## Tackling the blockchain scaling problem in onzecurrency's proof-of-chosen-ones?

### Preface

### The problem

In centralised systems:
In decentralised systems:

### Solution

The hash of a transaction leads to users whose first characters are the same as the first characters of this hash, these characters define the partitions of users. The transaction is sent to all these users.

When the block_creation_delay is due or if more than 2048 transactions arrived at these partitioned users, then one of these users is chosen (hash_block % hash_user[i] --> lowest remainder wins) to send all the transactions to the coordinator of the chosen_ones for this block. The coordinator sends these transactions to the chosen_ones. The other users send their hash of the block to all the chosen ones (they must be informed of the correct chosen_ones by the coordinator), just to compare. If a threshold of an amount of users is reached (3 for example), then these chosen_ones may inform their part of the network. 

Every partition has another start of the block_creation_delay resulting in a parallel distribution of transactions, which can than be processed thus also in parallel.

If a block is approx. 1 mb with 2048 transactions in and a block_creation_delay of 1 minute while all the users are distributed in 64 partitions, then every user must be able to process a block of 1 mb in 1 minute / 64 partitions = 0.9375 seconds/mb and the total throughput is 2048 x 64 or 131072 transactions per minute or 2184 transactions per second, theoretically.

### Conclusion

This means that the traditional bottleneck in scaling blockchain transactions is solved. Although, on a sidenote, visa is at 1700 transactions per second which makes my calculations look unreal, so I would be glad if more than 1000 transactions per second is attainable.

It's also a sort of sharding I couldn't yet imagine.

In proof of stake this trick isn't applicable as the stake pool participants form the bottleneck, they all send their block proposal to all the users so there's more network congestion.

Also the creation of preliminary blocks (block proposals), as is now implemented in onzecurrency, can be removed. It was meant as ddos protection, but when every second a block is created, then this feature is not needed anymore.

#### PS If you're interested in this project, I can certainly use a set of good hands and minds. Feel free to participate, you're most welcome! Thanks anyway!