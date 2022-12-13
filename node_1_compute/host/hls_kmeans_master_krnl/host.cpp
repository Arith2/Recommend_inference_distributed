#include <vector>
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
	

#include "util/allocation.h"
#include "util/dataIo.h"
#include "util/timer.h"
#include "util/arguments.h"
#include "mckm/mckm.h"

// #include "cmdlineparser.h"
#include "xcl2.hpp"

#include <chrono>
#include <stdlib.h>

#define DATA_SIZE 62500000

//Set IP address of FPGA
#define IP_ADDR 0x0A01D498
#define BOARD_NUMBER 0
#define ARP 0x0A01D498

#define DIMENSION_MAX 256
#define CENTERS_MAX  16
#define FP_MAX 256

void wait_for_enter(const std::string &msg) {
    std::cout << msg << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

typedef unsigned int uint;
#define N_CU_SEPARATE 1
#define N_CU_PER_GMEM 1
#define MAX_HBM_BANKCOUNT 32
#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
const int bank[MAX_HBM_BANKCOUNT] = {
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3),  BANK_NAME(4),
    BANK_NAME(5),  BANK_NAME(6),  BANK_NAME(7),  BANK_NAME(8),  BANK_NAME(9),
    BANK_NAME(10), BANK_NAME(11), BANK_NAME(12), BANK_NAME(13), BANK_NAME(14),
    BANK_NAME(15), BANK_NAME(16), BANK_NAME(17), BANK_NAME(18), BANK_NAME(19),
    BANK_NAME(20), BANK_NAME(21), BANK_NAME(22), BANK_NAME(23), BANK_NAME(24),
    BANK_NAME(25), BANK_NAME(26), BANK_NAME(27), BANK_NAME(28), BANK_NAME(29),
    BANK_NAME(30), BANK_NAME(31)};

float calculate_scale_ratio(int n, int d);
void scale_float2int_minibatch(std::vector<uint, aligned_allocator<uint>> *minibatch_gmem_points[], // 对象指针数组
                    float *dmatrix, float scale_ratio, int n_cu_seperate,
                    int batch_size, int threads, int batch_num, int d);
void scale_int2float_means(float *means, 
                    std::vector<uint, aligned_allocator<uint>> &means_gmem, // 对象引用
                    float scale_ratio, int k, int d);
void scale_float2int_means(std::vector<uint, aligned_allocator<uint>> &means_gmem, 
                        float *means, float scale_ratio, int k, int d);

// using namespace sda::utils;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> [<nclusters> <ndimensions> <npoints>][<ListenPort> <destIP> <destPort> <localIP> <boardNum> <connection> <rxbytecnt>] " << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    float *means = NULL;
    float *dmatrix = NULL;
    float *input = NULL;

    bool isBinary = false;

    int threads = 1;
    size_t K=40, D=20;
    size_t N = 150;
    uint32_t iteration = 1;
    int batch_size = 100;
    int batch_num = 0;
    CUtilTimer timer;
    float scale_ratio = 100;

    uint32_t connection = 4;
    uint32_t listenPort = 5001; 
    uint32_t rxByteCnt = 320000;
    uint32_t destIP = 0x0A01D46E; //alveo0
    uint32_t destPort = 5002;
    uint32_t localIP = 0x0A01D498;
    uint32_t boardNum = 1;
    
