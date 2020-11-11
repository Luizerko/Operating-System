import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import operator

f1 = open("tempo_grafico_pista_grande_poucos.txt", "r")

contador = 0
aux = []
for i in f1:
    i = i.split("\n")
    if(contador%2 != 0):
        aux.append(i[0])
    contador += 1

f1.close()
print(aux)
print()

tempo_5 = []
dicionario = {}
soma = 1
for i in aux:
    string_aux_min = ""
    string_aux_sec = ""
    start_min = 0
    start_sec = 0
    for j in i:
        if(j == 's'):
            start_sec = 0

        if(start_sec == 1):
            if(j == ','):
                string_aux_sec = string_aux_sec + '.'
            else:
                string_aux_sec = string_aux_sec + j

        if(j == 'm'):
            start_min = 0
            start_sec = 1

        if(start_min == 1):
            string_aux_min = string_aux_min + j

        if(j == 'l'):
            start_min = 1
    
    if(i != '5 ' and i != ''):
        tempo_5.append(float(string_aux_min)*60 + float(string_aux_sec))
        if('5' in dicionario.keys()):
            dicionario['5'] = dicionario['5'] + float(string_aux_min)*60 + float(string_aux_sec)
        else:
            dicionario['5'] = float(string_aux_min)*60 + float(string_aux_sec)
        soma += 1

dicionario['5'] = dicionario['5']/soma
print(tempo_5)
print(dicionario)
print()

f1 = open("tempo_grafico_pista_grande_medios.txt", "r")

contador = 0
aux = []
for i in f1:
    i = i.split("\n")
    if(contador%2 != 0):
        aux.append(i[0])
    contador += 1

f1.close()
print(aux)
print()

tempo_20 = []
soma = 1
for i in aux:
    string_aux_min = ""
    string_aux_sec = ""
    start_min = 0
    start_sec = 0
    for j in i:
        if(j == 's'):
            start_sec = 0

        if(start_sec == 1):
            if(j == ','):
                string_aux_sec = string_aux_sec + '.'
            else:
                string_aux_sec = string_aux_sec + j

        if(j == 'm'):
            start_min = 0
            start_sec = 1

        if(start_min == 1):
            string_aux_min = string_aux_min + j

        if(j == 'l'):
            start_min = 1
    
    if(i != '20 ' and i != ''):
        tempo_20.append(float(string_aux_min)*60 + float(string_aux_sec))
        if('20' in dicionario.keys()):
            dicionario['20'] = dicionario['20'] + float(string_aux_min)*60 + float(string_aux_sec)
        else:
            dicionario['20'] = float(string_aux_min)*60 + float(string_aux_sec)
        soma += 1

dicionario['20'] = dicionario['20']/soma
print(tempo_20)
print(dicionario)
print()

f1 = open("tempo_grafico_pista_grande_muitos.txt", "r")

contador = 0
aux = []
for i in f1:
    i = i.split("\n")
    if(contador%2 != 0):
        aux.append(i[0])
    contador += 1

f1.close()
print(aux)
print()

tempo_50 = []
soma = 1
for i in aux:
    string_aux_min = ""
    string_aux_sec = ""
    start_min = 0
    start_sec = 0
    for j in i:
        if(j == 's'):
            start_sec = 0

        if(start_sec == 1):
            if(j == ','):
                string_aux_sec = string_aux_sec + '.'
            else:
                string_aux_sec = string_aux_sec + j

        if(j == 'm'):
            start_min = 0
            start_sec = 1

        if(start_min == 1):
            string_aux_min = string_aux_min + j

        if(j == 'l'):
            start_min = 1
    
    if(i != '50 ' and i != ''):
        tempo_50.append(float(string_aux_min)*60 + float(string_aux_sec))
        if('50' in dicionario.keys()):
            dicionario['50'] = dicionario['50'] + float(string_aux_min)*60 + float(string_aux_sec)
        else:
            dicionario['50'] = float(string_aux_min)*60 + float(string_aux_sec)
        soma += 1

dicionario['50'] = dicionario['50']/soma
print(tempo_50)
print(dicionario)
print()

f, a = plt.subplots(2)

#stdev = [44.090, 153.446, 7.868000]

a[0].set_xlabel('Quantidade n de ciclistas na pista')
a[0].set_ylabel('Esperança do tempo')
a[0].set_title('Tempo de conclusão do programa para pista grande (s)')
#a[0].bar(dicionario.keys(), dicionario.values(), yerr=stdev, width=0.6, ecolor='black', color='blue')
a[0].bar(dicionario.keys(), dicionario.values(), width=0.6, color='blue')

f2 = open("memoria_grafico_pista_grande_poucos.txt", "r")

dicionario_mem = {}
aux = []
for i in f2:
    if(len(i) > 3):
        aux = i.split(" ")

aux_2 = ""
for i in aux[11]:
    if(i == ","):
        aux_2 = aux_2 + "."
    else:
        aux_2 = aux_2 + i

dicionario_mem['5'] = float(aux_2)
print(dicionario_mem)

f2 = open("memoria_grafico_pista_grande_medios.txt", "r")

aux = []
for i in f2:
    if(len(i) > 3):
        aux = i.split(" ")

aux_2 = ""
for i in aux[11]:
    if(i == ","):
        aux_2 = aux_2 + "."
    else:
        aux_2 = aux_2 + i

dicionario_mem['20'] = float(aux_2)
print(dicionario_mem)

f2 = open("memoria_grafico_pista_grande_muitos.txt", "r")

aux = []
for i in f2:
    if(len(i) > 3):
        aux = i.split(" ")

aux_2 = ""
for i in aux[11]:
    if(i == ","):
        aux_2 = aux_2 + "."
    else:
        aux_2 = aux_2 + i

dicionario_mem['50'] = float(aux_2)
print(dicionario_mem)

a[1].set_xlabel('Quantidade n de ciclistas na pista')
a[1].set_ylabel('Uso da memória')
a[1].set_title('Uso de memória do programa para pista grande (Kbytes)')
a[1].bar(dicionario_mem.keys(), dicionario_mem.values(), width=0.6, color='red')

plt.tight_layout()
plt.show()