#include <iostream>
#include <CL/cl2.hpp>
#include <ctime>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Function to generate random numbers between 0 and 9 for an array
void generateRandomArray(int* array, int size) {
    for (int i = 0; i < size; ++i) {
        array[i] = rand() % 10;
    }
}

int main() {
    const int SIZE = 600000;

    // Initialize input arrays with random numbers
    int arrayA[SIZE];
    int arrayB[SIZE];
    generateRandomArray(arrayA, SIZE);
    generateRandomArray(arrayB, SIZE);

    //store the start time
    auto start = high_resolution_clock::now();

    // Initialize result array
    int arrayC[SIZE] = {0};

        // Get available OpenCL platforms
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        // Get available OpenCL devices (use the first platform)
        std::vector<cl::Device> devices;
        platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

        if (devices.empty()) {
            std::cout << "No GPU device found. Using CPU." << std::endl;
            platforms[0].getDevices(CL_DEVICE_TYPE_CPU, &devices);
        }

        // Create an OpenCL context for the device
        cl::Context context(devices);

        // Create an OpenCL command queue for the context
        cl::CommandQueue queue(context, devices[0]);

        // Create OpenCL memory buffers for arrays A, B, and C
        cl::Buffer bufA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, arrayA);
        cl::Buffer bufB(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, arrayB);
        cl::Buffer bufC(context, CL_MEM_WRITE_ONLY, sizeof(int) * SIZE);

        // Load and compile the OpenCL program
        const char* source = R"(
            __kernel void add_arrays(__global const int* A, __global const int* B, __global int* C) {
                int i = get_global_id(0);
                if (i < 5) {
                    C[i] = A[i] + B[i];
                }
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
        cl::NDRange global(SIZE);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global);
        queue.finish();

        // Read the result array back to the host
        queue.enqueueReadBuffer(bufC, CL_TRUE, 0, sizeof(int) * SIZE, arrayC);

            //store the time (stop time) in a variable
    auto stop = high_resolution_clock::now();

    //calculates the time difference (duration of execution)
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds: " << endl;
/*
        // Print the input and result arrays
        std::cout << "Array A: ";
        for (int i = 0; i < SIZE; ++i) {
            std::cout << arrayA[i] << " ";
        }
        std::cout << std::endl;

        std::cout << "Array B: ";
        for (int i = 0; i < SIZE; ++i) {
            std::cout << arrayB[i] << " ";
        }
        std::cout << std::endl;

        std::cout << "Result Array C: ";
        for (int i = 0; i < SIZE; ++i) {
            std::cout << arrayC[i] << " ";
        }
        std::cout << std::endl;
    */

    return 0;
}
