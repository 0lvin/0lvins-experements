for f in *
do
	if [ -d "$f" ]; then
		cd "$f"
		ls -1
		ls -1 *.jpg | sed 's|-|\t|g' | sed 's|_|\t|g' | sed 's|\.|\t|g' | awk ' { print "jhead -mkexif -ts20" $3 ":" $2 ":" $1 "-" substr($4,0,3) ":" substr($4, 3, 2) ":00  " $1 "-" $2 "-" $3 "_" $4 ".jpg" } ' > 1.sh 
		sh 1.sh 
		jhead -cl "$f" *.jpg
		rm 1.sh
		cd ..
	fi
done
