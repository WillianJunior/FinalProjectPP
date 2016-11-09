REPS=$1
OUTPUT=$2
DELTA=$3

reps=1
while [ $reps -le $REPS ] ; do
	./parallel-omp.out test.g $DELTA >> $OUTPUT ;
	echo -n " " >> $OUTPUT ;
	reps=$(( $reps+1 )) ;
done

echo "" >> $OUTPUT