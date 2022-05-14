# Processing of the CRT data files

## Step 0

### Get BNB POT info from database

Use the scripts `pot/extract201*.sh` to download the BNB spill information for all the days in 2017 and 2018. These scripts will create a file for each day containing `second, millisecond, pot` information for each spill from the accelerator database. These files are created in txt format. Run the script `database_to_pickle.py` to have them in pkl format as well.
Copy the folder `extract_from_database` into the sbnd/data area: `/sbnd/data/users/mdeltutt/crt_data/pot/`.


## Step 1

Run the `pair_builder` to extract beam-triggered events. 
This code takes as input raw DAQ files and makes files in binary format that contain
beam-triggered events.

Steps to run:
- Go in `pair_builder/`
- Compile with `make`
- Adjust the output file path in `run_pair_builder.sh`
- Run with `source run_pair_builder.sh`

The pair_builder will create an uptput binary file for each input binary file, plus an
additional txt file with all the processed T1 events (beam spills). This txt file
is needed for step 2, as it will be correlated with the accelerator database to retrieve
the POT per spill.

## Step 2

Run the `parse_spill_files.py` to correlate the T1 spills with the POTs.


## Step 3

Convert the files from binary to libCRT ROOT format. The `ConvertRawPairsToRun.C` macro takes as input
binary files from the previous step, gets the 1D hits from them, makes 2D hits out of every two
coincident 1D hits, and save all these hits in libCRT format to an output ROOT file.

libCRT: https://github.com/kreslo/libCRT

After this step, one can merge all the output ROOT files into one, and the run:
- `GroupAndClassify`: Which groups 2D hits together based on time coincidence.
- `ExtractPassing`: Which

Steps to run:
- Go in `conversion/`
- Set the right file paths in `ReprocessAllRuns.sh`
- Run with `source ReprocessAllRuns.sh`


## Step 3 (POT)

Correlate the current entries with the POTs. To be documented.
















