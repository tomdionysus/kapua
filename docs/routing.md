# Routing

Kapua is intended to function as a mesh network:

* Every node has a unique, immutable ID (int64).
* Nodes should maintain a small number of direct connections to other nearby nodes (UDP PING every 16s +/- 4s randomised). The cost metric to each node is continuously evaluated.
* Nodes are organised into groups. Each group has a unique, immutable ID (int64). The cost metric to each group is continuously evaluated.
* Each node is a member of at least one and at most 8 groups.
* Periodically (64s +/- 8s randomised) Each node should evaluate all groups it is a member of, and:
	* Join the groups with the lowest cost metric
	* Leave groups with the highest cost metric
* The intent is that over time, nodes self-organise into overlapping groups.
* Groups can be discovered through a number of methods, in order or priority:
	* Local discovery (UDP broadcast) (Highest Priority)
	* Hints from other nodes, using their PING packets
	* Using a well known service (Tracker-like webservice)
	* By creating a new group with a random ID (Lowest Priority)
* When a node evaluates a new group to join, it should:
	* PING all members of the group
	* After a timeout, evaluate the cost metric of the new group
	* Send JOIN to all members if the group metric is suitable.
* When a node evaluates a group to leave, it should:
	* Send LEAVE to all members of the group.
* Groups should have a maximum membership of 32 nodes. When more than 32 nodes are detected in a group, the group should split:
	* A split vote packet should be sent to all members
	* Each member responds to all members with a new, random group ID
	* After a timeout (16s) each node:
		* Evaluates all votes it has recieved, and if its own ID is higher than the median ID of all responding nodes, takes the lowest new group ID it has recieved and joins that group.
* A node that is a member of two groups of less than 8 nodes should send a merge vote to all members of both groups.
* Each node that is a member of a group should maintain a routing table for other groups.
* Nodes should maintain a cache of node IDs to IP addresses for nodes known to be directly contactable.
* A node sending a packet to another node should use a number of methods, in order or priority:
	* If the node has the IP of the destination node, it should send to that node directly (UDP) (Highest Priority)
	* Otherwise, the node should forward the packet to a member of the most likely group
	* If no group is likely, the node should request the IP of the d estination node using a well known service (Tracker-like webservice) (Lowest Priority)
* A node receiving a packet for a different node should forward the packet using the above priority list   

# Node Cost Metric

Initially, the cost metric to a node is the round trip PING in ms (lower is better).

# Group Cost Metric

For any node, the cost metric of a group is the average of the node cost metrics to all members of the group.

# Addressing

A node address is simply its unique ID, however a destination address may be:
	* The node unique ID (looking up an IP/port on another node)
	* The node unique ID plus a group ID (128 bits)
	* A node address of 0x0000000000000000 lus a group ID (128 bits, broadcast to group)

# Routing

## PING Packet

From: ID of the Node
To: ID of Node in the same group
Type: PING
Payload: List of Group IDs the node is a member of
