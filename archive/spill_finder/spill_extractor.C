#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#define EVSPERFEB 1024   // max events per feb per poll to buffer
#define MAXFEBNR 256

typedef struct {
    uint16_t mac5;
    uint16_t flags;
    uint16_t lostcpu;
    uint16_t lostfpga;
    uint32_t ts0;
    uint32_t ts1;
    uint16_t adc[32];
} EVENT_t;

typedef struct {
    uint16_t mac5; // ==0xFFFF
    uint16_t flags; // ==0xFFFF
    uint16_t lostcpu;
    uint16_t lostfpga;
    uint32_t ts0; // ==MAGICWORD32
    uint32_t ts1; // ==MAGICWORD32
    int nevsinpoll;
    uint32_t start_s;
    uint32_t d1;
    uint16_t start_ms;
    uint16_t dd2;
    uint32_t d2;
    uint32_t end_s;
    uint32_t d3;
    uint16_t end_ms;
} EOP_EVENT_t;  // end-of-poll special event

EVENT_t evbuf[MAXFEBNR*EVSPERFEB+1+10000];    //buffer to receive events (same structure as the receiving events)

int act_time[2][MAXFEBNR+1];    //number to read out the second and ms out of received special event [0]:sec, [1]:ms [][mac]:module [][MAXFEBNR]:time last poll


int main (int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Provide file name. Exiting." << std::endl;
        return 1;
    }

    char *filename = argv[1];

    FILE *data = fopen(filename,"r");

    fseek(data, 0, SEEK_END); // seek to end of file
    long size = ftell(data); // get current file pointer
    fseek(data, 0, SEEK_SET); // seek back to beginning of file

    size_t size_ev = size / sizeof(EVENT_t);       //number of total events
    std::cout << "Total Number of events: " << size_ev << std::endl;

    std::cout << "size of EVENT_t: " << sizeof(EVENT_t) << std::endl;
    std::cout << "size of EOP_EVENT_t: " << sizeof(EOP_EVENT_t) << std::endl;
    std::vector<uint32_t> ts0_from_ts1_v;

    int received_events = 270000;

    for(int i = 0; i < received_events; i++)
    {
        fread(&evbuf[i], sizeof(EVENT_t), 1, data);

        int mac = evbuf[i].mac5;

        // Special Event
        if(evbuf[i].mac5 == 0xFFFF)
        {
            std::cout << std::endl;;
            std::cout << "\tThis is a special event" << std::endl;;
            // printf("\t\tStart s: %i, ms: %i, end s: %i, ms: %i, n events: %i \n", (int)refevent.start_s, (int)refevent.start_ms, (int)refevent.end_s, (int)refevent.end_ms, (int)refevent.nevsinpoll);
            EOP_EVENT_t refevent;   //special structure for the special events receiving (end of poll)
            memcpy(&refevent, &evbuf[i].mac5, sizeof(EOP_EVENT_t));
            // std::cout << "\t\t Start s: " << refevent.start_s << ", ms: " << refevent.start_ms << std::endl;
            std::cout << "\t\t End s:   " << refevent.end_s << ", ms: " << refevent.end_ms << std::endl;
            // previous_sec=act_time[0][MAXFEBNR];
            // previous_ms=act_time[1][MAXFEBNR];
            act_time[0][MAXFEBNR]=(int)refevent.end_s;
            act_time[1][MAXFEBNR]=(int)refevent.end_ms;
        }
        // T1 Reference Event
        else if ((0x8 & evbuf[i].flags) >> 3)
        {
            int CORR_MSEC = 35;
            int t1_s = act_time[0][MAXFEBNR];
            int t1_ms = evbuf[i].ts0 / 1e6 - CORR_MSEC;
            if(t1_ms < 0) {
              t1_ms = t1_ms + 1000;
              // t1_s = t1_s - 1;
              // if (mac == 90) printf("->reduced\n");
            }
            if (mac == 90)std::cout << std::endl;;
            if (mac == 90) std::cout << "\t\tThis is a T1 reference event --------------------------" << std::endl;
            if (mac == 90) printf("\t\tThis is a T1 reference event --------------------------------------, mac = %i, flags = %i\n", evbuf[i].mac5, evbuf[i].flags);
            if (mac == 90) printf("\t\t                             s = %i, ms = %i, t0 = %lu, t1 = %lu\n", t1_s, t1_ms, evbuf[i].ts0, evbuf[i].ts1);

            ts0_from_ts1_v.push_back(evbuf[i].ts0);
        }
        // T0 Reference Event
        else if ((0x4 & evbuf[i].flags) >> 2)
        {
            if (mac == 90)std::cout << std::endl;;
            if (mac == 90)std::cout << "\t\tThis is a T0 reference event --------------------------" << std::endl;
            uint32_t ts0_reference = evbuf[i].ts0;

            for (auto ts0 : ts0_from_ts1_v)
            {
                long ts0_scaled = ((ts0 * 1e9) / ts0_reference + 0.5); // calculate the scaling factor
                // std::cout << "ts0_scaled " << ts0_scaled << std::endl;
            }
            ts0_from_ts1_v.clear();

        }
        else
        {
            std::cout << ".";
        }
    }

}

















