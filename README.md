# Processing of the CRT data files

CRT data files are located at `/sbnd/data/CRT/CRT_data/`.


## Setup

Setup with `source setup.sh`.


## Step 0 - Preliminary (POT)

### Get BNB POT info from database

Use the scripts `pot/extract201*.sh` to download the BNB spill information for all the days in 2017 and 2018. These scripts will create a file for each day containing `second, millisecond, pot` information for each spill from the accelerator database. These files are created in txt format. Run the script `database_to_pickle.py` to have them in pkl format as well.
Copy the folder `extract_from_database` into the sbnd/data area: `/sbnd/data/users/mdeltutt/crt_data/pot/`.

You don't need to do this step, as files are already present in `/sbnd/data/users/mdeltutt/crt_data/pot/`.


## Step 1 - Pair Builder

Run the `pair_builder` to extract beam-triggered events. 
This code takes as input raw DAQ files and makes files in binary format that contain
beam-triggered events.

Steps to run:
- Go in `pair_builder/`
- Compile with `make`
- Test run with `source test_run.sh` (make sure you open change the destination path in the script first)

The `pair_builder` will create an ouptput binary file for each input binary file, plus an
additional txt file with all the processed T1 events (beam spills). This txt file
is needed for step 2, as it will be correlated with the accelerator database to retrieve
the POT per spill.


## Step 2 - POT Correlation

In the same `pair_builder/`, run the `parse_spill_files.py` to correlate the T1 spills with the POTs:
```
python parse_spill_files.py --file /path/to/step1/output/file
```

You can run both Step 1 and 2 with:
- Adjust the output file path in `run_pair_builder.sh`
- `source run_pair_builder.sh`.


## Step 3 - From Binary to libCRT ROOT Files

Convert the files from binary to libCRT ROOT format. This is done in the `conversion` folder.
The `ConvertRawPairsToRun.C` macro takes as input
binary files from the previous step, gets the 1D hits from them, makes 2D hits out of every two
coincident 1D hits, and saves all these hits in libCRT format to an output ROOT file.

- Orignal libCRT: https://github.com/kreslo/libCRT
- Currently used libCRT: https://github.com/marcodeltutto/libCRT

After this step, one can merge all the output ROOT files into one.

This is what get run:
- `ConvertRawPairsToRun`: From the binary file, it takes two consecutive 1D hits to make a 2D hit.
- `GroupAndClassify`: Groups 2D hits together based on time coincidence. We use a 50000 ns (50 ms) time
coincidence for this, as we want to group hits belonging to the same spill.
- `ExtractPassing`: Not used.

Run a test:
- Go in `conversion/`
- Set the right file paths in `test_run.sh`
Run with `source test_run.sh`

Run on all files:
- Set the right file paths in `ReprocessAllRuns.sh`
- Run with `source ReprocessAllRuns.sh`


## Step 4 - From libCRT to flat ROOT TTree

Flat the libCRT tree. This is done by `flat_crt_tree.C` in the `conversion` folder.
This script converts root files from libCRT to simple flat tree for easy analysis.


## Step 5 - Convert to art-ROOT

Finally, we want to convert to LArSoft art-ROOT format. Code to do this is in feature branch
`feature/mdeltutt_bt_data_import`
inside the `sbndcode/CRTBeamTelescope/DataImport` folder.

Run with 
`lar -c import_crt_data_sbnd.fcl -s /sbnd/data/users/mdeltutt/crt_data/test/ProdRun20170623_152137.beam.classified.flat.root`


## Step 6 - Custom Reconstruction

To be done. Work started in `sbndcode/CRTBeamTelescope/Reconstruction/` in feature branch
`feature/mdeltutt_bt_data_import`.





## Processed Files

### May 2022

Output of step 2 is in:
`/sbnd/data/users/mdeltutt/crt_data/beam_with_pot/`

Output of step 3 is in:
`/sbnd/data/users/mdeltutt/crt_data/beam_converted_may2022/`

Files called `ProdRun????????_??????.beam.classified.root` need to go to step 4.








