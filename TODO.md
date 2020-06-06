### What's next?
- update the leveldb with the new user in protocol.cpp when server receives new online user request
- communicate to all peers that there's an new online peer, so they can update their leveldb

- The communication protocol between peers
- Take into account hex values!
- Verification:
  + Download blockchain if no blockchain folder nor blockchain
  + Verify existing blockchain' latest block with peers + update
  + Create + update leveldb
- Continue setting up the p2p system

- What should be in the json value of a leveldb entry? {"version": "1.0", "ip": ip_of_peer, "upnp": "true", "fullnode": "true"}, the key is the peer's hash

- a test for the response_hello function in protocol.cpp (needs a block creation function)
- (in verification.cpp you need to clean up --> use this function in blockchain_update function in protocol.cpp)