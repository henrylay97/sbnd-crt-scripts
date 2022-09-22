datadir="/sbnd/data/users/mdeltutt/crt_data/beam_with_pot/"
outdir="/sbnd/data/users/mdeltutt/crt_data/beam_converted_may2022/"
merged_file="/sbnd/data/users/mdeltutt/crt_data/beam_converted_may2022_merged/merged_beam_all_libcrt"

# for f in $datadir*Run20??????_*-sbndcrt01.*
for f in $datadir*Run2017????_*-sbndcrt01.*
do

    bf=$(basename $f)
    cbf=${bf:0:22}
    echo "Processing $cbf ..."

    if [ -e "$outdir$cbf.beam.root" ]; then
        echo "File "$cbf".beam.root already exists in "$outdir
    else
        echo "File "$cbf".beam.root does not exist in "$outdir
        echo ./ConvertMergeOneRun.sh $datadir $cbf $outdir
        ./ConvertMergeOneRun.sh $datadir $cbf $outdir
    fi

    # root -l -b -q 'Append.C("'$outdir$cbf'.beam.root","'$merged_file'.root")'
    root -l -b -q 'GroupAndClassify.C("'$outdir$cbf'.beam.root","'$outdir$cbf'.beam.classified.root")'

    # break
done

# echo "Conversion done, now running GroupAndClassify"
# root -l -b -q 'GroupAndClassify.C("'$merged_file'.root","'$merged_file'.classified_test.root")'

# echo "Now running ExtractPassing"
# root -l -b -q 'ExtractPassing.C("'$merged_file'.root","'$merged_file'.passing.root")'
