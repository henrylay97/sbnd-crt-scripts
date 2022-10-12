for FILE in `ls /sbnd/data/users/hlay/crt_bt_data/flattened_trees/*.root`
do
    export FILE=$FILE
    export base_file_name=$(basename $FILE ".flattened.tree.root")
    echo $base_file_name
    lar -c import_crt_data_sbnd.fcl -s $FILE -o /sbnd/data/users/hlay/crt_bt_data/imported_data/${base_file_name}_imported_data.root
done




