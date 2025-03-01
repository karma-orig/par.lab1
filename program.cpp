#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <omp.h> 
#include <chrono>

using namespace std;

void writeMatrixToFile(const string& filename, const vector<vector<double>>& matrix) {
    ofstream file(filename);
    file << matrix.size() << " " << matrix[0].size() << "\n";
    for (const auto& row : matrix) {
        for (double val : row)
            file << val << " ";
        file << "\n";
    }
}

void writeResultToFile(const string& filename, int size, double time) {
    ofstream file(filename, ios::app);
    file << size << " " << fixed << setprecision(6) << time << "\n";
}

vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = (double)rand() / RAND_MAX;
    return matrix;
}

// vector<vector<double>> matrixMultiply(const vector<vector<double>>& a, const vector<vector<double>>& b) {
//     int rowsA = a.size(), colsA = a[0].size(), colsB = b[0].size();
//     vector<vector<double>> result(rowsA, vector<double>(colsB));

//     for (int i = 0; i < rowsA; i++) {
//         for (int j = 0; j < colsB; j++) {
//             double sum = 0.0;
//             for (int k = 0; k < colsA; k++) {
//                 sum += a[i][k] * b[k][j];
//             }
//             result[i][j] = sum;
//         }
//     }
//     return result;
// }


vector<vector<double>> matrixMultiply(const vector<vector<double>>& a, const vector<vector<double>>& b) {
    int rowsA = a.size(), colsA = a[0].size(), colsB = b[0].size();
    vector<vector<double>> result(rowsA, vector<double>(colsB));

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += a[i][k] * b[k][j];
                }
                result[i][j] = sum;
            }
        }
    return result;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <output.txt>" << endl;
        return 1;
    }

    ofstream timingFile(argv[1], ios::app);
    timingFile << "Matrixsizexsize(random),Time(s),equalsround1e-6\n";
    timingFile.close();

    srand(time(0));

    string python_script_path = "./verify_result.py";
    string result_filename = "result.txt";
    string mat1_filename = "mat1.txt";
    string mat2_filename = "mat2.txt";

    for (int size = 10, i = 0; size <= 500; ++i) {
        vector<vector<double>> matrix1 = generateRandomMatrix(size);
        vector<vector<double>> matrix2 = generateRandomMatrix(size);

        writeMatrixToFile(mat1_filename.c_str(), matrix1);
        writeMatrixToFile(mat2_filename.c_str(), matrix2);

        auto start = chrono::high_resolution_clock::now();
        vector<vector<double>> result = matrixMultiply(matrix1, matrix2);
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> time = (end - start);

        writeMatrixToFile(result_filename.c_str(), result);

        char command[1024];
        snprintf(command, sizeof(command), "python3 %s", python_script_path.c_str());

        FILE* pipe = popen(command, "r");
        if (!pipe) return -1;

        char buffer[128];
        std::string output = "";
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != nullptr)
                output += buffer;
        }
        pclose(pipe);

        ofstream outputFile(argv[1], ios::app);
        outputFile << size << "," << time.count() << "," << output;

        cout << "Time taken: " << time.count() << " seconds" << endl;
        cout << "Task size: " << size << "x" << size << " * " << size << "x" << size << " = " << size << "x" << size << endl;

        if(i >= 100) {
            size+=5;
            i = 0;
        }
    }

    remove("mat1.txt");
    remove("mat2.txt");
    remove("result.txt");

    return 0;
}
