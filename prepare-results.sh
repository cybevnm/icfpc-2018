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
echo "Assembling..."
files=(FA*_tgt.mdl)
for f in "${files[@]}"
do
	assemble $f "../results/${f%_tgt.mdl}.nbt" "../models/$f"
done

popd

#############

pushd problemsF

echo "-----------------------"
echo "Disassembling..."
files=(FD*_src.mdl)
for f in "${files[@]}"
do
	disassemble $f "../results/${f%_src.mdl}.nbt"
done

popd

#############

pushd problemsF

echo "-----------------------"
echo "Reassembling..."
files=(FR*_src.mdl)
for f in "${files[@]}"
do
		reassemble $f "${f%_src.mdl}_tgt.mdl" "../results/${f%_src.mdl}.nbt" \
		  "../models/${f%_src.mdl}_tgt.mdl"
done

popd

##############

pushd problemsF

echo "-----------------------"
echo "Testing assembled..."
files=(FA*_tgt.mdl)
for f in "${files[@]}"
do
	diff $f "../models/$f" || echo "Wrong model: ../models/$f"
done

echo "Testing reassembled..."
files=(FR*_tgt.mdl)
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
