### How login works:

- Input login and password
- A random salt must be used when verifying the password
- Saying hello to network:
  - communicate login and H(H(password+salt)) to chosen_one
  - chosen_one verifies and communicates back to requester (when ok of every layer 0 peer) H(login) and H(H(login)+H(H(password+salt)))
  - if ok: chosen_one communicates login and H(H(login)+H(H(password+salt))) to layer 0, layer 0 to layer 1, layer 1 to layer 2  
  --> so only chosen_one is able to see H(password+salt) and this H should never be communicated
  - in rocksdb of every peer store H(login) en H(H(login)+H(password+salt))
  - chosen one is H(hash_requesting_peer + salt)
  - use a counter/nonce if there's no consensus saying hello

  - salt must change with every tx, so update every peer in rocksdb
  --> chosen one is H(hash_requesting_peer + new salt)
  --> communicate login and H(password+salt) to chosen_one for logging in !!! single H !!!
  --> communicate login and H(H(password+new salt)) to chosen_one for changing password
  --> chosen_one verifies and communicates back to requester (when ok of every layer 0 peer) H(login) and H(H(login)+H(H(password+salt)))
  --> if ok: chosen_one communicates login and H(H(login)+H(H(password+new salt))) to layer 0, layer 0 to layer 1, layer 1 to layer 2
  - login must be email adress == rules

- Creating new user works the same as logging in but the H(login) should be

- !!! when doing transactions tx: login and H(password+salt) should be provided --> immediately change the salt and communicate H(H(password+salt)) !!!