import matplotlib.pyplot as plt
import numpy as np

objects = ('FCFS', 'SRTN', 'RR')
x = [14.3667, 57.1, 13.5667]
std_d = [3.7637, 4.2767, 3.9723]
y_pos = np.arange(len(objects))

barlist = plt.bar(y_pos, x, align='center', alpha=0.5, yerr=std_d)
barlist[0].set_color('r')
barlist[1].set_color('g')
barlist[2].set_color('b')
plt.xticks(y_pos, objects)
plt.xlabel('Escalonadores')
plt.ylabel('Média de deadlines cumpridas')
plt.title('Cumprimento de deadlines por cada escalonador para caso médio')
plt.show()