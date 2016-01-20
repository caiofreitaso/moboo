#!/bin/bash

EXE=./main

PARAMS="sys/zerg state/start model/"
BASIC=" -n one_swap -s 8 -r 1 -i 10 -std fronts -err time -r 30"

NSGA="-o nsga2 -c 0.87 -nc 34 \
-obj 2.73 -res 2.81 -dO 2.1 -dR 2.44 \
-Fobj 1.48 -Fres 1.66 -FdO 1.63 -FdR 1.78"
#"-o nsga2 -c 0.78 -nc 82 \
#-objm 2.13 -objM 6.69 -resm 2.31 -resM 2.47 -dOm 1.55 -dOM 2.31 -dRm 2.34 -dRM 3.21 \
#-Fobjm 3.5 -FobjM 4.03 -Fresm 4.83 -FresM 2.13 -FdOm 1.34 -FdOM 1.96 -FdRm 1.61 -FdRM 3.61"

ACO="-o aco -ls 3 -c 0.45 \
-an 7 -aa 0.73 -ab 3.4 -ae 0.6 -ac 2 \
-obj 3.84 -res 6.93 -dO 2.27 -dR 3.13 \
-Fobj 3.71 -Fres 4.45 -FdO 2.96 -FdR 3.47"

#"-o aco -ls 7 -c 0.51 \
#-an 10 -aa 0.95 -ab 7.91 -ae 0.28 -ac 2 \
#-Fobjm 4.11 -FobjM 4.44 -Fresm 5.78 -FresM 1.88 -FdOm 1.09 -FdOM 2.46 -FdRm 1.62 -FdRM 2.81"

FILES=(guardian-time hydralisk hydralisk-full muta-speedling-time ultralisk-time zerglings-speedfull-atk)

for i in ${FILES[@]}; do
	echo $i
	#echo "NSGA2"
	#$EXE ${PARAMS}$i ${BASIC} $NSGA 1>> nsga-$i.out 2>> nsga-$i.err
	#echo "MOACO"
	#$EXE ${PARAMS}$i ${BASIC} $ACO  1>> moaco-$i.out 2>> moaco-$i.err
	cat nsga-$i.out moaco-$i.out | norm/norm | hv/hv > $i.nhv
done