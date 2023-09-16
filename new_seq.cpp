#include <iostream>
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

    // Initialize result array
    int arrayC[SIZE] = {0};

    // Store the start time
    auto start = high_resolution_clock::now();

    // Sequential array addition
    for (int i = 0; i < SIZE; ++i) {
        arrayC[i] = arrayA[i] + arrayB[i];
    }

    // Store the stop time
    auto stop = high_resolution_clock::now();

    // Calculate the time difference (duration of execution)
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: " << duration.count() << " microseconds: " << endl;

    /*
    // Print the input and result arrays
    cout << "Array A: ";
    for (int i = 0; i < SIZE; ++i) {
        cout << arrayA[i] << " ";
    }
    cout << endl;

    cout << "Array B: ";
    for (int i = 0; i < SIZE; ++i) {
        cout << arrayB[i] << " ";
    }
    cout << endl;

    cout << "Result Array C: ";
    for (int i = 0; i < SIZE; ++i) {
        cout << arrayC[i] << " ";
    }
    cout << endl;
    */

    return 0;
}

