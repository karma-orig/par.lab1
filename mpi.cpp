#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

void writeMatrixToFile(const string& filename, const vector<vector<double> >& matrix) {
    ofstream file(filename.c_str());
    file << matrix.size() << " " << matrix[0].size() << "\n";
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix[i].size(); j++) {
            file << matrix[i][j] << " ";
        }
        file << "\n";
    }
}

void writeResultToFile(const string& filename, int size, double time, const string& python_output) {
    ofstream file(filename.c_str(), ios::app);
    file << size << "," << fixed << setprecision(6) << time << "," << python_output;
}

vector<vector<double> > generateRandomMatrix(int size) {
    vector<vector<double> > matrix(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = (double)rand() / RAND_MAX;
    return matrix;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc != 2) {
        if (rank == 0) {
            cerr << "Usage: " << argv[0] << " <output.txt>" << endl;
        }
        MPI_Finalize();
        return 1;
    }

    string python_script_path = "./py.txt";
    string result_filename = "result.txt";
    string mat1_filename = "mat1.txt";
    string mat2_filename = "mat2.txt";

    if (rank == 0) {
        ofstream timingFile(argv[1], ios::app);
        timingFile << "Matrixsizexsize(random),Time(s),equalsround1e-6\n";
        timingFile.close();
        srand(time(0));
    }

    for (int size = 10, i = 0; size <= 500; ++i) {
        vector<vector<double> > matrix1, matrix2, result;

        if (rank == 0) {
            matrix1 = generateRandomMatrix(size);
            matrix2 = generateRandomMatrix(size);
            writeMatrixToFile(mat1_filename, matrix1);
            writeMatrixToFile(mat2_filename, matrix2);
        }

        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        vector<double> flat_matrix1(size * size);
        vector<double> flat_matrix2(size * size);
        vector<double> flat_result(size * size, 0);

        if (rank == 0) {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    flat_matrix1[i * size + j] = matrix1[i][j];
                    flat_matrix2[i * size + j] = matrix2[i][j];
                }
            }
        }

        MPI_Bcast(flat_matrix1.data(), size * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(flat_matrix2.data(), size * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        int rows_per_proc = size / num_procs;
        int remainder = size % num_procs;
        int start_row = rank * rows_per_proc + min(rank, remainder);
        int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);

        double start_time = MPI_Wtime();

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < size; j++) {
                double sum = 0.0;
                for (int k = 0; k < size; k++) {
                    sum += flat_matrix1[i * size + k] * flat_matrix2[k * size + j];
                }
                flat_result[i * size + j] = sum;
            }
        }

        vector<int> recv_counts(num_procs);
        vector<int> displs(num_procs);

        int sum = 0;
        for (int p = 0; p < num_procs; p++) {
            int p_rows = size / num_procs + (p < remainder ? 1 : 0);
            recv_counts[p] = p_rows * size;
            displs[p] = sum;
            sum += recv_counts[p];
        }

        vector<double> all_results;
        if (rank == 0) {
            all_results.resize(size * size);
        }

        MPI_Gatherv(flat_result.data() + start_row * size, 
                   (end_row - start_row) * size, 
                   MPI_DOUBLE,
                   all_results.data(), 
                   recv_counts.data(), 
                   displs.data(), 
                   MPI_DOUBLE,
                   0, 
                   MPI_COMM_WORLD);

        double end_time = MPI_Wtime();
        double time = end_time - start_time;

        if (rank == 0) {
            result.resize(size, vector<double>(size));
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    result[i][j] = all_results[i * size + j];
                }
            }

            writeMatrixToFile(result_filename, result);

            char command[1024];
            snprintf(command, sizeof(command), "python3 %s", python_script_path.c_str());

            FILE* pipe = popen(command, "r");
            if (!pipe) {
                MPI_Finalize();
                return -1;
            }

            char buffer[128];
            string output = "";
            while (!feof(pipe)) {
                if (fgets(buffer, 128, pipe) != NULL)
                    output += buffer;
            }
            pclose(pipe);

            writeResultToFile(argv[1], size, time, output);

            cout << "Time taken: " << time << " seconds" << endl;
            cout << "Task size: " << size << "x" << size << " * " << size << "x" << size 
                 << " = " << size << "x" << size << endl;
        }

        if (rank == 0 && i >= 100) {
            size += 5;
            i = 0;
        }

        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&i, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        remove("mat1.txt");
        remove("mat2.txt");
        remove("result.txt");
    }

    MPI_Finalize();
    return 0;
}