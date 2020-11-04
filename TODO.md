### What's next?
- switch from udp to tcp
- connect all layer 0 peers, taking into account the upnp_peers

- setup the hello to the network with the login procedure described in LOGIN.md

- communicate to all peers that there's an new online peer, so they can update their rocksdb
  --> in udp_server.cpp update the message with an request value
  --> in udp_client and udp_server: when hello is received and the rquest is to inform your peers, inform your peers
- follow the update of rocksdb and verify if all data is present as stated here a few lines further

- The communication protocol between peers
- Take into account hex values!
<!-- - Verification:
  + Download blockchain if no blockchain folder nor blockchain
  + Verify existing blockchain' latest block with peers + update
  + Create + update rocksdb -->
- Continue setting up the p2p system

- What should be in the json value of a rocksdb entry? {"version": "1.0", "ip": ip_of_peer, "upnp": "true", "fullnode": "true"}, the key is the peer's hash

- a test for the response_hello function in protocol.cpp (needs a block creation function)
- general cleanup, there's a lot of commented code