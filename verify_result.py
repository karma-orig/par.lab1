import numpy as np

def read_matrix_from_file(filename):
    with open(filename, 'r') as file:
        size = tuple(map(int, file.readline().split()))
        matrix = [list(map(float, line.split())) for line in file]
    return np.array(matrix)

def compare_matrices(mat1, mat2, tol=1e-6):
    return np.allclose(mat1, mat2, atol=tol)

def main(mat1_filename, mat2_filename, result_filename):
    mat1 = read_matrix_from_file(mat1_filename)
    mat2 = read_matrix_from_file(mat2_filename)
    result = read_matrix_from_file(result_filename)

    np_result = np.matmul(mat1, mat2)

    if compare_matrices(result, np_result):
        print("true")
    else:
        print("false")

if __name__ == "__main__":
    import sys
    main("mat1.txt", "mat2.txt", "result.txt")