//    #ifndef __AVX__
  //      fprintf (stderr, "Unfortunately your CPU does not support AVX intrinsics. \nExit.\n");
    //    exit(1);
    //#endif
    

    
    // CmdLineParser parser;
    // // parsing_args(argc - 1, argv + 1, &N, &K, &D, &threads, filename, isBinary, &iteration, &repetition, &batch_size, &thre);
    // parser.addSwitch("--xclbin_file", "-x", "input binary file string", "");
    // parser.addSwitch("--input_file", "-f", "input test data flie", "");
    // parser.addSwitch("--nclusters", "-k", "number of clusters", "");
    // parser.addSwitch("--ndimensions", "-d", "number of dimensions", "");
    // parser.addSwitch("--npoints", "-n", "number of points", "");
    // parser.addSwitch("--iteration", "-i", "iteration", "");
    // parser.addSwitch("--samples", "-s", "number of samples", "");
    // parser.addSwitch("--threads", "-t", "threads", "");
    // parser.addSwitch("--scale", "-scl", "scale num", "");

    // parser.addSwitch("--listenPort", "-lp", "listenPort", "");
    // parser.addSwitch("--destIP", "-dip", "destIP", "");
    // parser.addSwitch("--destPort", "-dp", "destPort", "");
    // parser.addSwitch("--localIP", "-lip", "localIP", "");
    // parser.addSwitch("--boardNum", "-b", "boardNum", "");
    // parser.addSwitch("--connection", "-c", "connection", "");

    // parser.parse(argc, argv);

    // // Read settings
    // std::string binaryFile = parser.value("xclbin_file");
    // std::string filename = parser.value("input_file");
    // N = parser.value_to_int("npoints");
    // K = parser.value_to_int("nclusters");
    // D = parser.value_to_int("ndimensions");
    // iteration = parser.value_to_int("iteration");
    // batch_size = parser.value_to_int("samples");
    // threads = parser.value_to_int("threads");
    // scale_ratio = (float)(parser.value_to_int("scale"));

    // listenPort = parser.value_to_int("listenPort");
    // destIP = parser.value_to_int("destIP");
    // destPort = parser.value_to_int("destPort");
    // localIP = parser.value_to_int("localIP");
    // boardNum = parser.value_to_int("boardNum");

    // [<nclusters> <ndimensions> <npoints>][<ListenPort> <destIP> <destPort> <localIP> <boardNum>]
    if (argc >= 3)
    {
        K = strtol(argv[2], NULL, 10);
    }

    if (argc >= 4)
    {
        D = strtol(argv[3], NULL, 10);
    }

    if (argc >= 5)
    {
        N = strtol(argv[4], NULL, 10);
    }
    batch_size = N;

    if(argc >= 6)
        listenPort = strtol(argv[5], NULL, 10);

    if (argc >= 7)
    {
        std::string s = argv[6];
        std::string delimiter = ".";
        int ip [4];
        size_t pos = 0;
        std::string token;
        int i = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            ip [i] = stoi(token);
            s.erase(0, pos + delimiter.length());
            i++;
        }
        ip[i] = stoi(s); 
        destIP = ip[3] | (ip[2] << 8) | (ip[1] << 16) | (ip[0] << 24);
    }

    if (argc >= 8)
    {
        destPort = strtol(argv[7], NULL, 10);
    }

    if (argc >= 9)
    {
        std::string s = argv[8];
        std::string delimiter = ".";
        int ip [4];
        size_t pos = 0;
        std::string token;
        int i = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            ip [i] = stoi(token);
            s.erase(0, pos + delimiter.length());
            i++;
        }
        ip[i] = stoi(s); 
        localIP = ip[3] | (ip[2] << 8) | (ip[1] << 16) | (ip[0] << 24);
    }

    if (argc >= 10)
    {
        boardNum = strtol(argv[9], NULL, 10);
    }

    if (argc >= 11)
    {
        connection = strtol(argv[10], NULL, 10);
    }


    rxByteCnt = ((DIMENSION_MAX * CENTERS_MAX * 4 + CENTERS_MAX * 4 + 1023) / 1024) * 1024 ; //set to multiple of 1KB

    if(argc >= 12)
    {
	rxByteCnt= strtol(argv[11], NULL, 10);
    }
    const int n = N;           
    const int d = D ;  
    const int k = K ;          
    batch_size = (batch_size + threads - 1) / threads * threads; 
    batch_num = n / batch_size;

    printf("thread:%d; iteration: %d ;data size: %d; dimension: %d; number of cluster: %d; batch_size:%d\n", threads, iteration, n, d, k, batch_size);    
    printf("localIP:%x, boardNum:%d\n", localIP, boardNum);
    printf("rxByteCnt:%d, listen Port:%d, destIP:%x, destPort:%d, connection:%d\n", rxByteCnt, listenPort, destIP, destPort, connection);

    dmatrix = (float*) ddr_alloc(n * d * sizeof (float)); 
    input = (float*) ddr_alloc(n * d * sizeof (float));   

    memset(input, 0 , n * d * sizeof (float));            

    // read_file(input, n, d, filename.c_str(), isBinary);      
    normalization(d, n, input);                           
    shuffle_object (input, d, n, dmatrix);                
    means = (float*) ddr_alloc(k * d * sizeof (float));  // means = clusters
    initial_centroids(k, d, n, means, input);
    
 /////////////////////////////////////device_begin_1/////////////////////////////////////

    cl_int err;
    cl::CommandQueue q;
    cl::Context context;

    cl::Kernel user_kernel;
    cl::Kernel network_kernel;

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
                      context, {device}, CL_QUEUE_PROFILING_ENABLE |
                                        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));

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
                      user_kernel = cl::Kernel(program, "hls_kmeans_master_krnl", &err));
            valid_device++;
            break; // we break because we found a valid device
        }
    }
    if (valid_device == 0) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
    
    wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

