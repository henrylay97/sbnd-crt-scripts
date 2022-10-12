for FILE in `ls /sbnd/data/users/hlay/crt_bt_data/imported_data/*.root`
do
    export FILE=$FILE
    export base_file_name=$(basename $FILE "_imported_data.root")
    echo $base_file_name
    lar -c reco_data_crt_bt_sbnd.fcl -s $FILE -o /sbnd/data/users/hlay/crt_bt_data/reco/${base_file_name}_reco.root
done
