# Processing of the CRT data files

## Step 0 - Preliminary (POT)

### Get BNB POT info from database

Use the scripts `pot/extract201*.sh` to download the BNB spill information for all the days in 2017 and 2018. These scripts will create a file for each day containing `second, millisecond, pot` information for each spill from the accelerator database. These files are created in txt format. Run the script `database_to_pickle.py` to have them in pkl format as well.
Copy the folder `extract_from_database` into the sbnd/data area: `/sbnd/data/users/mdeltutt/crt_data/pot/`.


## Step 1 - Pair Builder

Run the `pair_builder` to extract beam-triggered events. 
This code takes as input raw DAQ files and makes files in binary format that contain
beam-triggered events.

Steps to run:
- Go in `pair_builder/`
- Compile with `make`
- Adjust the output file path in `run_pair_builder.sh`
- Test run with `source test_run.sh`
- Run on all files with `source run_pair_builder.sh`


The `pair_builder` will create an ouptput binary file for each input binary file, plus an
additional txt file with all the processed T1 events (beam spills). This txt file
is needed for step 2, as it will be correlated with the accelerator database to retrieve
the POT per spill.

## Step 2 - POT Correlation

In the same `pair_builder/`, run the `parse_spill_files.py` to correlate the T1 spills with the POTs.


## Step 3 - From Binary to libCRT ROOT Files

Convert the files from binary to libCRT ROOT format. This is done in the `conversion` folder.
The `ConvertRawPairsToRun.C` macro takes as input
binary files from the previous step, gets the 1D hits from them, makes 2D hits out of every two
coincident 1D hits, and saves all these hits in libCRT format to an output ROOT file.

Orignal libCRT: https://github.com/kreslo/libCRT
Currently used libCRT: 

After this step, one can merge all the output ROOT files into one, and the run:
- `ConvertRawPairsToRun`: 
- `GroupAndClassify`: Groups 2D hits together based on time coincidence. We use a 50000 ns (50 ms) time
coincidence for this, as we want to group hits belonging to the same spill.
- `ExtractPassing`: Which

Steps to run:
- Go in `conversion/`
- Set the right file paths in `ReprocessAllRuns.sh`
- Run with `source ReprocessAllRuns.sh`


## Step 4 - From libCRT to flat ROOT TTree

Flat the libCRT tree. This is done by `flat_crt_tree.C` in the `conversion` folder.


## Step 5 - Convert to art-ROOT

Finally, we want to convert to LArSoft art-ROOT format. Code to do this is in feature branch ``
inside the `sbndcode/CRTBeamTelescope/DataImport` folder.

Run with 
`lar -c import_crt_data_sbnd.fcl -s /sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.classified.flat.root`


## Step 6 - Custom Reconstruction

To be done. Work started in `sbndcode/CRTBeamTelescope/Reconstruction/` in feature branch ``.














