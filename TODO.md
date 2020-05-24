### What's next?
- a test for the response_hello function in protocol
- in verification.cpp you need to clean up

- update the leveldb with the new user in protocol.cpp when server receives new online user request
- communicate to all peers that there's an new online peer, so they can update their leveldb

- The communication protocol between peers
- Take into account hex values!
- Verification:
  + Download blockchain if no blockchain folder nor blockchain
  + Verify existing blockchain' latest block with peers + update
  + Create + update leveldb
- Continue setting up the p2p system
