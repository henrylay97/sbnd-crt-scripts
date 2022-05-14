#! /bin/bash

for date in 2017-{06..12}-{01..31}
#for date in 2017-{06..06}-{24..24}
#for date in 2018-01-{01..03}
do

if [[ ! -s BNB_TS_$date.txt ]];
then

echo Requesting beam data for $date
echo "https://dbdata1vm.fnal.gov:9080/ifbeam/data/data?v=E:TOR875&e=e,1d&t0="$date"T00:00:00.000-05:00&t1="$date"T23:59:59.999-05:00&action=Show+event&f=xml" ;
wget -q -O - "https://dbdata1vm.fnal.gov:9080/ifbeam/data/data?v=E:TOR875&e=e,1d&t0="$date"T00:00:00.000-05:00&t1="$date"T23:59:59.999-05:00&action=Show+event&f=xml"  |grep clock | sed -n 's/^.*clock="//p' | sed 's/">/ /' | sed 's/./& /10' | sed 's/ /,/g'  >> extract_from_database/BNB_TS_$date.txt

else

echo $date exists and not empty

fi


done
