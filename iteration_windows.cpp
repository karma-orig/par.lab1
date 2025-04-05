#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <cstdio> // Добавлено для popen/pclose
#ifdef _WIN32
    #include <Windows.h>
#endif

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

vector<vector<double>> generateRandomMatrix(int size) {
    vector<vector<double>> matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = (double)rand() / RAND_MAX;
    return matrix;
}

vector<vector<double>> matrixMultiply(const vector<vector<double>>& a, const vector<vector<double>>& b) {
    int rowsA = a.size(), colsA = a[0].size(), colsB = b[0].size();
    vector<vector<double>> result(rowsA, vector<double>(colsB));
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            double sum = 0;
            for (int k = 0; k < colsA; k++)
                sum += a[i][k] * b[k][j];
            result[i][j] = sum;
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
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

        auto start = chrono::high_resolution_clock::now();
        vector<vector<double>> result = matrixMultiply(matrix1, matrix2);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        double time = elapsed.count();

        writeMatrixToFile(result_filename, result);

        // Формируем команду для вызова Python
        string command = "python3 " + python_script_path;

        // Используем popen или _popen в зависимости от ОС
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

        // Закрываем поток
        #ifdef _WIN32
            _pclose(pipe);
        #else
            pclose(pipe);
        #endif

        // Записываем результаты в файл
        ofstream outputFile(argv[1], ios::app);
        outputFile << fixed << setprecision(6) << size << "," << time << "," << "\n" << output;
        outputFile.close();

        cout << fixed << setprecision(6);
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
