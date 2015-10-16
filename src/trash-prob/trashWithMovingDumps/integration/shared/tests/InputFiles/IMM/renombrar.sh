#/bin/bash
#cp A02.vehicles.txt cambiar
#cp A02.otherlocs.txt cambiar
for FILE in *; 
do 
	pref=`echo $FILE | sed 's/\..*.txt$//'`
	cp A02.otherlocs.txt cambiar;
	cd cambiar;
	for FILE in * ; do NEWFILE=`echo $FILE | sed 's/^A02.//'` ; mv $FILE $NEWFILE ; done
	for FILE in * ; do mv $FILE $pref.$FILE ; done
	mv * ../
	cd ..
done



