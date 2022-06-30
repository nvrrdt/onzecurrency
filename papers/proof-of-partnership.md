#### proof-of-partnership (pop)

DRAFT:

(1)
- txs come in at every node, the poco way, a tree of hundred subnodes per node
- the hash of a block leads to a certain node's id
- every node is being communicated the poco way by that node as coordinator
- everyone validates, so that's the partnership
- every tx must be accompanied with an id not present in block_time's other tx id's, this for easy verification
- also six confirmation passes necessary, longest wins, most txs wins too

- 51% sybil attack --> how to prevent sybil attacks? proof-of-work or limiting the possibility of sybil attack by proof-of-stake

something that's fastly verifiable and difficultly provable

a blockchain without attacks isn't possible

proof of work is maniacal, you can't mess with that


(2)
but everyone can assemble a block, but what do you need to do for verification
the node(s)/coordinator(s) may send only the previous hash(es), that would be ideal
and then the attack --> attacker can inform everyone, it isn't accepted by everyone, but the majority
                    --> attacker can stop the workings of the blockchain
                    --> identification attackers: difficult
                    --> preventio: control ip adresses for duplicates, ...


would splitting up the blockchan in a malicious one and a benign one solve this --> blacklisting