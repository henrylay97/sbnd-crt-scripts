#! /bin/bash

#
# Extracts beam trigger-related events from raw DAQ output files
#

# Where the CRT raw data is located:
datadir="/sbnd/data/CRT/CRT_data/"

# Where to place the processed files:
# outdir="/sbnd/data/users/mdeltutt/crt_data/beam/"
outdir="/sbnd/data/users/mdeltutt/crt_data/beam_with_pot/"
# outdir="/sbnd/data/users/mdeltutt/crt_data/test/"


# for f in $datadir*Run2017121?_*-sbndcrt01.*
for f in $datadir*Run*_*-sbndcrt0?.*
# for f in $datadir*Run20170623_15213?-sbndcrt02.1
# for f in $datadir*Run201806??_*-sbndcrt02.1
do
    echo "Running pair builder on file $f"

    dir_name=$(dirname $f)
    base_name=$(basename $f)

    echo "./pair_builder $f $outdir/$base_name.beam 3 > /dev/null"
    ./pair_builder $f $outdir/$base_name.beam 3 > /dev/null
    echo "python parse_spill_files.py --file $outdir/$base_name.beam.spills.txt > $outdir/$base_name.beam.spills.txt.log"
    python parse_spill_files.py --file $outdir/$base_name.beam.spills.txt > $outdir/$base_name.beam.spills.txt.log

done


