import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('out.txt')

matrix_size = data['Matrixsizexsize(random)']
time = data['Time(s)']

df = pd.DataFrame({'Matrix Size': matrix_size, 'Time (s)': time})

grouped_data = df.groupby('Matrix Size').mean().reset_index()

plt.figure(figsize=(10, 6))
plt.plot(grouped_data['Matrix Size'], grouped_data['Time (s)'], marker='o', linestyle='-', color='b')

plt.title('Average Time vs Matrix Size')
plt.xlabel('Matrix Size')
plt.ylabel('Average Time (s)')
plt.grid(True)

plt.savefig('average_time_vs_matrix_size.png')

plt.show()
