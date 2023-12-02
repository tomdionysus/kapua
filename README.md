# kapua

A fully decentralised storage, compute, database and networking platform, in C++.

## Project Goals

* A fully decentralised, replicated, soft cluster block-storage designed for use over the internet.
* A distributed key-value store based on the above.
* A distributed file system based on the above.
* A distributed DNS system based on the above.
* A distributed queue based on the above.
* A Virtual Private Cloud networking and security model based on the above.
* A distributed secure compute model based on the above.

## Infrastructure

All infrastructure is contained and implemened in client nodes. Client nodes are connected over IP, and may join or leave the system at any time.

## Storage and Replication

All storage is contained in client nodes. Each client node on signup allocates an amount of disk space (1Gb minimum) which becomes part of the distrubuted block store, and an amount of compute resource, which becomes part of the distributed compute pool. All blocks stored on a node are encrypted with the originator key, and a node may store blocks from any number of originators.