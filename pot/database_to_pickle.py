
import glob
import pandas as pd

files = glob.glob('./extract_from_database/BNB_TS*.txt')

for file in files:
    print('Converting file', file)

    df = pd.read_csv(file, names=['s', 'ms', 'pot'])

    df.to_pickle(file.replace('.txt', '.pkl'))




# import glob
# import pandas as pd

# out_df = None

# files = glob.glob('./BNB_TS*.txt')
# files = glob.glob('BNB_TS_2017-06-24.txt')

# for file in files:
#     print('Adding file', file)

#     df = pd.read_csv(file, names=['s', 'ms', 'pot'])

#     if out_df is None:
#         out_df = df
#     else:
#         out_df = pd.concat([out_df, df])

# out_df.to_pickle("BNB_TS_2017-06-24.pkl")
