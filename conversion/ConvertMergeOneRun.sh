#
# Argument 1: The data dir (i.e. "/sbnd/data/users/mdeltutt/crt_data/beam/")
# Argument 2: The file name base (i.e. "ProdRun20171210_001006")
# Argument 3: The output dir

echo "ConvertMergeOneRun starts..."

root -l -b -q 'ConvertRawPairsToRun.C("'$1'", "'$2'-sbndcrt01.1.beam", "'$3'")'
echo "Processed file $datadir$1 01"

root -l -b -q 'ConvertRawPairsToRun.C("'$1'", "'$2'-sbndcrt02.1.beam", "'$3'")'
echo "Processed file $datadir$1 02"

root -l -b -q 'Merge.C("'$3$2'-sbndcrt01.1.beam.libcrt.root","'$3$2'-sbndcrt02.1.beam.libcrt.root","'$3$2'.beam.root")'
echo "Merged"

#rm -f m*.root
rm -f datadir$1-sbndcrt0*.1.beam.libcrt.root

