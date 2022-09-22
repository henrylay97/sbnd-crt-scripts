# Runs the conversion for only one run

# Convert upstream
root -l -b -q 'ConvertRawPairsToRun.C("/sbnd/data/users/mdeltutt/crt_data/test/", "ProdRun20170623_152137-sbndcrt01.1.beam", "/sbnd/data/users/mdeltutt/crt_data/test/")'

# Convert downstream
root -l -b -q 'ConvertRawPairsToRun.C("/sbnd/data/users/mdeltutt/crt_data/test/", "ProdRun20170623_152137-sbndcrt02.1.beam", "/sbnd/data/users/mdeltutt/crt_data/test/")'

# Merge upstream and downstream
root -l -b -q 'Merge.C("/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137-sbndcrt01.1.beam.libcrt.root","/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137-sbndcrt02.1.beam.libcrt.root","/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.libcrt.root")'

# Classify
root -l -b -q 'GroupAndClassify.C("/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.libcrt.root","/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.libcrt.root.libcrt.classified.root")'

# Convert to flat tree
root -l -b -q 'flat_crt_tree.C("/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.libcrt.root.libcrt.classified.root", "/sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.classified.flat.root")'



