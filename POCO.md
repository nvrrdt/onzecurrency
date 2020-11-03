### How poco works:

peers:  
1               8                     --> layer 0  
|    \           |      \  
2      5       9         12        --> layer 1  
|  \    |  \     |   \        |   \  
3  4  6  7  10  11  13  14  --> layer 2  
layer 0 creates random numbers and communicates them to all layer 0  
if consensus for nonce = add all random numbers / uint32  
then layer 0 communicates the nonce0 to layer 1 and layer 1 to layer 2  
if not consensus, communicate until culprit is found and punish culprit or nonce1 + 1 and find new layer 0 peers  
when peer layer 2 receives nonce0, it needs to verify in layer 1 next peer and layer 1 nexttonext peer, the same for layer 1 peers but verify with  in layer 0  
add layer if layer 0 become > 128 == 128 tcp connections for a server  
in a network of 5 peers: s1 == server 1 to cl2 == client 2 and to cl 3 and to cl4 and cl5, s2 to cl3 and to cl4 and to cl5, s3 to cl4 and to cl5, s4 to cl5 --> draw a circle with 5 dots

### How login works:

f=login, h(f)  
g=pass, h(g)  
h(h(f)+h(g)) && h(f) == peer_id  
h(g) && f-->1peer-->h(f)-->rest van de peers == proof  
f == enkele rules == email  
h(g) moet met verschillende passes gemaakt worden, op te zoeken, mss f conc g

### Number of peers per layer:

peers layer 0 = peers^(1/3)  
peers layer 1 = peers^(1/2)  
peers layer 2 =(peers – peers^(1/2) – peers^(1/3)) / peers^(1/2)  

