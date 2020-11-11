#!/bin/bash

make;

for i in $(seq 1 1); do
	{ time ./ep2 1000 50 >/dev/null; } |& grep real >> tempo_grafico_pista_grande_muitos.txt;
done
echo finalizei tempo pista grande para muitos ciclistas;

#valgrind --log-file="memoria" ./ep2 500 5;
#cat memoria | grep allocated >> memoria_grafico_pista_media_poucos.txt;
#rm memoria;
#echo finalizei memoria pista media para poucos ciclistas;

#for i in $(seq 1 30); do
#	{ time ./ep2 1000 20 >/dev/null; } |& grep real >> tempo_grafico_pista_grande_medios.txt;
#done
#echo finalizei tempo pista grande para medios ciclistas;

#valgrind --log-file="memoria" ./ep2 1000 20;
#cat memoria | grep allocated >> memoria_grafico_pista_grande_medios.txt;
#rm memoria;
#echo finalizei memoria pista grande para medios ciclistas;

#for i in $(seq 1 30); do
#	{ time ./ep2 250 50 >/dev/null; } |& grep real >> tempo_grafico_pista_pequena_muitos.txt;
#done
#echo finalizei tempo pista pequena para muitos ciclistas;

#valgrind --log-file="memoria" ./ep2 250 50;
#cat memoria | grep allocated >> memoria_grafico_pista_pequena_muitos.txt;
#rm memoria;
#echo finalizei memoria pista pequena para muitos ciclistas;

make clean;
