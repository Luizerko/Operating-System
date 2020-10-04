#!/bin/bash

#for i in $(seq 1 30); do
#	./gera 100 trace$i.txt;
#done

for i in $(seq 1 30); do
	./ep1 1 trace$i.txt simulacao.txt;
done
