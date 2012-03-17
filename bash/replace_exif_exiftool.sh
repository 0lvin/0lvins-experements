for d in *
do
	if [ -d "$d" ]; then
		cd "$d"
		for f in *
		do
			if [ -d "$f" ]; then
				cd "$f"
				echo "in $f"
				ls -1
				ls -1 *.jpg | sed 's|-|\t|g' | sed 's|_|\t|g' | sed 's|\.|\t|g' | awk ' { print "exiftool -AllDates=20" $3 ":" $2 ":" $1 "-" substr($4,0,3) ":" substr($4, 3, 2) ":00  " $1 "-" $2 "-" $3 "_" $4 ".jpg" } ' > 1.sh 
				sh 1.sh
				rm 1.sh
				exiftool -Comment="$f" -Make="Motorola" -Model="SLVR L9" -Flash="Off, Did not fire" *.jpg
				rm *_original
				cd ..
			fi
		done
		cd ..
	fi
done
