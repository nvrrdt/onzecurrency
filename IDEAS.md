### Some ideas:
- removal of users who didn't use crowd for the last 2 years
- rebasing the blockchains when a certain amount of disk space is reached
- onzecoin: rewarding system like bitcoin: a few years 1000 coins you get when you're the chosen one and then a halving to 500 coins for the chosen one, so there's also a limited amount of coins
- Almost ZK solution: this describes a tx as data for in the block: a & b, userhash a & userhash b, end credit a & end credit b, begin credit a & begin crediet b, a hash from userhash & begin & end credit of a & the same for b & a hash from both hashes to include as data in the block, if a pays a new amount after the last tx, then only his userhash & prev end amount & prev begin amount is to be seen to c (a new b) with the hash from the prev b to verify the root hash from prev hash a & prev hash b; verification should also be done from the roothash in the crowd way of verification; what if the crowd verifier is malicious: then include the malicious one's hash in the root hash to form a new crowd verifier; there's the 2 last transactions that is to be known; the verifier should verify a correct tx and its hashes;;;; maybe it also works with a hash from userhash and end credit of both, while only communicating a tx amount to the verifier but not putting this amount in the blockchain, then there's only one tx shown in the data that the payee gets to see; if a malicious verifier is chosen by payer and payee, then the verifier should be included in the verifier of the block, so extra work for the verifier of the block who should verify every verifier of a tx;
- when logging in with an email adress, people could flood crowd with many emailadresses to improve their chances at winning a reward (yes for the coin there's a reward system), that's why there must be quickly built a login system that makes use of a fingerprint or a facial recognition, maybe we should prevent logging in from a pc or so to prevent that flooding, anyone has a smartphone. For development we could make categories of developers who are written in the blockchain or so. Also peers must be able to see a videostream of you logging in to be able to create and verify the resulting hash, if this is doable.
- The website hook: <H1>THE NEXT BLOCKCHAIN FOR THE CHEAP.</H1><H2>No energy mungering monstrosity, no concentrated stakeholders, best decentralisation of the market. And rewards.</H2>
- you could also to the verifying by 100 successive online chosen_one's and if 51% confirms a certain hash from the block, then the block's hash is verified
- The chosen_one should give a range of ip_adresses of online users (from the map), zo its peers can ping the next range of peers, and so on, until every online has been pinged. I have no clue yet about the quanities of people to ping at each stage. There's some mathematics involved. Key is the amount of online users. -- What about tor and vpn? A full node and a thin client?
- That's the xth element in the map. To show it really is a lottery becoming the chosen_one.
- A mail should be sent to the newly created user in order to verify the emailadres (given at user creation). Don't know yet how to implement this, but you should limit the amount of possible users per person, so also a deleting function should be immplemented, deleting the user after 1 year of not using crowd.
- Do add a version to the various implementations and show this in the user interface.
- Maybe use a mnemonic as password
- if the chosen_one distributes its result, the chosen_one's place in the map of online users is the starting point to communicate to peers, it goes up in the map until a response comes, those peers do the same for the next online users in the map, until 51% of it's respondends confirm and those peers let the chosen_one that 51% confirms and if 51% of total responders confirms the result, the chosen_one remains silent if 51% is reached otherwise that chosen_one launches a new chosen_one by including his/hers hash in the block that didn't work out, and the following DDOS prevention point should also be activated --> it's not precisely 51%, it's a comparison with the probabilities of competing blocks
- preventing DDOS'ing: if there's no new user in the last second of the 2 hour timeframe for creating a block, then it's known who will be the chosen_one, and the ip of the chosen_one is known. To prevent DDOS multiple chosen_one's should be appointed by incorporating the previous chosen_one's hash in the block, which results in another chosen_one, and we keep creating chose_one's until 51% of online users has reacted positively to a certain chosen_one. A delay for appointing a new chosen_one may be included in order to not flood the network
- How to continue development and governance: You have to make it hard for trolls to do harm in de development process, and that's why we'd like to obtain critic developers, both from mainstream and from the margins. And so we'd like to introduce a development reward scheme that resembles a hanging cable, in the middle of the cable you receive nothing while to the edges you'll gain one onze for every time there's a new pull request to the repository, while the edges represent being right about the outcome of the new piece of code and on the other side being always wrong on the outcome. Off course more than 50% of the votes determine the outcome and all developers should vote, unless ..., but the unless is for this voting board of developers to improve this process of development over time.
- First SHARDING proposal. The starting character of your uint32 hash decides if the blockchain needs to be divided or merged. There should be a trigger to divide or merge, there also should be starting and ending blocks when dividing or merging. The trigger to divide is when in less time the maximum block size is reached than the block creation time, the trigger for merging is when 4x times more time is needed to reach maximum block size than block creation time. The maximum block size will be 10 MB according to careful calculations, only full nodes/peers need to be able to download them easily. 10 MB and a block creation time of less than 10 minutes will result in improved scalability compared to eg bitcoin. There's another trigger involved, that's when a certain time is reached, let's say a minute, and the maximum block size limit isn't reached yet. Don't know yet how to do starting and ending blocks though.
  + When dividing the blockchain each partial blockchain starts with a certain range of characters given in hex. Every tx with payer and payee will be hashed, the first character of that hash decides in which partial blockchain the tx will be included. All tx's form a block and sealing of the block is as follows: the block will be hashed including the nonce (the random number creation of all the peers in that partial blockchain leading to a unique nonce) and if the first character of the resulting hash is not in the range for the first character of that partial blockchain then 1 will be added to the nonce, if that doesn't give a correct range then 2 will added and so further. Then the block is sealed, the transactions done and on-chain sharding done. I don't really believe in off-chain sharding.
  + The dividing and merging blocks consists of: ...
- There should be a confirmation when a transaction is waiting in a preliminary block, maybe the peer above the hash from the tx with payer and payee, is able to give an ok or nok.

### 51% voting mechanism
Process to finalize a project:
- 1st voting: 501 voters pro of 1000 voters
- 1 week of improving ideas
- 2nd voting: 511 pro of 1000 (+51% relatively)
- 1 week of improving ideas
- 3rd voting: 517 pro of 1000 (+51% relatively)
- 2 weeks of improving ideas
- 4th voting: 526 pro of 1000 (+51% relatively)
- 4 weeks of improving ideas
- continue until you're able to execute the idea
- the reverse is also possible (back to the drawing boards)

### On a side note:
- futurewise: the os in the blockchain: couldn't installed programs or perhaps an entire operating system be hashed part by part in order to be compared with what peers have installed. The goal is to figure out if someone's system has been tampered with by someone else, a hacker perhaps. Configuration files remain an issue in this setup, how do you compare configuration files. Also software that isn't distributed or very scarce won't be comparable in this setup. The Compared Web - CW. To introduce entropy when hashing a whole computer a by consensus determined nonce-like number can create instantanious hashes of the whole system to be compared by peers, this to obstruct already fixed hashes made by a malicious peer.
- responsability for what's being communicated, like a registered letter, should be trivial in the blockchain world. Only privacy remains an issue.

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

