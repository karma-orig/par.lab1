#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cuda_runtime.h>
#include <chrono>
#ifdef _WIN32
    #include <Windows.h>
#endif

using namespace std;

__global__ void matrixMultiplyKernel(double* a, double* b, double* result, int size) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    if (row < size && col < size) {
        double sum = 0;
        for (int k = 0; k < size; k++) {
            sum += a[row * size + k] * b[k * size + col];
        }
        result[row * size + col] = sum;
    }
}

void writeMatrixToFile(const string& filename, const vector<vector<double>>& matrix) {
    ofstream file(filename);
    file << matrix.size() << " " << matrix[0].size() << "\n";
    for (const auto& row : matrix) {
        for (double val : row)
            file << val << " ";
        file << "\n";
    }
}

vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = (double)rand() / RAND_MAX;
    return matrix;
}

vector<vector<double>> matrixMultiplyCUDA(const vector<vector<double>>& a, const vector<vector<double>>& b) {
    int size = a.size();
    int bytes = size * size * sizeof(double);

    double *d_a, *d_b, *d_result;
    cudaMalloc((void**)&d_a, bytes);
    cudaMalloc((void**)&d_b, bytes);
    cudaMalloc((void**)&d_result, bytes);

    vector<double> a_flat(size * size), b_flat(size * size);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++) {
            a_flat[i * size + j] = a[i][j];
            b_flat[i * size + j] = b[i][j];
        }

    cudaMemcpy(d_a, a_flat.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b_flat.data(), bytes, cudaMemcpyHostToDevice);

    dim3 threadsPerBlock(16, 16);
    dim3 blocksPerGrid((size + 15) / 16, (size + 15) / 16);
    matrixMultiplyKernel<<<blocksPerGrid, threadsPerBlock>>>(d_a, d_b, d_result, size);

    vector<double> result_flat(size * size);
    cudaMemcpy(result_flat.data(), d_result, bytes, cudaMemcpyDeviceToHost);

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_result);

    vector<vector<double>> result(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            result[i][j] = result_flat[i * size + j];
    
    return result;
}

int main(int argc, char* argv[]) {
    cout<<"Running";
    cout << "argc = " << argc << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv[" << i << "] = " << argv[i] << endl;
    }
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <output.txt>" << endl;
        return 1;
    }
    ofstream timingFile(argv[1], ios::app);
    timingFile << "Matrixsizexsize(random),Time(s),equalsround1e-6\n";
    timingFile.close();

    srand(time(0));

    string python_script_path = "d:/3Curszprodolj/paralel/kod/1labavsc/verify_result.py";
    string result_filename = "result.txt";
    string mat1_filename = "mat1.txt";
    string mat2_filename = "mat2.txt";

    for (int size = 10, i = 0; size <= 100; ++i) {
        vector<vector<double>> matrix1 = generateRandomMatrix(size);
        vector<vector<double>> matrix2 = generateRandomMatrix(size);

        writeMatrixToFile(mat1_filename, matrix1);
        writeMatrixToFile(mat2_filename, matrix2);

        clock_t start = clock();
        vector<vector<double>> result = matrixMultiplyCUDA(matrix1, matrix2);
        clock_t end = clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;

        writeMatrixToFile(result_filename, result);

        string command = "python3 " + python_script_path;

        #ifdef _WIN32
            FILE* pipe = _popen(command.c_str(), "r");
        #else
            FILE* pipe = popen(command.c_str(), "r");
        #endif

        if (!pipe) {
            cerr << "Ошибка: не удалось запустить Python-скрипт" << endl;
            return -1;
        }

        char buffer[128];
        string output;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }

        #ifdef _WIN32
            _pclose(pipe);
        #else
            pclose(pipe);
        #endif

        ofstream outputFile(argv[1], ios::app);
        outputFile << size << "," << time << "," << "\n" << output;
        outputFile.close();

        cout << "Time taken: " << time << " seconds" << endl;
        cout << "Task size: " << size << "x" << size << " * " << size << "x" << size << " = " << size << "x" << size << endl;

        if (i >= 100) {
            size += 5;
            i = 0;
        }
    }

    remove("mat1.txt");
    remove("mat2.txt");
    remove("result.txt");

    return 0;
}