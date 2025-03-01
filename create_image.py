import pandas as pd
import matplotlib.pyplot as plt

data_parallel = pd.read_csv('parallel.txt')
data_iterative = pd.read_csv('iteration.txt')

matrix_size_parallel = data_parallel['Matrixsizexsize(random)']
time_parallel = data_parallel['Time(s)']

matrix_size_iterative = data_iterative['Matrixsizexsize(random)']
time_iterative = data_iterative['Time(s)']

df_parallel = pd.DataFrame({'Matrix Size': matrix_size_parallel, 'Time (s)': time_parallel})
df_iterative = pd.DataFrame({'Matrix Size': matrix_size_iterative, 'Time (s)': time_iterative})

grouped_parallel = df_parallel.groupby('Matrix Size').mean().reset_index()
grouped_iterative = df_iterative.groupby('Matrix Size').mean().reset_index()

plt.figure(figsize=(12, 8))

plt.plot(grouped_parallel['Matrix Size'], grouped_parallel['Time (s)'], marker='o', linestyle='-', color='b', label='Parallel OpenMP')

plt.plot(grouped_iterative['Matrix Size'], grouped_iterative['Time (s)'], marker='x', linestyle='--', color='r', label='Iterative Method')

plt.title('Comparison of Average Time vs Matrix Size')
plt.xlabel('Matrix Size')
plt.ylabel('Average Time (s)')
# plt.yscale('log')
plt.legend()
plt.grid(True, which='both', linestyle='--', linewidth=0.5)

plt.savefig('comparison_average_time_vs_matrix_size_log.png')

plt.show()
