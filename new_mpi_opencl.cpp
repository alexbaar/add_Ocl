#include <iostream>
#include <CL/cl2.hpp>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <mpi.h>

using namespace std;
using namespace std::chrono;

// Function to generate random numbers between 0 and 9 for an array
void generateRandomArray(int* array, int size) {
    for (int i = 0; i < size; ++i) {
        array[i] = rand() % 10;
    }
}

int main(int argc, char** argv) {
    const int SIZE = 600000 ;
    int num_procs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int local_size = SIZE / num_procs;

    // Initialize input arrays with random numbers
    int* localArrayA = new int[local_size];
    int* localArrayB = new int[local_size];
    int* localArrayC = new int[local_size];

    if (rank == 0) {
        int* arrayA = new int[SIZE];
        int* arrayB = new int[SIZE];
        generateRandomArray(arrayA, SIZE);
        generateRandomArray(arrayB, SIZE);
/*
        // Print Array A
        cout << "       Array A: ";
        for (int i = 0; i < SIZE; ++i) {
            cout << arrayA[i] << " ";
        }
        cout << endl;

        // Print Array B
        cout << "       Array B: ";
        for (int i = 0; i < SIZE; ++i) {
            cout << arrayB[i] << " ";
        }
        cout << endl;
*/
        // Scatter input data to all processes
        MPI_Scatter(arrayA, local_size, MPI_INT, localArrayA, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(arrayB, local_size, MPI_INT, localArrayB, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        delete[] arrayA;
        delete[] arrayB;
    } else {
        // Receive scattered data
        MPI_Scatter(nullptr, local_size, MPI_INT, localArrayA, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(nullptr, local_size, MPI_INT, localArrayB, local_size, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Store the start time
    auto start = high_resolution_clock::now(); // Moved this line up

    // Initialize OpenCL
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    // Get available OpenCL devices (use the first platform)
    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty()) {
        //std::cout << "No GPU device found. Using CPU." << std::endl;
        platforms[0].getDevices(CL_DEVICE_TYPE_CPU, &devices);
    }

    // Create an OpenCL context for the device
    cl::Context context(devices);

    // Create an OpenCL command queue for the context
    cl::CommandQueue queue(context, devices[0]);

    // Create OpenCL memory buffers for arrays A, B, and C
    cl::Buffer bufA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * local_size, localArrayA);
    cl::Buffer bufB(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * local_size, localArrayB);
    cl::Buffer bufC(context, CL_MEM_WRITE_ONLY, sizeof(int) * local_size);

    // Load and compile the OpenCL program
    const char* source = R"(
        __kernel void add_arrays(__global const int* A, __global const int* B, __global int* C) {
            int i = get_global_id(0);
            C[i] = A[i] + B[i];
        }
    )";

    cl::Program program(context, source);
    program.build(devices);

    // Create the OpenCL kernel
    cl::Kernel kernel(program, "add_arrays");

    // Set kernel arguments
    kernel.setArg(0, bufA);
    kernel.setArg(1, bufB);
    kernel.setArg(2, bufC);

    // Execute the OpenCL kernel
    cl::NDRange global(local_size);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global);
    queue.finish();

    // Read the result array back to the host
    queue.enqueueReadBuffer(bufC, CL_TRUE, 0, sizeof(int) * local_size, localArrayC);

    // Gather results on rank 0
    if (rank == 0) {
        int* combinedArrayC = new int[SIZE];
        MPI_Gather(localArrayC, local_size, MPI_INT, combinedArrayC, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        // Store the stop time
        auto stop = high_resolution_clock::now();

        // Calculate the time difference (duration of execution)
        auto duration = duration_cast<microseconds>(stop - start);

        // Print the result array C
/*        cout << "Result Array C: ";
        for (int i = 0; i < SIZE; ++i) {
            cout << combinedArrayC[i] << " ";
        }
        cout << endl;
*/        
        cout << "Time taken by function: " << duration.count() << " microseconds" << endl;

        delete[] combinedArrayC;
    } else {
        MPI_Gather(localArrayC, local_size, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    delete[] localArrayA;
    delete[] localArrayB;
    delete[] localArrayC;

    MPI_Finalize();

    return 0;
}
