/**********
Copyright (c) 2019, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
#include "xcl2.hpp"
#include <vector>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../kernel/common/include/constants.hpp"

#define DATA_SIZE 62500000

//Set IP address of FPGA
#define IP_ADDR 0x0A01D498
#define BOARD_NUMBER 0
#define ARP 0x0A01D498

void wait_for_enter(const std::string &msg) {
    std::cout << msg << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> [<#RxPkt> <local BoardNum> <Dest BoardNum>]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    cl_int err;
    cl::CommandQueue q;
    cl::Context context;

    cl::Kernel user_kernel;
    cl::Kernel network_kernel;

    auto size = DATA_SIZE;
    
    //Allocate Memory in Host Memory
    auto vector_size_bytes = sizeof(int) * size;
    std::vector<int, aligned_allocator<int>> network_ptr0(size);
    std::vector<int, aligned_allocator<int>> network_ptr1(size);


    //OPENCL HOST CODE AREA START
    //Create Program and Kernel
    auto devices = xcl::get_xil_devices();

    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    int valid_device = 0;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context({device}, NULL, NULL, NULL, &err));
        OCL_CHECK(err,
                  q = cl::CommandQueue(
                      context, {device}, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i
                  << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
                  cl::Program program(context, {device}, bins, NULL, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i
                      << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err,
                      network_kernel = cl::Kernel(program, "network_krnl", &err));
            OCL_CHECK(err,
                      user_kernel = cl::Kernel(program, "hls_bil_krnl", &err));
            valid_device++;
            break; // we break because we found a valid device
        }
    }
    if (valid_device == 0) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
    
    uint32_t local_IP = 0x0A01D498;
    uint32_t local_boardNum = 1;
    uint32_t local_Port = 5001; 

    uint32_t connection = 1;
    uint32_t rxPkt = 100;
    uint32_t txPkt = 100;
    // uint32_t rxByteCnt = rxPkt * 4096;
    uint32_t dest_IP = 0x0A01D46E; //alveo0
    uint32_t dest_Port = 5002;
    uint32_t dest_boardNum = 1;

    if(argc >=3)
    {
        rxPkt = strtol(argv[2], NULL, 10) * 3;
        txPkt = rxPkt;
        // rxByteCnt = rxPkt * 4096;
    }
    // rxPkt = BATCH_NUM * BATCH_SIZE;
    // txPkt = BATCH_NUM * BATCH_SIZE;

    // if(argc >=4)
    // {
    //     rxPkt = strtol(argv[3], NULL, 10);
    //     // rxByteCnt = rxPkt * 4096;
    // }

    // if(argc >= 4)
    //     local_Port = strtol(argv[3], NULL, 10);
    int ip_local_array [4];
    ip_local_array[0] = 10;
    ip_local_array[1] = 253;
    ip_local_array[2] = 74;
    ip_local_array[3] = 68;
    
    if (argc >= 4)
    {
        local_boardNum = strtol(argv[3], NULL, 10);
        ip_local_array[3] = 68 + (local_boardNum - 1) * 4;
        local_IP = ip_local_array[3] | (ip_local_array[2] << 8) | (ip_local_array[1] << 16) | (ip_local_array[0] << 24);
        local_Port = 5000 + local_boardNum;
     }

    int ip_dest_array [4];

    if (argc >= 5)
    {
        dest_boardNum = strtol(argv[4], NULL, 10);

//////////////////////////////////////////////////////////////////////
// This part is useless but when removing it, the execution would fail
        std::string s = argv[4];
        std::string delimiter = ".";
        size_t pos = 0;
        std::string token;
        int i = 0;
        
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            ip_dest_array [i] = stoi(token);
            s.erase(0, pos + delimiter.length());
            i++;
        }
////////////////////////////////////////////////////////////////////////
        // ip_dest_array[i] = stoi(s); 
        // 
        ip_dest_array [0] = 10;
        ip_dest_array [1] = 253;
        ip_dest_array [2] = 74;
        ip_dest_array [3] = 68 + (dest_boardNum - 1) * 4;
        dest_IP = ip_dest_array[3] | (ip_dest_array[2] << 8) | (ip_dest_array[1] << 16) | (ip_dest_array[0] << 24);
        dest_Port = 5000 + dest_boardNum;
    }

    printf("\nlocal_IP: %d.%d.%d.%d, %x\n", ip_local_array[0], ip_local_array[1], ip_local_array[2], ip_local_array[3], local_IP);
    printf("local_boardNum: %d, local_port: %d\n", local_boardNum, local_Port);
    printf("dest_IP: %d.%d.%d.%d, %d\n", ip_dest_array[0], ip_dest_array[1], ip_dest_array[2], ip_dest_array[3], dest_IP);
    printf("local_boardNum: %d, dest_port: %d\n", dest_boardNum, dest_Port);
    // printf("rxPkt: %d, packetWord: %d \n", rxPkt, numPacketWord);
    printf("txPkt: %d, packetWord: %d \n", txPkt, 64);
    printf("rxPkt: %d, packetWord: %d \n", rxPkt, 64);

    wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

    // printf("local_IP:%x, local_boardNum:%d\n", local_IP, local_boardNum);

    // Set network kernel arguments
    OCL_CHECK(err, err = network_kernel.setArg(0, local_IP)); // Default IP address
    OCL_CHECK(err, err = network_kernel.setArg(1, local_boardNum)); // Board number
    OCL_CHECK(err, err = network_kernel.setArg(2, local_IP)); // ARP lookup

    OCL_CHECK(err,
              cl::Buffer buffer_r1(context,
                                   CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   vector_size_bytes,
                                   network_ptr0.data(),
                                   &err));
    OCL_CHECK(err,
            cl::Buffer buffer_r2(context,
                                CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                vector_size_bytes,
                                network_ptr1.data(),
                                &err));

    OCL_CHECK(err, err = network_kernel.setArg(3, buffer_r1));
    OCL_CHECK(err, err = network_kernel.setArg(4, buffer_r2));

    printf("enqueue network kernel...\n");
    OCL_CHECK(err, err = q.enqueueTask(network_kernel));
    OCL_CHECK(err, err = q.finish());

    double durationUs = 0.0;

    //Set user Kernel Arguments
    //Set user Kernel Arguments
    OCL_CHECK(err, err = user_kernel.setArg(16, connection));
    OCL_CHECK(err, err = user_kernel.setArg(17, local_Port));
    OCL_CHECK(err, err = user_kernel.setArg(18, rxPkt));
    OCL_CHECK(err, err = user_kernel.setArg(19, txPkt));
    OCL_CHECK(err, err = user_kernel.setArg(20, dest_IP));
    OCL_CHECK(err, err = user_kernel.setArg(21, dest_Port));

    //Launch the Kernel
    auto start = std::chrono::high_resolution_clock::now();
    printf("enqueue user kernel...\n");
    OCL_CHECK(err, err = q.enqueueTask(user_kernel));
    OCL_CHECK(err, err = q.finish());
    auto end = std::chrono::high_resolution_clock::now();
    durationUs = (std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / 1000.0);
    printf("durationUs:%f\n",durationUs);
    //OPENCL HOST CODE AREA END    

    std::cout << "EXIT recorded" << std::endl;
}
