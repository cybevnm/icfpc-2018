#!/bin/sh

set -e

pushd ignore

##############

echo "-----------------------"
echo "Preapring results dir..."
rm -rf results/
mkdir results
cp dfltTracesF/* results/

echo "-----------------------"
echo "Preparing models dir..."
rm -rf models/
mkdir models/

#############

pushd problemsF

echo "-----------------------"
echo "Genearating..."
files=(FA*_tgt.mdl)
for f in "${files[@]}"
do
	etrace $f "../results/${f%_tgt.mdl}.nbt" "../models/$f"
done

popd

##############

pushd problemsF

echo "-----------------------"
echo "Testing..."
files=(FA*_tgt.mdl)
for f in "${files[@]}"
do
	diff $f "../models/$f" || echo "Wrong model: ../models/$f"
done

popd

##############

echo "-----------------------"
echo "Packing..."
pushd results

zip -q ../results.zip *
shasum -a 256 ../results.zip

popd

##############

popd
