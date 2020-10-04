#!/bin/bash

#for i in $(seq 1 30); do
#	./gera 10 trace$i.txt;
#done

for i in $(seq 1 30); do
	./ep1 3 trace$i.txt simulacao.txt d;
done
