REPS=$1
OUTPUT=$2
DELTA=$3
EXEC=$4

reps=1
while [ $reps -le $REPS ] ; do
	./$EXEC test.g $DELTA >> $OUTPUT ;
	echo -n " " >> $OUTPUT ;
	reps=$(( $reps+1 )) ;
done

echo "" >> $OUTPUT