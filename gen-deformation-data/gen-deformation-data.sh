#!/bin/bash

# (cd ../src && make && echo && pwd && echo) # automake

cmake CMakeLists.txt
make

echo && pwd && echo

for epoch in 0 1 2
do
    echo epoch $epoch
    ./gen-deformation-data-$epoch > data-$epoch.gkf
    ../src/gama-local data-$epoch.gkf \
		      --xml  data-$epoch.xml \
		      --svg  data-$epoch.svg \
		      --text data-$epoch.txt \

done

DEFO=`find .. -name gama-local-deformation`

$DEFO data-1.xml \
	  data-2.xml \
     --svg data_1-2.svg \
	 --text data_1-2.txt
