# python parse_spill_files.py --file  /sbnd/data/users/mdeltutt/crt_data/test/test02___.spills.txt

import os.path
import argparse
import pandas as pd
import numpy as np
import uproot3 as uproot
import datetime

TIME_SECOND_RESOLUTION = 3
TIME_MILLISECOND_RESOLUTION = 50 # BNB spills have the potential to come as close to each other as ~67 ms (15 Hz). Using 50 ms here to be sure.
JAN1_2017 = 1483228800
BNB_FILE_PATH = '/sbnd/data/users/mdeltutt/crt_data/pot/extract_from_database/'
N_BNB_FILE_TO_OPEN = 5

parser = argparse.ArgumentParser(description='Process spill files.')
parser.add_argument('--file', type=str,
                    help='Full file path')

args = parser.parse_args()
print('Using file', args.file)

# Understand the start date from the file
date = args.file.split('ProdRun')[1].split('_')[0]
year  = int(date[0:4])
month = int(date[4:6])
day   = int(date[6:8])
date  = datetime.date(year, month, day)

print('Run started on', date)
# bnb_file = 'BNB_TS_' + date.__str__() + '.pkl'
# print('Using BNB file', bnb_file)


bnb_files = []
for i in range(0, N_BNB_FILE_TO_OPEN):
    this_date = date + datetime.timedelta(days=i)
    bnb_file = BNB_FILE_PATH + 'BNB_TS_' + this_date.__str__() + '.pkl'
    if os.path.isfile(bnb_file):
        print('Using BNB file', bnb_file)
        bnb_files.append(bnb_file)
    else:
        print('File', bnb_file, 'not available.')

pot_df = None
for bnb_file in bnb_files:
    df = pd.read_pickle(bnb_file)
    if pot_df is None:
        pot_df = df
    else:
        pot_df = pd.concat([pot_df, df])




# bnb_pot_file = '/sbnd/data/users/mdeltutt/crt_data/pot/BNB_TS_2017-18.root'
# pot_tree = uproot.open(bnb_pot_file)['dt']
# pot_df = pot_tree.pandas.df()

# bnb_pot_file = '/sbnd/app/users/mdeltutt/Projects/CRTData/file_processing/pot/extract_from_database/bnb_ts.pkl'
# bnb_pot_file = '/sbnd/app/users/mdeltutt/Projects/CRTData/file_processing/pot/extract_from_database/BNB_TS_2017-06-23.pkl'
# pot_df = pd.read_pickle(bnb_pot_file)



t1_df = pd.read_csv(args.file, comment='#')

mac = 95 if 95 in t1_df.mac.unique() else 96 # Need to select one MAC otherwise we may have multiple entries for the same s and ms

print('Using mac', mac)

t1_df = t1_df.query('s > @JAN1_2017')
t1_df = t1_df.query('mac == @mac')
t1_df = t1_df.drop_duplicates(subset=['s', 'ms'])

min_s = t1_df['s'].min()
max_s = t1_df['s'].max()


pot_df = pot_df.query('s >= @min_s - @TIME_SECOND_RESOLUTION and s <= @max_s + @TIME_SECOND_RESOLUTION')

print('min_s', min_s, '- max_s', max_s, '- len(pot_df)', len(pot_df), '- len(t1_df)', len(t1_df))

# Output lists
out_t1_seconds = []
out_t1_milliseconds = []
out_pot = []
out_delta_s = []
out_delta_ms = []

n_matched = 0
n_unmatched = 0

# Group the dataframe based on the second
grouped = t1_df.groupby('s')

# Loop over this grouped dataframe, and for each
# second lopp over all the T1 events, then loop
# over all the spills from the POT tree, and look
# for a time match
for second, df_group in grouped:

    # print('Looking at second:', second)

    # Get the spills compatible with this second
    this_pot_df = pot_df.query('s >= @second - @TIME_SECOND_RESOLUTION and s <= @second + @TIME_SECOND_RESOLUTION')

    # Loop over the T1 events in this second
    for i, (t1_index, t1_row) in enumerate(df_group.iterrows()):

        match_found = False
        min_delta_s = 1e20
        min_delta_ms = 1e20
        min_delta_s_pot = -1
        min_delta_ms_pot = -1

        # Loop over the spills compatible with this second
        for j, (pot_index, pot_row) in enumerate(this_pot_df.iterrows()):

            delta_s = abs(t1_row.s - pot_row.s)
            delta_ms = abs(t1_row.ms - pot_row.ms)
            delta_ms = min(delta_ms, abs(delta_ms - 1000))
            # Last line needed because ms are cyclical, if we have an event at ms = 0 and another
            # at ms = 999 (because in the previous second), this is a good match

            # print('pot s', pot_row.s, '- t1 s', t1_row.s)

            if delta_s <= TIME_SECOND_RESOLUTION and delta_ms <= TIME_MILLISECOND_RESOLUTION:

                match_found = True

                # Look for a minimum
                if delta_s < min_delta_s:
                    min_delta_s = delta_s
                    min_delta_s_pot = pot_row.pot

                if delta_ms < min_delta_ms:
                    min_delta_ms = delta_ms
                    min_delta_ms_pot = pot_row.ms

        out_t1_seconds.append(t1_row.s)
        out_t1_milliseconds.append(t1_row.ms)

        out_delta_s.append(min_delta_s)
        out_delta_ms.append(min_delta_ms)

        if not match_found:
            print('!!!!!!!!!!!!!! Cannot match T1 event with s', t1_row.s, 'ms', t1_row.ms)
            n_unmatched += 1
            out_pot.append(0)

        else:
            print('Matched - s', t1_row.s, 'ms', t1_row.ms, 'pot', min_delta_s_pot)
            n_matched += 1
            out_pot.append(min_delta_s_pot)

print('Number of matched T1 events:', n_matched)
print('Number of unmatched T1 events:', n_unmatched)

data = {
    't1_s': out_t1_seconds,
    't1_ms': out_t1_milliseconds,
    'pot': out_pot,
    'delta_s': out_delta_s,
    'delta_ms': out_delta_ms,
}

out_df = pd.DataFrame(data)


out_df.to_csv(args.file + '.pot.txt')


# Write the total POT on a separate file
f = open(args.file + '.totalpot.txt', "w")
f.write(str(np.sum(out_pot)) + '\n') # total pot
f.write(str(len(out_pot))) # number of spills
f.close()



# https://dbdata1vm.fnal.gov:9080/ifbeam/data/data?v=E:TOR875&e=e,1d&t0=2017-12-26T00:00:00.000-05:00&t1=2017-12-26T23:59:59.999-05:00&action=Show+event&f=xml
# http://ifb-data.fnal.gov:8100/ifbeam/data/data?b=BoosterNeutrinoBeam_read&e=e,1d&t0=2017-12-26T00:00:00.000-05:00&t1=2017-12-26T23:59:59.999-05:00&f=csv

