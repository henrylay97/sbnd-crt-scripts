
# python get_total_pot.py --file "/sbnd/data/users/mdeltutt/crt_data/beam_with_pot/ProdRun*-sbndcrt02.1.beam.spills.txt.pot.txt"


import os.path
import argparse
import pandas as pd
import numpy as np
import uproot3 as uproot
import datetime
import glob

parser = argparse.ArgumentParser(description='Process spill files.')
parser.add_argument('--file', type=str,
                    help='Full file path')

args = parser.parse_args()
print('Using file', args.file)

files = glob.glob(args.file)
print('Expanded to', len(files), 'files')


dates = []
pots = []

for i, f in enumerate(files):


    # Understand the start date from the file
    s = f.split('ProdRun')[1].split('_')
    date = s[0]
    year  = int(date[0:4])
    month = int(date[4:6])
    day   = int(date[6:8])

    date = s[1].split('-sbndcrt')[0]
    hour   = int(date[0:2])
    minute = int(date[2:4])
    second = int(date[4:6])
    # print('file', f)
    # print(hour, minute, second)


    time_str = f'{day}/{month}/{year} {hour}:{minute}:{second}'
    date_format_str = '%d/%m/%Y %H:%M:%S'
    # create datetime object from timestamp string
    date = datetime.datetime.strptime(time_str, date_format_str)

    # print('date', date, int(date.timestamp()))

    df = pd.read_csv(f)
    pot = np.sum(df.pot.values) * 1e12

    pots.append(pot)
    dates.append(int(date.timestamp()))

    print('File', i, 'of', len(files), '      ', date, 'pot:', pot)


data = {
    'date': dates,
    'pot': pots,
}

out_df = pd.DataFrame(data)
out_df.to_csv('all_pot.csv')





