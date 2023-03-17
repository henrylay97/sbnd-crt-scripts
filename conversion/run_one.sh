for FILE in `ls /sbnd/data/users/hlay/crt_bt_data/flattened_trees/*.root`
do
    export FILE=$FILE
    export base_file_name=$(basename $FILE ".flattened.tree.root")
    echo $base_file_name
    lar -c import_crt_data_sbnd.fcl -s $FILE -o /pnfs/sbnd/scratch/users/hlay/crt_bt_data/imported_data/${base_file_name}_imported_data.root
    lar -c reco_data_crt_bt_sbnd.fcl -s /pnfs/sbnd/scratch/users/hlay/crt_bt_data/imported_data/${base_file_name}_imported_data.root -o /pnfs/sbnd/scratch/users/hlay/crt_bt_data/reco/${base_file_name}_reco.root
    lar -c ana_data_crt_bt_sbnd.fcl -s /pnfs/sbnd/scratch/users/hlay/crt_bt_data/reco/${base_file_name}_reco.root -T /pnfs/sbnd/scratch/users/hlay/crt_bt_data/ana/${base_file_name}_ana.root -n 1000000
    break
done
