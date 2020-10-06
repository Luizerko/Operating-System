import matplotlib.pyplot as plt
import numpy as np

objects = ('FCFS', 'SRTN')
x = [1000, 1085]
std_d = [0, 0]
y_pos = np.arange(len(objects))

barlist = plt.bar(y_pos, x, align='center', alpha=0.5, yerr=std_d)
barlist[0].set_color('r')
barlist[1].set_color('g')
plt.xticks(y_pos, objects)
plt.xlabel('Escalonadores')
plt.ylabel('Média de mudanças de contexto')
plt.title('Mudanças de contexto por cada escalonador para caso grande')
plt.show()