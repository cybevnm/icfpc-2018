#!/bin/sh

set -e

pushd ignore

##############

echo "Removing results..."
rm -rf results/

echo "Making results..."
mkdir results

echo "Copying defaults..."
cp dfltTracesF/* results/

#############

pushd problemsF

echo "Genearating..."
files=(FA*_tgt.mdl)
for f in "${files[@]}"
do
	etrace $f "../results/${f%_tgt.mdl}.nbt"
done

popd

##############

echo "Packing..."
pushd results

zip ../results.zip *
shasum -a 256 ../results.zip

popd

##############

popd
