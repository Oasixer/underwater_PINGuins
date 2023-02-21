#!/bin/bash
NANOPB_GENERATOR_PATH=`cat ./nanopb_generator_path`
#if [ ! -f $NANOPB_GENERATOR_PATH ]; then
#	echo
#	echo Error! no file found at $NANOPB_GENERATOR_PATH
#	echo
#	echo FIX WITH THE FOLLOWING STEPS!!
#	echo ---------------------------------------------------------------
#	echo "git clone https://github.com/nanopb/nanopb ~/nanopb"
#	echo "echo ~/nanopb/generator/nanopb_generator.py > ./nanopb_generator_path"
#	echo
#	echo "(optional): replace ~/nanopb in both commands with a different install location"
#	echo ---------------------------------------------------------------
#fi
DIR=test_adc_dump

for i in tmp_data
do
	cd proto; winpty python.exe "$NANOPB_GENERATOR_PATH" -I . $i.proto
	cd ..
	mv -v proto/$i.pb.h ./$DIR/
	mv -v proto/$i.pb.c ./$DIR/
done
# protoc --python_out=./dashboard ./proto/*_data.proto
~/protoc/bin/protoc --python_out=./dashboard ./proto/*_data.proto

# replace "#include <pb.h>" with "#include "src/nanopb/pb.h" in all generated .pb.h files in main/
sed -i 's/#include <pb.h>/#include "src\/nanopb\/pb.h"/g' $DIR/*.pb.h
