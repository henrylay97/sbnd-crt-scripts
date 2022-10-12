for FILE in `ls /sbnd/data/users/mdeltutt/crt_data/beam_converted_may2022/ProdRun*.beam.classified.root`
do
    export FILE=$FILE
    export base_file_name=$(basename $FILE ".beam.classified.root")
    echo $base_file_name
    root -l -b -q 'flat_crt_tree.C("${FILE}", "/sbnd/data/users/hlay/crt_bt_data/flattened_trees/${base_file_name}.flattened.tree.root")'
done




