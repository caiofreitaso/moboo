#!/bin/bash

EXE=./main

PARAMS="sys/zerg state/start model/"
BASIC=" -n one_swap -s 8 -r 1 -i 10 -std fronts -err time -r 30"

NSGA="-o nsga2 -c 0.87 \
-nc 34 \
-obj 2.73 -res 2.81 -dO 2.1 -dR 2.44 \
-Fobj 1.48 -Fres 1.66 -FdO 1.63 -FdR 1.78"
#"-o nsga2 -c 0.78 -nc 82 \
#-objm 2.13 -objM 6.69 -resm 2.31 -resM 2.47 -dOm 1.55 -dOM 2.31 -dRm 2.34 -dRM 3.21 \
#-Fobjm 3.5 -FobjM 4.03 -Fresm 4.83 -FresM 2.13 -FdOm 1.34 -FdOM 1.96 -FdRm 1.61 -FdRM 3.61"

ACO="-o aco -ls 3 -c 0.45 \
-an 7 -aa 0.73 -ab 3.4 -ae 0.6 -ac 2 \
-obj 3.84 -res 6.93 -dO 2.27 -dR 3.13 \
-Fobj 3.71 -Fres 4.45 -FdO 2.96 -FdR 3.47"

KNEE="-o knee -c 0.57 \
-nc 72 \
-obj 2.85 -res 12.76 -dO 1.64 -dR 1.77 \
-Fobj 1.87 -Fres 2.34 -FdO 1.76 -FdR 1.83"

MOGRASP="-o mograsp -ls 8 -c 0.49 \
-mc 12 -mp 0.8 -mh 2 \
-obj 8.03 -res 11.57 -dO 1.7 -dR 3.04 \
-Fobj 5.76 -Fres 5.84 -FdO 2.69 -FdR 2.79"

MOGRASPA="-o mograsp -ls 8 -c 0.51 \
-mc 17 -mp 0.68 -mh 2 \
-obj 2.42 -res 14.74 -dO 1.77 -dR 2.61 \
-Fobj 5.03 -Fres 7.65 -FdO 1.82 -FdR 1.84"

MOGRASPB="-o mograsp -ls 3 -c 0.2 \
-mc 15 -mp 0.94 -mh 2 \
-obj 4.19 -res 6.26 -dO 1.79 -dR 2.11 \
-Fobj 2.86 -Fres 8.78 -FdO 1.56 -FdR 2.09"

MOGRASPC="-o mograsp -ls 1 -c 0.2 \
-mc 15 -mp 0.94 -mh 1 \
-obj 2.73 -res 2.81 -dO 2.1 -dR 2.44 \
-Fobj 1.48 -Fres 1.66 -FdO 1.63 -FdR 1.78"


#"-o aco -ls 7 -c 0.51 \
#-an 10 -aa 0.95 -ab 7.91 -ae 0.28 -ac 2 \
#-Fobjm 4.11 -FobjM 4.44 -Fresm 5.78 -FresM 1.88 -FdOm 1.09 -FdOM 2.46 -FdRm 1.62 -FdRM 2.81"

#
FILES=( guardian-time guardian-full hydralisk hydralisk-full hydralisk-3hatch muta-speedling-time muta-speedling muta-speedling-full ultralisk-time ultralisk-full zerglings-speedtime zerglings-speed zerglings-speedfull zerglings-full )

for i in ${FILES[@]}; do
	echo $i
	#echo "NSGA2"
	#$EXE ${PARAMS}$i ${BASIC} $NSGA 1>> nsga-$i.out 2>> nsga-$i.err
	#echo "MOACO"
	#$EXE ${PARAMS}$i ${BASIC} $ACO  1>> moaco-$i.out 2>> moaco-$i.err
	echo "KNEE"
	$EXE ${PARAMS}$i ${BASIC} $KNEE  1>> knee2-$i.out 2>> knee2-$i.err
	#echo "MOGRASP"
	#$EXE ${PARAMS}$i ${BASIC} $MOGRASPC  1>> mograsp2-$i.out 2>> mograsp2-$i.err
	cat nsga-$i.out knee2-$i.out moaco-$i.out mograsp-$i.out | norm/norm | hv/hv > $i.2nhv
done