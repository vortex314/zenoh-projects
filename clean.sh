HERE=`pwd`
for d in `find . -name Cargo.toml -print`
do
	DIR=`dirname $d`
	cd $DIR
	echo $DIR
	cargo clean
	cd $HERE
done
rm -rf */.pio
