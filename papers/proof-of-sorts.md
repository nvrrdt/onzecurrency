### proof-of-sorts --> posorts

a sortofachain

1) root = (h(h(h(tx1)+tx2))+tx3)+... when starting the software, some txs in a file and so on, root is communicated to verify with peers
2) txs are being sorted, no prev_hash, equal txs are being numbered

- more decentralised as every node doesn't have to communicate everything like blocks, just txs
- every node does the verification/validation, so every node's id must also have a file on disk with contents, no need for a database (unless for ip's of nodes)

attack vectors: everyone looks at itself, so malicious peers won't be communicated with, there is no 51% attack of some sorts
--> there is not really a consensus algorithm during running the software, only when starting the software

rewards: see bootstrap

payment = A -> B: B verifies A, they all verify A

bootstrap: how to grow coins? adding a satoshi (10^-8 coin) to everyone with every tx until >21 million coins is reached

database_sharding: divide users in shards based on their node_id, h(node_id+tx) leads to shard, and send the tx then
network_sharding: do a traceroute and see to which routers you belong, then search for the right nodes