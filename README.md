# RecEasyNet

In this project, a large recommendation model (three-layer forward neural network) is deployed in FPGA clusters. Communication among nodes is realized via Send()/Recv() functions from EasyNet.

Five bitstreams are generated:
1. **node_1_mem_first**: as the start node, do memory reading and computation in layer 1 
2. **node_1_compute**: do computation only in layer 1
3. **node_1_mem**: do memory reading and computation in layer 1
4. **node_2**: do computation in layer 2
5. **node_3**: do computation in layer 3 and output layer

The basic communication pattern is in a ring method like: node_1_mem_first -> node_1_compute -> node_1_mem -> node_1_compute ... -> node_1_mem -> node_1_compute -> node_2 -> node_3.