/////////////////////////////////////////////////////////////////////////////////////////////

    auto size = DATA_SIZE;
    
    //Allocate Memory in Host Memory
    auto vector_size_bytes = sizeof(int) * size;
    std::vector<int, aligned_allocator<int>> network_ptr0(size);
    std::vector<int, aligned_allocator<int>> network_ptr1(size);
    
    // Set network kernel arguments
    OCL_CHECK(err, err = network_kernel.setArg(0, localIP)); // Default IP address
    OCL_CHECK(err, err = network_kernel.setArg(1, boardNum)); // Board number
    OCL_CHECK(err, err = network_kernel.setArg(2, localIP)); // ARP lookup

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


 /////////////////////////////////////device_end_1///////////////////////////////////////
 /////////////////////////////////////device_begin_2/////////////////////////////////////
    int minibatch_gmem_size = batch_size / threads * d * batch_num; 
    std::vector<uint, aligned_allocator<uint>> *minibatch_gmem_points[N_CU_SEPARATE];
    for (int i = 0; i < N_CU_SEPARATE; i++){
        minibatch_gmem_points[i] = new std::vector<uint, aligned_allocator<uint>>(minibatch_gmem_size);
    }
    int means_gmem_size = k * d;
    std::vector<uint, aligned_allocator<uint>> means_gmem(means_gmem_size);
    scale_float2int_minibatch(minibatch_gmem_points, dmatrix, scale_ratio, N_CU_SEPARATE, batch_size, threads, batch_num, d);
    scale_float2int_means(means_gmem, means, scale_ratio, k, d);

    std::vector<cl_mem_ext_ptr_t> minibatch_gmem_Ext(N_CU_SEPARATE);
    cl_mem_ext_ptr_t merge_gmem_Ext;
    std::vector<cl::Buffer> minibatch_gmem_buf(N_CU_SEPARATE);
    cl::Buffer merge_gmem_buf;
    for (int i = 0; i < N_CU_SEPARATE; i++) {
        minibatch_gmem_Ext[i].obj = (*minibatch_gmem_points[i]).data(); // 
        minibatch_gmem_Ext[i].param = 0;
        //minibatch_gmem_Ext[i].flags = bank[i / N_CU_PER_GMEM];
        minibatch_gmem_Ext[i].flags = bank[i];
        //minibatch_gmem_Ext[i].flags = bank[0];
    }

    merge_gmem_Ext.obj = means_gmem.data();
    merge_gmem_Ext.param = 0;
    merge_gmem_Ext.flags = bank[0];
    //merge_gmem_Ext.flags = bank[N_CU_SEPARATE];


    for (int i = 0; i < N_CU_SEPARATE; i++) {
        OCL_CHECK(err, minibatch_gmem_buf[i] = cl::Buffer(
                       context, CL_MEM_READ_ONLY | CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR,
                       sizeof(int) * minibatch_gmem_size, &minibatch_gmem_Ext[i], &err));
    }    
    OCL_CHECK(err, merge_gmem_buf = cl::Buffer(
                       context,   CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR,
                       sizeof(int) * means_gmem_size, &merge_gmem_Ext, &err));
    timer.start();    
    for (int i = 0; i < N_CU_SEPARATE; i++) {
        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({minibatch_gmem_buf[i]}, 0 /* 0 means from host*/));
    }
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({merge_gmem_buf}, 0));
    q.finish();    
    timer.stop();  
    std::cout<<"host to device: "<<timer.get_time()<<"\n";
    

    int narg = 0;
    for (int i = 0; i < N_CU_SEPARATE; i++) {
    // Setting the k_vadd Arguments
        OCL_CHECK(err, err = user_kernel.setArg(narg++, minibatch_gmem_buf[i]));
    }
    OCL_CHECK(err, err = user_kernel.setArg(narg++, merge_gmem_buf));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, n));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, k));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, d));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, iteration));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, batch_size));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, batch_size / threads));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, batch_num));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, iteration * batch_num));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, connection));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, listenPort));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, rxByteCnt));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, destIP));
    OCL_CHECK(err, err = user_kernel.setArg(narg++, destPort));

    timer.start();    
    OCL_CHECK(err, err = q.enqueueTask(user_kernel));
    q.finish();
    timer.stop();
    std::cout<<"computing time: "<<timer.get_time()<<"\n";

    timer.start();
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects(
                                    {merge_gmem_buf},
                                    CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();
    timer.stop();
    std::cout<<"device to host: "<<timer.get_time()<<"\n";
    
    scale_int2float_means(means, means_gmem, scale_ratio, k, d);
    
    float sse = get_sse(n, k, d, dmatrix, means);
    printf("Final mini batch sse:%f\n", sse);
 /////////////////////////////////////device_end_2///////////////////////////////////////
}

