# Processing of the CRT data files

## Step 1

Run the `pair_builder` to extract beam-triggered events. 
This code takes as input raw DAQ files and makes files in binary format that contain
beam-triggered events.

Steps to run:
- Go in `pair_builder/`
- Compile with `make`
- Adjust the output file path in `run_pair_builder.sh`
- Run with `source run_pair_builder.sh`


## Step 2

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
