# Water_Content_Network_beta
This repository contains the code files used to program a network that send data to thingSpeak platform.
The network is composed by two nodes that broadcast water content and temperature measurements to each other.
The data is then sent by node 1 to another ESP board configured as a bridge through serial communication,
responsible to upload the data to a ThingSpeak channel.
Then you will find three files:
1) MeshBridge_Node1_v4: Configures the data transmission from the mesh through serial communication and then upload it to the cloud
2) MeshExample_Node1_v5: Configures the data transmission inbetween the mesh nodes and send all the data to the bridge by serial communication
3) MeshExample_Node2_v5: Configures the data transmission inbetween the mesh nodes
