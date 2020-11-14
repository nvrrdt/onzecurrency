### Explanation of how's the network protocol done:

#### Howto first steps:

update blockchain && update rocksdb from blockchain and being online of a peer
- blockchain only when new peer or salt changes
- salt is previous hash of block, a new block every 10 minutes
- blockchain contains: create timestamp, previous hash, counter value, all new peers H(Hemail+HH(password+salt)) && Hemail && salt, block_number, ...
- rocksdb contains: H(Hemail+H(H(password+salt)+salt)+salt) && Hemail && salt from blockchain, version, ip adress, full node, upnp, online since timestamp (om te weten wat de volgorde van de peers is in de layer manager)

#### A new peer introduction in blockchain and rocksdb

email, H(H(password+salt)+salt) and salt --> findupnppeer H(Hemail+H(H(password+salt)+salt)+salt) --> if new upnp exchange peers --> old peer gives ok and connects to layer 0  