float calculate_scale_ratio(int n, int d){
    return 1000;
}

void scale_float2int_minibatch(std::vector<uint, aligned_allocator<uint>> *minibatch_gmem_points[], 
                    float *dmatrix, float scale_ratio, int n_cu_seperate,
                    int batch_size, int threads, int batch_num, int d){
    int tmp0 = batch_size / threads;
    int vd = d;
    for (int i = 0; i < n_cu_seperate; i++){
        for (int j = 0; j < batch_num; j++){
            for (int k = 0; k < tmp0; k++){
                for (int m = 0; m < d; m++){
                    (*minibatch_gmem_points[i])[j * tmp0 * vd + k * vd + m] = (uint)(dmatrix[j * batch_size * d + i * tmp0 * d + k * d + m] * pow(2,31));
                }
                for (int m = d; m < vd; m++){
                    (*minibatch_gmem_points[i])[j * tmp0 * vd + k * vd + m] = 0;
                }                
            }
        } 
    }
}

void scale_int2float_means(float *means, 
                    std::vector<uint, aligned_allocator<uint>> &means_gmem, 
                    float scale_ratio, int k, int d){
    int vd = d;                    
    for (int i = 0; i < k; i++){
        for (int j = 0; j < d; j++){
            means[i * d + j] = (float)(means_gmem[i * vd + j]) / pow(2,31);
        }
    }

}

void scale_float2int_means(std::vector<uint, aligned_allocator<uint>> &means_gmem, 
                        float *means, float scale_ratio, int k, int d){
    int vd = d;
    for (int i = 0; i < k; i++){
        for (int j = 0; j < d; j++){
            means_gmem[i * vd + j] = (uint)(means[i * d + j] * pow(2,31));
        }
        for (int j = d; j < vd; j++){
            means_gmem[i * vd + j] = 0;
        }        
    }    
}
