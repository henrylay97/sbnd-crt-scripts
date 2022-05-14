///////////////////////////////////////////////////////////////////////////////////////////////////////
//    This program receive hits from a febdriver and searches for ts1_ref events
//    Note: Set maximal timedifference of events to select wrt ts1_ref in the code
//    This Version is for permanent use (no counters)
//    This filter is for uBooNe!!!
//    Written by Thomas Mettler
///////////////////////////////////////////////////////////////////////////////////////////////////////

//#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

//#define MAX_TIME_DIFFERENCE 20000   //Set the timedifference [ns] between ts1_ref and events to select.
// ----- Original values:
// #define MAX_TIME_PREBEAM 50000
// #define MAX_TIME_PASTBEAM 200
// ----- Marco's values:
#define MAX_TIME_PREBEAM 100000
#define MAX_TIME_PASTBEAM 50000
// ----- For quick POT values:
// #define MAX_TIME_PREBEAM 10
// #define MAX_TIME_PASTBEAM 50
#define EVLEN 80        // event length of a raw event (80 for uBooNE)
#define WAIT 0       // wait x us after sending
#define EVSPERFEB 1024   // max events per feb per poll to buffer
#define MAXFEBNR 256
#define MSOVERLAP 50000
#define MAX_TIME_DIFFERENCE 400   //Set the maximal timedifference between hits


//define numbers to controll bufferstatus
#define PROBUF_READY_TO_FILL  0
#define PROBUF_FILLING        1
#define PROBUF_READY_TO_SCALE 2
#define PROBUF_SHIFT_SCALE    3

#define SCANBUF_READY_TO_FILL 0
#define SCANBUF_SCANNING      1

#define FILTER_MODE 1
#define PAIR_MODE 2
#define FILTER_PAIR_MODE 3


#define DEBUG 1

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
		uint16_t mac5;
		uint16_t flags;
    uint16_t lostcpu;
    uint16_t lostfpga;
		uint32_t ts0;
		uint32_t ts1;
		uint16_t adc[32];
    uint16_t recover;
    uint32_t nrtrigger;
    uint32_t nrtrigger_11;
} EVENT_t_send;

typedef struct {
		uint16_t mac5;
		uint16_t flags;
    uint16_t lostcpu;
		uint16_t lostfpga;
		uint32_t ts0;
		uint32_t ts1;
		uint16_t adc[32];
		uint32_t ts0_scaled;
    uint32_t ts1_scaled;
		uint32_t sec;
    uint32_t ts0_ref;
    uint32_t ms;
    uint16_t recover;
    uint32_t nrtrigger;
    uint32_t nrtrigger_11;
} EVENT_tpro;

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

typedef struct{
  uint32_t sec;
  int ref_nr;
  uint32_t ts0_ref;
  int flags;
} SCAN_ORDER;

EVENT_t evbuf[MAXFEBNR*EVSPERFEB+1];    //buffer to receive events (same structure as the receiving events)

EVENT_tpro evbuf_pro[MAXFEBNR+1][4*EVSPERFEB+1];  //buffer for processing (add the second, millisecond from sepcial events)
EVENT_tpro evbuf_scan[MAXFEBNR+1][4*EVSPERFEB+1]; //buffer for scanning for coincidences (same structure as the buffer for processing)
EVENT_tpro evbuf_filter[MAXFEBNR+1][4*EVSPERFEB+1];
EVENT_tpro evbuf_filter_scan[MAXFEBNR+1][4*EVSPERFEB+1];

EVENT_t_send beam_ev[10][4*EVSPERFEB+1];    //buffer to send out the coincidences (structure idealy same as the received events)
EVENT_t ts0_ref_event[2];
EVENT_t_send ts0_ref_event_buffer[MAXFEBNR+1][2];
EVENT_t_send coincidence[10][MAXFEBNR+1];    //buffer to send out the coincidences (structure idealy same as the received events)
int coincidence_sec[10][MAXFEBNR+1];  //integer to store the second in a special event with the coincidences
int coincidence_ms[10][MAXFEBNR+1];   //integer to store the milli second in a special event with the coincidences


int ev_counter_mac[MAXFEBNR+1];   //Number of events per module (mac) in the processing buffer
int ev_counter_scan[MAXFEBNR+1];  //Number of events per module (mac) in the scanning buffer
int ev_counter_filter_scan[MAXFEBNR+1];  //Number of events per module (mac) in the scanning buffer
int ev_counter_filter[MAXFEBNR+1];
EOP_EVENT_t refevent;   //special structure for the special events receiving (end of poll)
int act_time[2][MAXFEBNR+1];    //number to read out the second and ms out of received special event [0]:sec, [1]:ms [][mac]:module [][MAXFEBNR]:time last poll
int previous_sec, previous_ms;
int previous2_sec;
int event_time_diff[MAXFEBNR+1];
int event_time_diff_old[MAXFEBNR+1];

void * publisher; //Socket for 3-4 hits in one coincidence or more

int ready_to_send[10], send_bufnr=0, ready_to_fill, ready_to_scan;
FILE *data=0;
long size_ev=0;
long counter_tot=0;
long counter_old=0;
long stuck_event_counter=0;
SCAN_ORDER order_buffer[MAXFEBNR+1]; //to scan the events with the lowest second number
//beamfilter variables
int last2_ms[MAXFEBNR+1];
int last1_ms[MAXFEBNR+1];
int number_ms[MAXFEBNR+1];

//beamfilter V2 variables
int ts1ref_buffer[100][100];
int ts1ref_buffer_s[100][100]; // marco
// std::vector<std::pair<int,int>> _approved_t1s; // marco
int ts1ref_counter[100];
int ts1ref_second[100];
int previous_sec_scan[MAXFEBNR+1];
int previous_sec_mac[MAXFEBNR+1];
int previous_sec_mac2[MAXFEBNR+1];

FILE *file_spills = 0;

int ts0ref_counter[MAXFEBNR+1];
int nrwm1[MAXFEBNR+1], nrwm2[MAXFEBNR+1];
int run_mode=0; //choose the mode (filtering, pairfinding etz...)

int minus2_counter1=0, same_counter1=0;

void usage();   //gives you information how to run
void receive_data(void * subscriber); // receive data from zmq socket
void shift_scale(int mac, int ref_nr, int ts0_ref); //scale the timestamps and copy them for processing
void scale_buffer();  // scale the timestams
void scan_buffer_filter(int mac);  // scan/process all hits of the FEBs and searches for coincidences
void filter_buffer(int mac);
void scan_filter_buffer(int mac);
int send_coinc(int bufnr, int found_coinc); //send coincidences with 3-4 hits
int send_ts0_ref(int mac);
int send_ts0_ref_buffer(int mac);

int find_min_sec( int send_bufnr, int coinc_counter); //search for the minimum second in a coincidence
int find_max_sec( int send_bufnr, int coinc_counter); //search for the aximum second in a coincidence
int find_min_ms( int bufnr, int hit_number);
void store_data_pairs(int bufnr, int found_coinc);
void scan_buffer(int mac);  // scan/process all hits of the FEBs and searches for coincidences


void store_data(int bufnr, int found_coinc);
void store_data_ts0(int mac);
void store_data_ts0_buffer(int mac);
FILE *file_store;

void free_bufer(void * data, void * hint);  //clear the buffer after sending the coincidence
//int find_min_sec( int send_bufnr, int coinc_counter); //search for the minimum second in a coincidence
//int find_max_sec( int send_bufnr, int coinc_counter); //search for the aximum second in a coincidence
//int find_min_ms( int bufnr, int hit_number);

int main (int argc, char **argv){
  if(argc!=4) { usage(); return 0;} //test the input variables and give hint if needed

  // In case you want to make a text file to look on the data...
  //text = fopen("Data_scan_feb.txt","w");
  //fprintf(text,"mac5	flags	ts0	ts0_scaled adc1\n");

  //  Socket to receive data from febs
  /*int rv_receive;
  char *iface_receive=argv[1];
  void * context_receive = zmq_ctx_new ();
  void *subscriber = zmq_socket (context_receive, ZMQ_SUB);*/
  void *subscriber = 0; /*
  if(subscriber<=0) {printf("Can't initialize the socket for receiving!\n"); return 0;}
  rv_receive=zmq_connect (subscriber, iface_receive);
  if(rv_receive<0) {printf("Can't connect to the socket for receiving!\n"); return 0;}
  rv_receive=zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
  if(rv_receive<0) {printf("Can't set SUBSCRIBE option to the socket!\n"); return 0;}
  */
  char *filename=argv[1];
  data=fopen(filename,"r");
  fseek(data, 0, SEEK_END); // seek to end of file
	long size = ftell(data); // get current file pointer
	fseek(data, 0, SEEK_SET); // seek back to beginning of file

	size_ev=size/sizeof(EVENT_t);		//number of total events
  //size_ev=100000;
	printf("Total Number of events: %ld\n",size_ev);

  //  Socket to send data to clients, more than 2 hits
  /*void * context_send = zmq_ctx_new();
  int rv_send;
  char *iface_send=argv[2];
  publisher = zmq_socket (context_send, ZMQ_PUB);
  if(publisher<=0) {printf("Can't initialize the socket for sending!\n"); return 0;}
  rv_send = zmq_bind (publisher, iface_send);
  if(rv_send<0) {printf("Can't bind tcp socket for data sending! Exiting.\n"); return 0;}
  */
  char *filename_store=argv[2];
  file_store=fopen(filename_store,"wb");
	run_mode=atoi(argv[3]);

  // marco: spills
  file_spills = fopen(strcat(filename_store, ".spills.txt"), "w"); // marco
  if (file_spills == NULL) {
    printf("Error opening file_spills file!\n");
  }
  fprintf(file_spills, "# Spills from T1 reference events\ns,ms,mac\n");

  //set all hit counters (from scan buffer and pro buffer) to 0
  for(int i=0;i<MAXFEBNR+1;i++){
    ev_counter_mac[i]=0;
    ev_counter_scan[i]=0;
    ev_counter_filter[i]=0;
    order_buffer[i].flags=1;
    nrwm1[i]=0;
    nrwm2[i]=0;
    ts0ref_counter[i]=0;
    ev_counter_filter_scan[i]=0;
    event_time_diff[i]=0;
    event_time_diff_old[i]=0;
    previous_sec_scan[i]=0;
    previous_sec_mac[i]=0;
  }
  previous_sec=0;
  previous_ms=0;
  for(int i=0; i<100;i++){
    ts1ref_counter[i]=0;
    ts1ref_second[i]=0;
  }
  ready_to_fill=PROBUF_READY_TO_FILL;

  int counter = 0;

  //endless loop for receiving-processing-sending events///////////////////////////////////////////////
  while(1){   //endless loop over all events receiving
    // printf("********************** endless loop: %i\n", counter);
    //If one pro buffer is full->scan the whole buffer without scaling, else print status of buffer
    for(int i=0;i<MAXFEBNR;i++){
      if(ev_counter_mac[i]>(4*EVSPERFEB)){  //test if there is an overflow in the receiving buffer
        //error++;
        printf("pro buffer scaned and reseted without scaling of %d...\n",i);
        shift_scale(i, 4*EVSPERFEB, 1e9);
        ready_to_fill=PROBUF_READY_TO_FILL;
        ev_counter_mac[i]=0;  //if one buffer is overload, it is scaled w/out scaling...
      }
      else if(ev_counter_mac[i]!=0){ //if everything if fine, print the number of events in the buffers
        //printf("fill status of %d: %d - %d\n",i,ev_counter_mac[i], ev_counter_scan[i]);
      }
    }
    //receive new data
    if(ready_to_fill==PROBUF_READY_TO_FILL){
      ready_to_fill=PROBUF_FILLING;
      if(counter_tot>size_ev) return 0;
      receive_data(subscriber);
    }
    //scale and scan the new data
    if(ready_to_fill==PROBUF_READY_TO_SCALE){
      ready_to_fill=PROBUF_SHIFT_SCALE;
      scale_buffer();   //in case more than one second is in the buffer
      scale_buffer();   //scale_buffer should then be used more than ones
      //scale_buffer();   //only one is needed for polltimes<1 sec
      // if (counter == 100) exit(0);
    }
    else{
      printf("pro buffer is in use... \n");
      //error++;
    }
    while(ready_to_fill!=PROBUF_READY_TO_SCALE && ready_to_fill!=PROBUF_READY_TO_FILL){
      //printf("waiting for probuffer %ld...\n", waiting_counter);
      //waiting_counter++;
    }
    //printf(" %ld/%ld errors/waitings, %ld 5er-sendings, %ld received hits, %ld prozessed hits\n",error, waiting_counter, sending_counter, received_hit_counter, prozessed_hit_counter);
    //printf("\n");
    counter++;
  }
  //colse all 0mq conections (this part will never be reached!!!)
  //zmq_close (publisher);
  //zmq_ctx_destroy (context_send);

  fclose(file_store);
  fclose(file_spills);
  //zmq_close (publisher2);
  //zmq_ctx_destroy (context_send2);
  //zmq_close (subscriber);
  //zmq_ctx_destroy (context_receive);
  return 0;
}

void usage(){      //print some hints if input is wrong
 printf("Connects to data stream from running febdrv at a given data socket and send (adds) data.\n Usage: ");
 printf("beam_filter_file <path to file> <path_to_store> run_mode\n");
 printf("example:  \"rawdata_top.bin  file_store.bin 1\n");
}

//receive a poll of hits from the modules//////////////////////////////////////////////////////
void receive_data(void * subscriber){
  // printf("========== receive data, %i\n", ev_counter_mac[94]);
  /*time_t t0,t1;
  int dt,dt0;*/
  //int event_counter=0;
  int mac;
  //initialize the variables to receive
  /*zmq_msg_t reply;
  zmq_msg_init (&reply);
  t0=time(NULL);
  dt=0; dt0=0;
  while(zmq_msg_recv (&reply, subscriber, ZMQ_DONTWAIT)==-1){
    dt=time(NULL)-t0;
    if(dt>2 && dt!=dt0){
      printf("No data from driver for %d seconds!\n",dt);
      dt0=dt;}
  };
  // receive a bunch of events and give the number of hits in there
  int byte=0; int maxlen=MAXFEBNR*EVSPERFEB+1;
  byte=zmq_msg_size (&reply);
  if(maxlen > byte) maxlen=byte;

  //copy the data into the buffer for receiving
  memcpy((uint8_t*)evbuf,(uint8_t*)zmq_msg_data(&reply),maxlen);
  int received_events=byte/EVLEN;
  //received_hit_counter+=received_events;
  //printf("received %d events\n", received_events);
  */
  for(int i=0;i<1000;i++){
    fread(&evbuf[i],sizeof(EVENT_t),1,data);
  }
  int received_events=1000;
  counter_tot+=received_events;
  if(counter_tot-counter_old>10000){
   printf("\rprocessed: %ld/%ld (%ld%%), stuckt events: %ld (%.2f permill), second assignement: same: %d, lost: %d\n", counter_tot, size_ev, 100*counter_tot/size_ev, stuck_event_counter, 1000*(float)stuck_event_counter/(counter_tot-1000), same_counter1,minus2_counter1);
    counter_old=counter_tot;
  }
  int reassign=0;//control number for changes in second assignement
  int check=0;//control number for print outs in second assignement
  int all_fine=0;//control number if the jump happened after a reference pulse

  for(int i=0; i<received_events; i++){

    // printf("At event %i, mac: %i , t1: %lu\n", i, evbuf[i].mac5, evbuf[i].ts1);
    int do_print = 0;

    mac = evbuf[i].mac5;

    if(evbuf[i].mac5==0xFFFF){    //reads the spezial event
      if (DEBUG) printf("\tThis is a special event, %i\n", i);
      // printf("\t\tStart s: %i, ms: %i, end s: %i, ms: %i, n events: %i \n", (int)refevent.start_s, (int)refevent.start_ms, (int)refevent.end_s, (int)refevent.end_ms, (int)refevent.nevsinpoll);
      memcpy(&refevent,&evbuf[i].mac5,sizeof(EOP_EVENT_t));
      //previous2_sec=previous_sec;
      previous_sec=act_time[0][MAXFEBNR];
      previous_ms=act_time[1][MAXFEBNR];
       //printf("End:	%d seconds	%d millisec\n",act_time[0][MAXFEBNR], act_time[1][MAXFEBNR]);
      act_time[0][MAXFEBNR]=(int)refevent.end_s;
      act_time[1][MAXFEBNR]=(int)refevent.end_ms;
      //prozessed_hit_counter++;
      //printf("Start:	%d seconds	%d millisec\n",(int)refevent.start_s, refevent.start_ms);
      //printf("\n");
      //printf("End:	%d seconds	%d millisec\n",(int)refevent.end_s, refevent.end_ms);
      //printf("\n");
    }
    else {
      // printf("event %i\tts0: %u\tts1: %u\tsec: %u \n", i, evbuf[i].ts0, evbuf[i].ts1, act_time[0][MAXFEBNR]);
      // if (evbuf[i].ts1 < 0) printf("event %i\tts0: %i\tts1: %i\n", i, evbuf[i].ts0, evbuf[i].ts1);
      //add the second to the events and copy the buffer into another
      // printf("\tThis is NOT a special event\n");
      // printf("\t\tFlags: %i\n", evbuf[i].flags);
      if ((0x8 & evbuf[i].flags) >> 3) {
        int CORR_MSEC = 35;
        int t1_s = act_time[0][mac]; //act_time[0][MAXFEBNR];
        int t1_ms = evbuf[i].ts0 / 1e6 - CORR_MSEC;
        if(t1_ms < 0) {
          t1_ms = t1_ms + 1000;
          // t1_s = t1_s - 1;
          // if (mac == 90) printf("->reduced\n");
        }
        if (mac == 95) { // just pick one for printing
          if (DEBUG) printf("\t\tThis is a T1 reference event --------------------------------------, mac = %i, flags = %i\n", evbuf[i].mac5, evbuf[i].flags);
          if (DEBUG) printf("\t\t                             s = %i, ms = %i, t0 = %lu, t1 = %lu\n", t1_s, t1_ms, evbuf[i].ts0, evbuf[i].ts1);
          if (DEBUG) do_print = 1;
          // if (t1_s > 1498249682) exit(-1);
        }
        // printf("mac5: %i\nflags: %i\nlostcpu: %i\nlostfpga: %i\nts0: %i\nts1: %i\n", evbuf[i].mac5, evbuf[i].flags, evbuf[i].lostcpu, evbuf[i].lostfpga, evbuf[i].ts0, evbuf[i].ts1);
      }
      if ((0x4 & evbuf[i].flags) >> 2) {
        if (DEBUG)  printf("\t\tThis is a T0 reference event --------------------------------------, pedestal mac = %i, adc[0] = %i \n", evbuf[i].mac5, evbuf[i].adc[0]);
      }
      // if (mac == 90 && act_time[0][mac] > 1498249389 && act_time[0][mac] <= 1498249395) printf("At event %i, mac: %i , t0: %lu t1: %lu\n", i, evbuf[i].mac5, evbuf[i].ts0, evbuf[i].ts1);


      check=0;
      reassign=0;

      // check for jump, excluding all ref_events
      if(ev_counter_mac[mac] > 0 &&
        (evbuf_pro[mac][ev_counter_mac[mac]-1].flags == 3 || // T0 and T1 ref signal are there
         evbuf_pro[mac][ev_counter_mac[mac]-1].flags == 1 || // Only T0 ref signal is there
         evbuf_pro[mac][ev_counter_mac[mac]-1].lostcpu == 99)) {
        // printf("\t\tCheck for jump\n");
        //printf("Here!!: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d\n", evbuf[i].mac5, evbuf[i].flags, evbuf[i].ts0,evbuf[i].ts1, act_time[0][mac]);

        if(abs(event_time_diff[mac]-(evbuf[i].ts0-evbuf[i].ts1)) > 500) { // the jump has to be bigger than 500
          if (DEBUG) printf("\t\t\tJump is bigger than 500\n");
          all_fine = 0;
          //printf("Here2!!: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d\n", evbuf[i].mac5, evbuf[i].flags, evbuf[i].ts0,evbuf[i].ts1, act_time[0][mac]);
          //check for what referent event was missed or if it was a stucked event...
          if(abs(event_time_diff[mac]-(evbuf[i].ts0-evbuf[i].ts1)-1e9)<30000){ //check if ts0_ref was missed 30us > dead time FEB
            evbuf[i].lostcpu=1;

            // Check if the previous event is a T0 reference event. If so, all fine.
            if(evbuf_pro[mac][ev_counter_mac[mac]-1].flags==5 || evbuf_pro[mac][ev_counter_mac[mac]-1].flags==7) all_fine=1; // It's T0 ref

            // If the previous event is not a T0 ref event:
            if(all_fine != 1) {

              if(previous_sec == act_time[0][MAXFEBNR]) {
                act_time[0][mac] = act_time[0][MAXFEBNR] + 1; // check if already poll of new second
              }
              else {
                act_time[0][mac] = act_time[0][MAXFEBNR];
              }

              if(act_time[0][mac] == previous_sec_mac[mac]) {
                reassign = 1;
                check = 2;
              }

              if(act_time[0][mac] == previous_sec_mac[mac] + 2) {
                //printf("1pre+2 %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
                //act_time[0][mac]--;
                reassign = 3;
                check = 2;
              }

              if(evbuf_pro[mac][ev_counter_mac[mac]-1].sec == act_time[0][mac] && reassign != 1) {
                same_counter1++;
                check = 1;
              }

              if(abs(evbuf_pro[mac][ev_counter_mac[mac]-1].sec-act_time[0][mac]) > 1 && reassign != 2) {
                //printf("1pre+2.2 %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
                reassign = 3;
                act_time[0][mac]--;
                minus2_counter1++;
                check = 2;
              }

              ts0ref_counter[mac]++;
              //assign new event at the right place with ts0 = 1e9 (no scaling)
              evbuf_pro[mac][ev_counter_mac[mac]].sec=act_time[0][mac];
              evbuf_pro[mac][ev_counter_mac[mac]].ms=act_time[1][MAXFEBNR];
              evbuf_pro[mac][ev_counter_mac[mac]].mac5=mac;
              evbuf_pro[mac][ev_counter_mac[mac]].flags=7;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0=1e9;
              evbuf_pro[mac][ev_counter_mac[mac]].ts1=evbuf_pro[mac][ev_counter_mac[mac]-1].ts1+(1e9-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0);
              for(int j=0; j<32;j++) evbuf_pro[mac][ev_counter_mac[mac]].adc[j]=0;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0_scaled=0;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0_ref=0;
              evbuf_pro[mac][ev_counter_mac[mac]].lostcpu=0;
              evbuf_pro[mac][ev_counter_mac[mac]].lostfpga=0;
              if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T0 REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
              if (DEBUG) printf("Second: %i, ms: %i, mac: %i\n", evbuf_pro[mac][ev_counter_mac[mac]].sec, evbuf_pro[mac][ev_counter_mac[mac]].ms, mac);
              if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T0 REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
              //printf("Found TS0_ref!!! new event: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d\n", evbuf_pro[mac][ev_counter_mac[mac]].mac5, evbuf_pro[mac][ev_counter_mac[mac]].flags, evbuf_pro[mac][ev_counter_mac[mac]].ts0,evbuf_pro[mac][ev_counter_mac[mac]].ts1,evbuf_pro[mac][ev_counter_mac[mac]].sec);
              ev_counter_mac[mac]++;
            }
          }
          else if(abs((event_time_diff_old[mac]-(evbuf[i].ts0-evbuf[i].ts1)-1e9))<30000){ //check if ts0_ref was missed 30us > dead time FEB
            evbuf[i].lostcpu=2;
            //if(evbuf_pro[mac][ev_counter_mac[mac]-1].flags==5 || evbuf_pro[mac][ev_counter_mac[mac]-1].flags==7) all_fine=1;
            if(all_fine!=1){
              if(previous_sec==act_time[0][MAXFEBNR]) act_time[0][mac]=act_time[0][MAXFEBNR]+1; //check if already poll of new second
              else act_time[0][mac]=act_time[0][MAXFEBNR];
              if(act_time[0][mac]==previous_sec_mac[mac]){reassign=1; check=2;}
              if(act_time[0][mac]==previous_sec_mac[mac]+2){
                //printf("2pre+2 %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
                //act_time[0][mac]--;
                reassign=3;
                check=2;
              }
              if(evbuf_pro[mac][ev_counter_mac[mac]-1].sec==act_time[0][mac] && reassign!=1) {same_counter1++;check=1;}
              if(abs(evbuf_pro[mac][ev_counter_mac[mac]-1].sec-act_time[0][mac])>1&& reassign!=2) {
                //printf("2pre+2.2 %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
                reassign=3;
                act_time[0][mac]--;
                minus2_counter1++;
                check=2;
              }
              ts0ref_counter[mac]++;
              //assign new event at the right place with ts0 = 1e9 (no scaling)
              evbuf_pro[mac][ev_counter_mac[mac]].sec=act_time[0][mac];
              evbuf_pro[mac][ev_counter_mac[mac]].ms=act_time[1][MAXFEBNR];
              evbuf_pro[mac][ev_counter_mac[mac]].mac5=mac;
              evbuf_pro[mac][ev_counter_mac[mac]].flags=7;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0=1e9;
              evbuf_pro[mac][ev_counter_mac[mac]].ts1=evbuf_pro[mac][ev_counter_mac[mac]-1].ts1+(1e9-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0);
              for(int j=0; j<32;j++) evbuf_pro[mac][ev_counter_mac[mac]].adc[j]=0;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0_scaled=0;
              evbuf_pro[mac][ev_counter_mac[mac]].ts0_ref=0;
              evbuf_pro[mac][ev_counter_mac[mac]].lostcpu=0;
              evbuf_pro[mac][ev_counter_mac[mac]].lostfpga=0;
              if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T0 REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
              if (DEBUG) printf("Second: %i, ms: %i, mac: %i\n", evbuf_pro[mac][ev_counter_mac[mac]].sec, evbuf_pro[mac][ev_counter_mac[mac]].ms, mac);
              if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T0 REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

              //printf("Found TS0_ref!!! new event: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d\n", evbuf_pro[mac][ev_counter_mac[mac]].mac5, evbuf_pro[mac][ev_counter_mac[mac]].flags, evbuf_pro[mac][ev_counter_mac[mac]].ts0,evbuf_pro[mac][ev_counter_mac[mac]].ts1,evbuf_pro[mac][ev_counter_mac[mac]].sec);
              ev_counter_mac[mac]++;
            }
          }
          else if((evbuf[i].ts1<(evbuf[i].ts0-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0+20)&& evbuf[i].ts0<(evbuf_pro[mac][ev_counter_mac[mac]-1].ts0+1e8))){ //check if ts1_ref
            evbuf[i].lostcpu=3;

            // Check if the previous event was a T1 ref event
            if(evbuf_pro[mac][ev_counter_mac[mac]-1].flags==10 || evbuf_pro[mac][ev_counter_mac[mac]-1].flags==11) all_fine=1;
            if(all_fine!=1){
              // 1. check if there is an event with wrong assigned flag
              if(abs(evbuf[i].ts0-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0-evbuf[i].ts1)<20) {
                evbuf_pro[mac][ev_counter_mac[mac]-1].flags=11;
              }
              //2. if no event is there add new event
              else if((evbuf[i].ts0-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0)<1e7) {
                evbuf_pro[mac][ev_counter_mac[mac]].sec=act_time[0][mac];
                evbuf_pro[mac][ev_counter_mac[mac]].ms=act_time[1][MAXFEBNR];
                evbuf_pro[mac][ev_counter_mac[mac]].mac5=mac;
                evbuf_pro[mac][ev_counter_mac[mac]].flags=11;//evbuf[i].flags | 0x100;
                evbuf_pro[mac][ev_counter_mac[mac]].ts0=evbuf[i].ts0-evbuf[i].ts1;
                evbuf_pro[mac][ev_counter_mac[mac]].ts1=evbuf_pro[mac][ev_counter_mac[mac]-1].ts1+(evbuf[i].ts0-evbuf[i].ts1-evbuf_pro[mac][ev_counter_mac[mac]-1].ts0);
                for(int j=0; j<32;j++) evbuf_pro[mac][ev_counter_mac[mac]].adc[j]=0;
                evbuf_pro[mac][ev_counter_mac[mac]].ts0_scaled=0;
                evbuf_pro[mac][ev_counter_mac[mac]].ts0_ref=4e9;    //not really used, marco: i am adding a flag here to remember this is a fake spill
                evbuf_pro[mac][ev_counter_mac[mac]].lostcpu=0;
                evbuf_pro[mac][ev_counter_mac[mac]].lostfpga=0;
                if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T1REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                if (DEBUG) printf("Second: %i, ms: %i, mac: %i\n", evbuf_pro[mac][ev_counter_mac[mac]].sec, evbuf_pro[mac][ev_counter_mac[mac]].ms, mac);
                if (DEBUG) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ADDING A NEW T1REF EVENT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                //printf("\nnew event: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d\n", evbuf_pro[mac][ev_counter_mac[mac]].mac5, evbuf_pro[mac][ev_counter_mac[mac]].flags, evbuf_pro[mac][ev_counter_mac[mac]].ts0,evbuf_pro[mac][ev_counter_mac[mac]].ts1,evbuf_pro[mac][ev_counter_mac[mac]].sec);
                ev_counter_mac[mac]++;
              }
              else {
                evbuf[i].lostcpu=99; stuck_event_counter++;
              }
            }
          }
          else {  //it has to be a stucked event. Count them for statistic, asign something special to it...
            evbuf[i].lostcpu=99;
            //mac=1;
            stuck_event_counter++;
          }
        }
        else{
          if(evbuf_pro[mac][ev_counter_mac[mac]-1].lostcpu==99) evbuf[i].lostcpu=10;
        }
        //event_time_diff[mac]=evbuf[i].ts0-evbuf[i].ts1; //calculating again the timedifference between ts0 and ts1, for jump searches in the beginning...
      }

      //fill the aktual event normal (if a jump happend, an event before is inserted if a ref was missed...)
      // If this is T0 ref event
      if((evbuf[i].flags == 7 || evbuf[i].flags == 5) && (evbuf[i].lostcpu !=99 && evbuf[i].lostcpu != 10)) {

        // if (mac == 90) printf("--------------------------------------------------> T0 ref, mac: %i\n", mac);

        previous_sec_mac2[mac] = previous_sec_mac[mac];
        previous_sec_mac[mac] = act_time[0][mac];

        if(previous_sec == act_time[0][MAXFEBNR]) {
          // if (mac == 90) printf("--------------------------------------------------> T0 ref 1\n");
          act_time[0][mac] = act_time[0][MAXFEBNR] + 1;
        } else {
          act_time[0][mac] = act_time[0][MAXFEBNR];
          // if (mac == 90) printf("--------------------------------------------------> T0 ref 2, %i\n", act_time[0][mac]);
        }

        if(act_time[0][mac] == previous_sec_mac[mac]) {
          // if (mac == 90) printf("--------------------------------------------------> T0 ref 3\n");
          reassign=1;
          check=2;
        }

        if(act_time[0][mac] == previous_sec_mac[mac] + 2) {
          // if (mac == 90) printf("--------------------------------------------------> T0 ref 4\n");
          if(act_time[1][MAXFEBNR] > 200) {
            // if (mac == 90) printf("--------------------------------------------------> T0 ref 4a\n");
            act_time[0][mac]--;
          }
          reassign=2;
          check=2;
        }
        //act_time[0][mac]=act_time[0][MAXFEBNR]+1;  // || evbuf[i].ts0<=(act_time[1][mac])
        ts0ref_counter[mac]++;
        if(evbuf_pro[mac][ev_counter_mac[mac]-1].sec==act_time[0][mac] && reassign!=1){
          same_counter1++;
          //printf("same %d, %d, %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac],evbuf_pro[mac][ev_counter_mac[mac]-1].sec,evbuf_pro[mac][ev_counter_mac[mac]-1].ts0);
          check=1;
        }
        if(abs(evbuf_pro[mac][ev_counter_mac[mac]-1].sec-act_time[0][mac])>1&& reassign!=2) {
          //printf("pre+2.2 %d, %d, %d\n",previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
          reassign=2;
          act_time[0][mac]--;
          minus2_counter1++;
          check=2;
        }
        //act_time[1][mac]=act_time[1][MAXFEBNR];
        //printf("ts0: %d, ts1: %d\n", first, second);
      }
      if((evbuf[i].flags==7 || evbuf[i].flags==5)&&(evbuf[i].lostcpu==99||evbuf[i].lostcpu==10)) ts0ref_counter[mac]++;

      // printf("\t\tFilling evbuf_pro, reassign is %i.\n", reassign);
      evbuf_pro[mac][ev_counter_mac[mac]].sec=act_time[0][mac];
      evbuf_pro[mac][ev_counter_mac[mac]].ms=act_time[1][MAXFEBNR];
      evbuf_pro[mac][ev_counter_mac[mac]].mac5=mac;
      evbuf_pro[mac][ev_counter_mac[mac]].flags=evbuf[i].flags;
      evbuf_pro[mac][ev_counter_mac[mac]].ts0=evbuf[i].ts0;
      evbuf_pro[mac][ev_counter_mac[mac]].ts1=evbuf[i].ts1;
      for(int j=0; j<32;j++) evbuf_pro[mac][ev_counter_mac[mac]].adc[j]=evbuf[i].adc[j];
      evbuf_pro[mac][ev_counter_mac[mac]].ts0_scaled=0;
      evbuf_pro[mac][ev_counter_mac[mac]].ts0_ref=0;    //not really used
      evbuf_pro[mac][ev_counter_mac[mac]].lostcpu=evbuf[i].lostcpu;
      evbuf_pro[mac][ev_counter_mac[mac]].lostfpga=evbuf[i].lostfpga;
      //printf("End:   sec  %d ms: %d\n",(int)refevent.end_s, refevent.end_ms);
      //printf("event: sec: %d ms: %d ....%d mac5,  ts0: %d \n", evbuf_pro[mac][ev_counter_mac[mac]].sec, evbuf_pro[mac][ev_counter_mac[mac]].ms,evbuf_pro[mac][ev_counter_mac[mac]].mac5, evbuf_pro[mac][ev_counter_mac[mac]].ts0);
      ev_counter_mac[mac]++;
      if(reassign == 1) {
        for(int z=0; z<ev_counter_mac[mac]-1;z++){
          evbuf_pro[mac][z].sec=evbuf_pro[mac][z].sec-1;
        }
      }
      if(reassign == 2) {
        if(act_time[1][MAXFEBNR]<200) {
          for(int z=0; z<ev_counter_mac[mac]-1;z++) {
            evbuf_pro[mac][z].sec=evbuf_pro[mac][z].sec+1;
          }
        if(evbuf_pro[mac][ev_counter_mac[mac]].flags!=5 || evbuf_pro[mac][ev_counter_mac[mac]].flags!=7) evbuf_pro[mac][ev_counter_mac[mac]].sec+=1;
        }
		    // else act_time[0][mac]--;
      }

      if(reassign == 3) {
        for(int z=0; z<ev_counter_mac[mac]-1;z++) {
         // evbuf_pro[mac][z].sec=evbuf_pro[mac][z].sec+1;
        }
        // if(evbuf_pro[mac][ev_counter_mac[mac]].flags!=5 || evbuf_pro[mac][ev_counter_mac[mac]].flags!=7) evbuf_pro[mac][ev_counter_mac[mac]].sec+=1;
      }

      if (DEBUG) if (do_print) printf("Second is %i, reassing is %i\n", evbuf_pro[mac][ev_counter_mac[mac]-1].sec, reassign);

      if(check==10 || check==20){ //just printouts if needed... Row of events read in...
        printf("check: %d\n", check);
        printf("pre: %d: %d, %d, %d\n",reassign,previous_sec_mac2[mac], previous_sec_mac[mac], act_time[0][mac]);
        int sum=0, sum1=0, sum2=0, sum3=0, sum4=0;
        for(int amp=0; amp<32;amp++) {
          sum+=evbuf[i].adc[amp];
          sum1+=evbuf_pro[mac][ev_counter_mac[mac]].adc[amp];
          sum3+=evbuf_pro[mac][ev_counter_mac[mac]-1].adc[amp];
          sum4+=evbuf_pro[mac][ev_counter_mac[mac]-2].adc[amp];
          sum2+=evbuf[i+1].adc[amp];
        }
        // printf("after:  previous sec: %d, this second: %d\n",previous_sec,act_time[0][MAXFEBNR]);
        for(int z=0; z<5;z++){
          printf("jump: %3d: mac: %2d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d  dt: %d, lostcpu: %d\n", z,evbuf_pro[mac][z].mac5, evbuf_pro[mac][z].flags, evbuf_pro[mac][z].ts0,evbuf_pro[mac][z].ts1,evbuf_pro[mac][z].sec, evbuf_pro[mac][z].ts0-evbuf_pro[mac][z].ts1, evbuf_pro[mac][z].lostcpu );
          // evbuf_pro[mac][z].sec--;
        }
        for(int z=ev_counter_mac[mac]-5; z<ev_counter_mac[mac];z++){
          printf("jump: %3d: mac: %2d, flags: %2d, ts0: %10d, ts1: %10d, sec: %10d  dt: %d, lostcpu: %d\n", z,evbuf_pro[mac][z].mac5, evbuf_pro[mac][z].flags, evbuf_pro[mac][z].ts0,evbuf_pro[mac][z].ts1,evbuf_pro[mac][z].sec, evbuf_pro[mac][z].ts0-evbuf_pro[mac][z].ts1, evbuf_pro[mac][z].lostcpu );
          // evbuf_pro[mac][z].sec--;
        }
        printf("jump+1: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sum adc: %5d sec: %10d, dt: %d\n", evbuf[i+1].mac5, evbuf[i+1].flags, evbuf[i+1].ts0,evbuf[i+1].ts1, sum2,0, evbuf[i+1].ts0-evbuf[i+1].ts1);
        printf("jump+2: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sum adc: %5d sec: %10d, dt: %d\n", evbuf[i+2].mac5, evbuf[i+2].flags, evbuf[i+2].ts0,evbuf[i+2].ts1, sum2,0, evbuf[i+2].ts0-evbuf[i+2].ts1);
        printf("jump+3: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sum adc: %5d sec: %10d, dt: %d\n", evbuf[i+3].mac5, evbuf[i+3].flags, evbuf[i+3].ts0,evbuf[i+3].ts1, sum2,0,evbuf[i+3].ts0-evbuf[i+3].ts1);
        printf("jump+4: mac: %d, flags: %2d, ts0: %10d, ts1: %10d, sum adc: %5d sec: %10d, dt: %d\n", evbuf[i+4].mac5, evbuf[i+4].flags, evbuf[i+4].ts0,evbuf[i+4].ts1, sum2,0,evbuf[i+4].ts0-evbuf[i+4].ts1);
        printf("\n");
      }
      if(evbuf[i].flags!=99) { //if the last one was not a stucked event, calculate the ts0-ts1 new...
        if(abs(event_time_diff[mac]-event_time_diff_old[mac])>50) event_time_diff_old[mac]=event_time_diff[mac];
        event_time_diff[mac]=evbuf[i].ts0-evbuf[i].ts1;
      }
    }
    //fprintf(text,"%d %d %d %d %d\n", evbuf[i].mac5, evbuf[i].flags, evbuf[i].ts0, evbuf[i].adc[1], evbuf_pro[evbuf[i].mac5][ev_counter_mac[evbuf[i].mac5]-1].sec);
  }
  //zmq_msg_close (&reply);
  ready_to_fill=PROBUF_READY_TO_SCALE;
  // printf("Done receive_data, %i\n", ev_counter_mac[94]);
}

//searches referent events (produced via PPS) and process them further/////////////////////////////
void scale_buffer(){
  // printf("scale_buffer starts\n");
  // printf("scale all buffers\n");
  for(int j = 0; j < MAXFEBNR; j++) { //loop over all planes
   for(int i = 0; i < ev_counter_mac[j]; i++) { // loop over the number of events in one plane

    if ((0x4 & evbuf_pro[j][i].flags) >> 2) {
      // printf("\t*** This is a T0 reference event %i %i --------------------------------------\n", j, i);
    }

    // 5 is 101 and 7 is 111, so we have T0 ref signal, and this is a T0 ref event
    if(evbuf_pro[j][i].flags==5 || evbuf_pro[j][i].flags==7){ //search a time ref event
      // printf("Time t0 ref event: sec %d, ts0 %d, ts0_ref %d \n", evbuf_pro[j][i].sec, evbuf_pro[j][i].ts0, evbuf_pro[j][i].ts0_ref);
      if(ready_to_scan==SCANBUF_READY_TO_FILL){
        order_buffer[j].sec=evbuf_pro[j][i].sec;
        order_buffer[j].ref_nr=i;
        order_buffer[j].ts0_ref=evbuf_pro[j][i].ts0;
        order_buffer[j].flags=0;
        //printf("%d %d %d %d %d %d\n", evbuf_pro[j][i].mac5, evbuf_pro[j][i].flags,evbuf_pro[j][i].ts0, evbuf_pro[j][i].ts1, evbuf_pro[j][i].adc[0],evbuf_pro[j][i].sec);
      }
      else {
        printf("Buffer not ready to scan!!\n");
        // error++;
      }
      // printf("break!\n");
      i=ev_counter_mac[j]; // to exit the for loop
      break;
    }
   }
  }
  //the following part looks that the buffers with sec<sec_max is scaned first/////

  // Find the T0 reference event max second over all planes
  int sec_max=0;
  for(int j=0;j<MAXFEBNR+1;j++){  //find max_sec
    if(order_buffer[j].flags==0 && order_buffer[j].sec>sec_max) {
      sec_max=order_buffer[j].sec;
    }
  }

  //Scan the buffer first with second < max_second in the buffer
  for(int j=0;j<MAXFEBNR+1;j++){  //scale/scan all seconds < max_sec
    if(order_buffer[j].flags==0 && order_buffer[j].sec <sec_max){
      ready_to_scan=SCANBUF_SCANNING;
      if((sec_max-order_buffer[j].sec)>1) {
        printf("\nsec1: %d, sec_max: %d\n",order_buffer[j].sec,sec_max);
      }
      shift_scale(j, order_buffer[j].ref_nr, order_buffer[j].ts0_ref);
      order_buffer[j].ts0_ref=0;
      order_buffer[j].flags=1;
    }
  }
  for(int j=0;j<MAXFEBNR+1;j++){  //scan seconds with max_sec
   if(order_buffer[j].flags==0){
     ready_to_scan=SCANBUF_SCANNING;
     shift_scale(j,order_buffer[j].ref_nr,order_buffer[j].ts0_ref);
     order_buffer[j].ts0_ref=0;
     order_buffer[j].flags=1;
   }
  }
  ready_to_fill=PROBUF_READY_TO_FILL;
  // printf("AAA scale_buffer ends\n");
  // exit(0);
}

//if a hole second is in the processing buffer the hits are copyed and scaled for scanning//////////////////////////////////////
// All the events are in the evbuf_pro buffer. Here, all the events up to the first T0 ref event
// are copied to the evbuf_scan buffer, and then deleted from evbuf_pro
void shift_scale(int mac, int ref_nr, int ts0_ref){ //scale all events and store them in the scan buffer
  // printf("shift_scale starts for mac %i\n", mac);
 // if (mac == 94) printf("mac 94, ref_nr: %i\n", ref_nr);
 // if (mac == 94) printf("In shift_scale start, ev_counter_mac: %i, ev_counter_scan: %i\n", ev_counter_mac[94], ev_counter_scan[94]);
  // printf("\twill read hits until entry %i (total is %i), number_ms is %i, ts0_ref is %i\n", ref_nr, ev_counter_mac[mac], number_ms[mac], ts0_ref);
  for(int i=0; i< number_ms[mac];i++){
    evbuf_scan[mac][i]=evbuf_scan[mac][ev_counter_scan[mac]-number_ms[mac]+i];
  }
  for(int i=0;i<ref_nr;i++){ //loop over all hits of a plane
   evbuf_scan[mac][i+number_ms[mac]]=evbuf_pro[mac][i];    //shift the hit from the pro to the scan buffer
   long scale0=((evbuf_pro[mac][i].ts0*1e9)/ts0_ref+0.5);  //calculate the scaling factor
   long scale1=((evbuf_pro[mac][i].ts1*1e9)/ts0_ref+0.5);  //calculate the scaling factor
   evbuf_scan[mac][i+number_ms[mac]].ts0_scaled=(int)scale0;    //store the scaled value
   evbuf_scan[mac][i+number_ms[mac]].ts1_scaled=(int)scale1;    //store the scaled value
   //evbuf_scan[mac][i+number_ms[mac]].ts0_scaled=evbuf_pro[mac][i].ts0;    //store the scaled value
   //evbuf_scan[mac][i+number_ms[mac]].ts1_scaled=evbuf_pro[mac][i].ts1;    //store the scaled value
   if (evbuf_scan[mac][i+number_ms[mac]].ts0_ref == 4e9) {
    if (evbuf_scan[mac][i+number_ms[mac]].sec > 1498249328 && evbuf_scan[mac][i+number_ms[mac]].sec < 1498249332)
      if (DEBUG) printf("---------------------- sec %i\n", evbuf_scan[mac][i+number_ms[mac]].sec);
   }
   if (evbuf_scan[mac][i+number_ms[mac]].ts0_ref != 4e9) { // marco's addition
     evbuf_scan[mac][i+number_ms[mac]].ts0_ref=ts0_ref;
   }
   //fprintf(text,"%d %d %d %d %d %d\n", evbuf_scan[mac][i].mac5, evbuf_scan[mac][i].flags,evbuf_scan[mac][i].ts0, evbuf_scan[mac][i].ts0_scaled, evbuf_scan[mac][i].adc[1],evbuf_scan[mac][i].sec );
 }
  // just for beam filtering...
	if(run_mode==FILTER_MODE){  //generate a special event with the additional information (e.g seconds)
    ts0_ref_event[0].mac5=evbuf_pro[mac][ref_nr].mac5;
    ts0_ref_event[0].flags=evbuf_pro[mac][ref_nr].flags;
    ts0_ref_event[0].lostcpu=evbuf_pro[mac][ref_nr].lostcpu;
    ts0_ref_event[0].lostfpga=evbuf_pro[mac][ref_nr].lostfpga;
    ts0_ref_event[0].ts0=evbuf_pro[mac][ref_nr].ts0;
    ts0_ref_event[0].ts1=evbuf_pro[mac][ref_nr].ts1;
    for(int amp=0; amp<32; amp++) ts0_ref_event[0].adc[amp]=evbuf_pro[mac][ref_nr].adc[amp];
    if(evbuf_pro[mac][ref_nr].flags!=7 && evbuf_pro[mac][ref_nr].flags!=5){
      printf("\n Error\n");
      for(int i=-2;i<5;i++){
        printf("%d %d %d %d %d %d\n", evbuf_pro[mac][ref_nr+i].mac5, evbuf_pro[mac][ref_nr+i].flags,evbuf_pro[mac][ref_nr+i].ts0, evbuf_pro[mac][ref_nr+i].ts1, evbuf_pro[mac][ref_nr+i].adc[0],evbuf_pro[mac][ref_nr+i].sec);
      }
      printf("Error\n");
    }
    ts0_ref_event[1].mac5=0xFFFF;
    ts0_ref_event[1].flags=1;
    ts0_ref_event[1].lostcpu=999;
    ts0_ref_event[1].lostfpga=0;
    ts0_ref_event[1].ts0=evbuf_pro[mac][ref_nr].sec;
    ts0_ref_event[1].ts1=evbuf_pro[mac][ref_nr].ms;
    for(int amp=0; amp<32; amp++) ts0_ref_event[1].adc[amp]=0;
	}
  //up to here...

 for(int i=ref_nr+1;i<ev_counter_mac[mac];i++){ //shift the rest to the beginnig of the pro buffer
  evbuf_pro[mac][i-ref_nr-1]=evbuf_pro[mac][i];
 }
 ev_counter_mac[mac]=ev_counter_mac[mac]-ref_nr-1;  //set the hit counter of the pro buffer
 ev_counter_scan[mac]=ref_nr+number_ms[mac]; //set the hit counter of the scan buffer
 // if (mac == 94) printf("In shift_scale, ev_counter_mac: %i, ev_counter_scan: %i\n", ev_counter_mac[94], ev_counter_scan[94]);
 if(run_mode==FILTER_MODE || run_mode==FILTER_PAIR_MODE) { //filters first only beam events and then eventuall looking  for pairs
	 filter_buffer(mac);
	 scan_buffer_filter(mac);
 }
	if(run_mode==PAIR_MODE) {//just looking for pairs
	 scan_buffer(mac);
 }
  // printf("shift_scale ends for mac %i\n", mac);
}

void scan_buffer_filter(int mac){  //scan over all events of one plane over one sec and search all possible coincidences
  // printf("scan_buffer_filter starts for mac %i\n", mac);
  //printf("scan feb nr %d\n", mac);
  last2_ms[mac]=0;

  // Get the second of the last event in the scan buffer
  int second = evbuf_scan[mac][ev_counter_scan[mac]-1].sec;
  // if (mac == 94) printf("scan_buffer_filter ----> second: %i\n",  evbuf_scan[mac][ev_counter_scan[mac]-1].sec);

  // ex: 1498249323 % 100 = 23
  if(ts1ref_second[second%100]!=second){
    ts1ref_counter[second%100]=0;
    ts1ref_second[second%100]=second;
  }
  //if(previous_sec_scan[mac]==second) printf("\nequal seconds in %d: presecond: %d, this second: %d\n", mac,previous_sec_scan[mac], second);
  //if(second-previous_sec_scan[mac]>1) printf("\nlost second in %d: presecond: %d, this second: %d\n", mac,previous_sec_scan[mac], second);
  previous_sec_scan[mac]=second;
  int filter_counter=0;
  //int last_ts1_ref=0;
  //int second=evbuf_scan[mac][number_ms[mac]].sec;
  //printf("mac: %d, flag: %d, ts0: %d, sec: %d, tot events: %d\n", evbuf_scan[mac][0].mac5,evbuf_scan[mac][0].flags,evbuf_scan[mac][0].ts0,evbuf_scan[mac][0].sec, ev_counter_scan[mac]);
  //printf("last1_ms[mac]: mac: %d, flag: %d, ts0: %d, sec: %d\n", evbuf_scan[mac][last1_ms[mac]].mac5,evbuf_scan[mac][last1_ms[mac]].flags,evbuf_scan[mac][last1_ms[mac]].ts0,evbuf_scan[mac][last1_ms[mac]].sec);
  //printf("\n");
  // if (mac == 94) printf("scan_buffer_filter ----> last1_ms: %i\n",  last1_ms[mac]);
  for(int i=last1_ms[mac];i<ev_counter_scan[mac];i++){
    //copy events in buffer for filtering
    evbuf_filter[mac][filter_counter]=evbuf_scan[mac][i];
    filter_counter++;

    //set last2_ms
    if(evbuf_scan[mac][i].ts0>=(1e9-2*MSOVERLAP) && i>number_ms[mac]){
      if(last2_ms[mac]==0){
        last2_ms[mac]=i;
        //printf("last2: %d \n", last2_ms[mac]);
      }
    }
    //set last1_ms
    if((evbuf_scan[mac][i].ts0>=(1e9-MSOVERLAP) && i>number_ms[mac])||i==(ev_counter_scan[mac]-1)){
      if(last2_ms[mac]==0) last2_ms[mac]=i;
      last1_ms[mac]=i;
      number_ms[mac]=ev_counter_scan[mac]-last2_ms[mac];
      last1_ms[mac]=last1_ms[mac]-last2_ms[mac];
      //printf("last1: %d \n", last1_ms[mac]);
      break;
    }
    // marco: 10 is 1010 and 11 is 1011, so we have t1 ref event and t1 signal is present.
    if(evbuf_scan[mac][i].flags==10 || evbuf_scan[mac][i].flags==11){
       //printf("mac: %d, flag: %d, ts0: %d, ts1 %d sec: %d, tot events: %d\n", evbuf_scan[mac][i].mac5,evbuf_scan[mac][i].flags,evbuf_scan[mac][i].ts0,evbuf_scan[mac][i].ts1,evbuf_scan[mac][i].sec, ev_counter_scan[mac]);
      // Let's check if this is a new T1 ref event, or if we have encoutered this before
      int new=1;
      for(int j=0;j<ts1ref_counter[second%100];j++){
        if(abs(evbuf_scan[mac][i].ts0-ts1ref_buffer[second%100][j])<20000) new++;
        //else printf("abs: %d ts1: %d second %d evsec: %d\n",abs(evbuf_scan[mac][i].ts1-ts1ref_buffer[second%100][j]),ts1ref_buffer[second%100][j], ts1ref_second[second%100],evbuf_scan[mac][i].sec);
      }

      // If it is a new one, let's save it, also make sure that this not a fake T1 event
      // made before when looking for jumps. We used the ts0_ref as a flag for these
      // if (evbuf_scan[mac][i].ts0_ref == 4e9 && mac == 90) {
      //   int t1_ms = evbuf_scan[mac][i].ts0/ 1e6 - 35;
      //   printf("======================== s %i    ms %i\n", evbuf_pro[mac][ev_counter_mac[mac]].sec, t1_ms);
      //   exit(0);
      // }
      if(new == 1 && evbuf_scan[mac][i].ts0_ref != 4e9 && evbuf_scan[mac][i].lostcpu != 99){
        // printf("new ts1: %d, number: %d\n", evbuf_scan[mac][i].ts0, ts1ref_counter[second%100]);
        ts1ref_buffer[second%100][ts1ref_counter[second%100]]=evbuf_scan[mac][i].ts0;
        ts1ref_buffer_s[second%100][ts1ref_counter[second%100]]=evbuf_scan[mac][i].sec; // marco
        ts1ref_counter[second%100]++;
      }
    }
    //printf("event.sec= %d   second= %d,  i=%d\n", evbuf_scan[mac][i].sec, second,i);
    //number_ms[mac]=ev_counter_scan[mac]-last2_ms[mac];
  }
  ready_to_scan=SCANBUF_READY_TO_FILL;
  ev_counter_filter[mac]=filter_counter;
  // if (mac == 94) printf("scan_buffer_filter ----> ev_counter_filter[mac]: %i\n", ev_counter_filter[mac]);
  //printf("ts1 counter: %d\n",ts1ref_counter[second%100]);
  //printf("\n");
  store_data_ts0_buffer(mac);
  //prozessed_hit_counter+=ev_counter_scan[mac]+1;
  // printf("scan_buffer_filter ends for mac %i\n", mac);
}

// filter beam events/correct the ts1 to minus//////////////////////////////////////////////////////////////////////////////////////////////
void filter_buffer(int mac){
  // printf("filter_buffer starts for mac %i\n", mac);
  // printf("\tev_counter_filter %i, second %i\n", ev_counter_filter[mac], evbuf_filter[mac][ev_counter_filter[mac]-1].sec);
  //int MAXDIFFERENCE=(MAX_TIME_PASTBEAM-MAX_TIME_PREBEAM)/2;
  //int OFFSET=(MAX_TIME_PASTBEAM+MAX_TIME_PREBEAM)/2;
  int second = evbuf_filter[mac][ev_counter_filter[mac]-1].sec;
  // if (mac == 94) printf("filter_buffer ---------------> %i\n", ev_counter_filter[mac]);
  int beam_ev_counter=0, ts1_ref_counter=0;
  int buf_to_send;
  int ts1_ref_local[100];
  int ts1_ref_approved[100][2];
  int ts1_ref_approved_s[100][2]; //marco
  int approved_ts1ref=0;
  for(int i=0; i<ev_counter_filter[mac];i++){
    for(int j=0;j<ts1ref_counter[second%100];j++){
      // printf("Event %i T0 %lu  -  T1 %i T0 %lu\n", i, evbuf_filter[mac][i].ts0_scaled, j, ts1ref_buffer[second%100][j]);
      if(abs(evbuf_filter[mac][i].ts0_scaled+1e9*(evbuf_filter[mac][i].sec-second)-ts1ref_buffer[second%100][j])<200){
        // printf("--->Made it!, Event flag: %i\n", evbuf_filter[mac][i].flags);
        //check if ts1_ref is also an event without flags in this second
        nrwm1[mac]++;
        ts1_ref_approved[approved_ts1ref][1]=0; //0= not recovered
        ts1_ref_approved[approved_ts1ref][0]=ts1ref_buffer[second%100][j];
        ts1_ref_approved_s[approved_ts1ref][0]=ts1ref_buffer_s[second%100][j];
        if(evbuf_filter[mac][i].flags!=11) {
          evbuf_filter[mac][i].flags = evbuf_filter[mac][i].flags | 0x100;
          ts1_ref_approved[approved_ts1ref][1]=1;
          ts1_ref_approved_s[approved_ts1ref][1]=1;
        }//1= recovered
        else nrwm2[mac]++;
        approved_ts1ref++;
        // printf("Approved ts1 ref with t0 %i\n", ts1_ref_approved[approved_ts1ref][0]);
      }
    }
  }
  for(int j=0;j<approved_ts1ref;j++) {
    // marco
    int CORR_MSEC = 35;
    int t1_s = ts1_ref_approved_s[j][0];
    int t1_ms = ts1_ref_approved[j][0] / 1e6 - CORR_MSEC;
    if(t1_ms < 0) {
      t1_ms = t1_ms + 1000;
      t1_s = t1_s - 1;
    }
    fprintf(file_spills, "%i,%i,%i\n", t1_s, t1_ms, mac);

    // printf("... Analyzing a t1 reference event with a t0: %i, s: %i, ms: %i\n", ts1_ref_approved[j][0], t1_s, t1_ms);
    // marco
  }


  for(int i = 0; i < ev_counter_filter[mac]; i++){
    // printf("\tentered loop 2 -----------------------------------------------------------\n");

    int delta_seconds = evbuf_filter[mac][i].sec - second;

    uint32_t event_t0 = evbuf_filter[mac][i].ts0_scaled + 1e9 * delta_seconds;

    // printf("Analyzing a hit with a t0: %i, and a second difference of: %i, sec: %i, second: %i, ms: %i\n", evbuf_filter[mac][i].ts0_scaled, (evbuf_filter[mac][i].sec - second), evbuf_filter[mac][i].sec, second, evbuf_filter[mac][i].ms);

    for(int j=0;j<approved_ts1ref;j++){

      // printf("... Analyzing a t1 reference event with a t0: %i, s: %i, ms: %i\n", ts1_ref_approved[j][0], t1_s, t1_ms);// marco

      // if (((ts1_ref_approved[j][0] > event_t0)  && (ts1_ref_approved[j][0] - event_t0) < MAX_TIME_PREBEAM)) {
      //   if (evbuf_filter[mac][i].flags == 3) printf("Case 1\n");
      // }

      // if (((ts1_ref_approved[j][0] <= event_t0) && (event_t0 - ts1_ref_approved[j][0]) < MAX_TIME_PASTBEAM)) {
      //   if (evbuf_filter[mac][i].flags == 3) printf("******************************************************************************************************************************** Case 2\n");
      // }

      if(((ts1_ref_approved[j][0] > event_t0)  && (ts1_ref_approved[j][0] - event_t0) < MAX_TIME_PREBEAM) ||
         ((ts1_ref_approved[j][0] <= event_t0) && (event_t0 - ts1_ref_approved[j][0]) < MAX_TIME_PASTBEAM)){

        // printf("------> Made it!\n");
        // printf("t1: %i, event_time: %i, max_time_prebeam: %i\n", ts1_ref_approved[j][0], evbuf_filter[mac][i].ts0_scaled+1e9*(evbuf_filter[mac][i].sec-second));
        // printf("t0: %u, sec: %u, second: %u\n", evbuf_filter[mac][i].ts0_scaled);
        // printf("sec: %u\n", evbuf_filter[mac][i].sec);
        // printf("second: %i\n", second);
        evbuf_filter_scan[mac][beam_ev_counter]=evbuf_filter[mac][i];
        if(ts1_ref_approved[j][1]==1) evbuf_filter_scan[mac][beam_ev_counter].recover=1;
        if(event_t0 < ts1_ref_approved[j][0]){
          // If the event is before the T1 ref event time, subtract the T1 ref offset
          evbuf_filter_scan[mac][beam_ev_counter].ts1=4e9+ts1_ref_approved[j][0]-evbuf_filter[mac][i].ts0;
          // printf("------>  %lu\n", evbuf_filter_scan[mac][beam_ev_counter].ts1);
          // if (evbuf_filter[mac][i].flags == 3) printf("============================================================================>>>>>>>>>>>  %i   %i   %i\n", mac, (int)(evbuf_filter[mac][i].ts0 - ts1_ref_approved[j][0]), evbuf_filter[mac][i].flags);
        }
        else {
          // printf("------> Made it! %lu\n", evbuf_filter[mac][i].ts1);
          // if (evbuf_filter[mac][i].flags == 3) printf("============================================================================()()()()()()()()()()()()()()()()()()()()()()>>>>>>>>>>>  %i   %lu   %i\n", mac, evbuf_filter[mac][i].ts1, evbuf_filter[mac][i].flags);
          evbuf_filter_scan[mac][beam_ev_counter].ts1=evbuf_filter[mac][i].ts1;
        }
        evbuf_filter_scan[mac][beam_ev_counter].nrtrigger=nrwm1[mac];
        evbuf_filter_scan[mac][beam_ev_counter].nrtrigger_11=ts0ref_counter[mac];
        beam_ev_counter++;
        break;
      }
    }
    if(evbuf_filter[mac][i].flags==11 || evbuf_filter[mac][i].flags==10) {
      ts1_ref_local[ts1_ref_counter]=evbuf_filter[mac][i].ts0;
      ts1_ref_counter++;
    }
  }
  if(run_mode==FILTER_PAIR_MODE) {
    ev_counter_filter_scan[mac]=beam_ev_counter;
    scan_filter_buffer(mac);
  }


  if(run_mode==FILTER_MODE){
    for(int i=0; i<beam_ev_counter; i++){
      beam_ev[send_bufnr][i].mac5=evbuf_filter_scan[mac][i].mac5;
      beam_ev[send_bufnr][i].flags=evbuf_filter_scan[mac][i].flags;
      beam_ev[send_bufnr][i].lostcpu=evbuf_filter_scan[mac][i].lostcpu;
      beam_ev[send_bufnr][i].lostfpga=evbuf_filter_scan[mac][i].lostfpga;
      beam_ev[send_bufnr][i].ts0=evbuf_filter_scan[mac][i].ts0;
      beam_ev[send_bufnr][i].ts1=evbuf_filter_scan[mac][i].ts1;
      for(int amp=0; amp<32; amp++) beam_ev[send_bufnr][i].adc[amp]=evbuf_filter_scan[mac][i].adc[amp];
      beam_ev[send_bufnr][i].recover=evbuf_filter_scan[mac][i].recover;
      beam_ev[send_bufnr][i].nrtrigger=evbuf_filter_scan[mac][i].nrtrigger;
      beam_ev[send_bufnr][i].nrtrigger_11=evbuf_filter_scan[mac][i].nrtrigger_11;
      //evbuf_filter_scan[mac][i]

    }
    beam_ev[send_bufnr][beam_ev_counter].mac5=0xFFFF;
    beam_ev[send_bufnr][beam_ev_counter].flags=beam_ev_counter;
    beam_ev[send_bufnr][beam_ev_counter].lostcpu=ts1_ref_counter;
    beam_ev[send_bufnr][beam_ev_counter].lostfpga=approved_ts1ref;
    beam_ev[send_bufnr][beam_ev_counter].ts0=evbuf_filter[mac][ev_counter_filter[mac]-1].sec;
    beam_ev[send_bufnr][beam_ev_counter].ts1=evbuf_filter[mac][ev_counter_filter[mac]-1].ms;

    for(int amp=0;amp<32;amp++) beam_ev[send_bufnr][beam_ev_counter].adc[amp]=0;
    beam_ev_counter++;
    buf_to_send=send_bufnr;
    //send_coinc(buf_to_send, j);
    store_data(buf_to_send, beam_ev_counter);
    store_data_ts0(mac);
    send_bufnr=(send_bufnr+1)%10;
  }
  //usleep(WAIT);

  // printf("filter_buffer ends for mac %i\n", mac);
}

/*
int send_coinc(int bufnr, int found_coinc){    //send the data
  zmq_msg_t msg;
  //for(int i=0; i<found_coinc;i++){ printf("mac: %d, flags: %d, ts0: %d, ts1:  %d, adc0: %d\n", coincidence[bufnr][i].mac5, coincidence[bufnr][i].flags, coincidence[bufnr][i].ts0, coincidence[bufnr][i].ts1, coincidence[bufnr][i].adc[0]);}
  zmq_msg_init_data (&msg, coincidence[bufnr], EVLEN*found_coinc , free_bufer, (void *) bufnr);
  zmq_msg_send (&msg, publisher, ZMQ_DONTWAIT);
  zmq_msg_close (&msg);
  //printf("finish sending\n");
}

int send_ts0_ref(){    //send the data
  zmq_msg_t msg;
  //for(int i=0; i<2;i++){ printf("ts0:  mac: %d, flags: %d, ts0: %d, ts1:  %d, adc0: %d\n", ts0_ref_event[i].mac5, ts0_ref_event[i].flags, ts0_ref_event[i].ts0, ts0_ref_event[i].ts1, ts0_ref_event[i].adc[0]);}
  zmq_msg_init_data (&msg, ts0_ref_event, EVLEN*2 , free_bufer, (void *) 2);
  zmq_msg_send (&msg, publisher, ZMQ_DONTWAIT);
  zmq_msg_close (&msg);
  //printf("finish sending\n");
}

void free_bufer (void *data, void *hint){ //call back from ZMQ sent function, hint points to subbufer index
  coincidence[(int)hint][0].mac5=0;
  ready_to_send[(int)hint]=0;
  //printf("free bufnr %d\n", (int)hint);
}
*/
void store_data_ts0_buffer(int mac){
  ts0_ref_event_buffer[mac][0].mac5=ts0_ref_event[0].mac5;
  ts0_ref_event_buffer[mac][0].flags=ts0_ref_event[0].flags;
  ts0_ref_event_buffer[mac][0].ts0=ts0_ref_event[0].ts0;
  ts0_ref_event_buffer[mac][0].ts1=ts0_ref_event[0].ts1;
  for(int j=0; j<32;j++) ts0_ref_event_buffer[mac][0].adc[j]=0;
  ts0_ref_event_buffer[mac][0].lostcpu=ts0_ref_event[0].lostcpu;
  ts0_ref_event_buffer[mac][0].lostfpga=ts0_ref_event[0].lostfpga;
  ts0_ref_event_buffer[mac][0].recover=0;
  ts0_ref_event_buffer[mac][0].nrtrigger=nrwm1[mac];
  ts0_ref_event_buffer[mac][0].nrtrigger_11=nrwm2[mac];
  //second event
  ts0_ref_event_buffer[mac][1].mac5=ts0_ref_event[1].mac5;
  ts0_ref_event_buffer[mac][1].flags=ts0_ref_event[1].flags;
  ts0_ref_event_buffer[mac][1].ts0=ts0_ref_event[1].ts0;
  ts0_ref_event_buffer[mac][1].ts1=ts0_ref_event[1].ts1;
  for(int j=0; j<32;j++) ts0_ref_event_buffer[mac][1].adc[j]=0;
  ts0_ref_event_buffer[mac][1].lostcpu=ts0_ref_event[1].lostcpu;
  ts0_ref_event_buffer[mac][1].lostfpga=ts0_ref_event[1].lostfpga;
  ts0_ref_event_buffer[mac][1].recover=0;
  ts0_ref_event_buffer[mac][1].nrtrigger=nrwm1[mac];
  ts0_ref_event_buffer[mac][1].nrtrigger_11=nrwm2[mac];
}

void store_data_ts0(int mac){
  fwrite(ts0_ref_event_buffer[mac], sizeof(EVENT_t_send)*2, 1, file_store);
}

void store_data(int bufnr, int found_coinc){
  // printf("store_data\n");
  fwrite(beam_ev[bufnr], sizeof(EVENT_t_send)*found_coinc, 1, file_store);

}

//scanned all hits of one module and looks for time coincidences in the second of all other modules/////////////////////////////////////////
void scan_buffer(int mac){  //scan over all events of one plane over one sec and search all possible coincidences
  // printf("scan_buffer for mac %d\n", mac);
  long time1, time2;
  long delta;
  int coinc_counter=1;
  for(int i=0;i<ev_counter_scan[mac];i++){
    coinc_counter=1;
    time1=evbuf_scan[mac][i].ts0_scaled;
    //printf("time1= %ld\n",time1);
    for(int j=0;j<MAXFEBNR;j++){
        if(j!=mac && ev_counter_scan[j]!=0){
        for(int k=0;k<ev_counter_scan[j];k++){
         //time2=(evbuf_scan[j][k].sec-evbuf_scan[mac][i].sec)*1e9+evbuf_scan[j][k].ts0_scaled;
         time2=evbuf_scan[j][k].ts0_scaled;
         //printf("time2= %ld\n",time2);
         delta=time2-time1;

         if((abs(delta)<MAX_TIME_DIFFERENCE)&&(evbuf_scan[j][k].flags==3 && evbuf_scan[mac][i].flags==3)){
           while(ready_to_send[send_bufnr]){ printf("wait bufnr: %d writting!\n", send_bufnr);
                                            //waiting_counter++;
                                           }
           //printf("coincidence found: %ld delta\n",delta);
           //printf("mac %d, flag: %d, ts0: %d ts0_scaled %d, adc[1] %d, sec: %d\n", evbuf_scan[mac][i].mac5, evbuf_scan[mac][i].flags,evbuf_scan[mac][i].ts0, evbuf_scan[mac][i].ts0_scaled, evbuf_scan[mac][i].adc[1],evbuf_scan[mac][i].sec);
           //printf("mac %d, flag: %d, ts0 %d, ts0_scaled %d, adc[1] %d, sec: %d\n", evbuf_scan[j][k].mac5, evbuf_scan[j][k].flags,evbuf_scan[j][k].ts0, evbuf_scan[j][k].ts0_scaled, evbuf_scan[j][k].adc[1],evbuf_scan[j][k].sec);
           coincidence[send_bufnr][0].mac5=evbuf_scan[mac][i].mac5;
           coincidence[send_bufnr][0].flags=evbuf_scan[mac][i].flags;
           coincidence[send_bufnr][0].lostcpu=evbuf_scan[mac][i].lostcpu;
           coincidence[send_bufnr][0].lostfpga=evbuf_scan[mac][i].lostfpga;
           coincidence[send_bufnr][0].ts0=evbuf_scan[mac][i].ts0_scaled;
           coincidence[send_bufnr][0].ts1=evbuf_scan[mac][i].ts1;
           for(int amp=0;amp<32;amp++) coincidence[send_bufnr][0].adc[amp]=evbuf_scan[mac][i].adc[amp];
           coincidence[send_bufnr][0].recover=evbuf_scan[mac][i].recover;
           coincidence[send_bufnr][0].nrtrigger=evbuf_scan[mac][i].nrtrigger;
           coincidence[send_bufnr][0].nrtrigger_11=evbuf_scan[mac][i].nrtrigger_11;

           coincidence_sec[send_bufnr][0]=evbuf_scan[mac][i].sec;
           coincidence_ms[send_bufnr][0]=evbuf_scan[mac][i].ms;

           coincidence[send_bufnr][1].mac5=evbuf_scan[j][k].mac5;
           coincidence[send_bufnr][1].flags=evbuf_scan[j][k].flags;
           coincidence[send_bufnr][1].lostcpu=evbuf_scan[j][k].lostcpu;
           coincidence[send_bufnr][1].lostfpga=evbuf_scan[j][k].lostfpga;
           coincidence[send_bufnr][1].ts0=evbuf_scan[j][k].ts0_scaled;
           coincidence[send_bufnr][1].ts1=evbuf_scan[j][k].ts1;
           for(int amp=0;amp<32;amp++) coincidence[send_bufnr][1].adc[amp]=evbuf_scan[j][k].adc[amp];
           coincidence[send_bufnr][1].recover=evbuf_scan[j][k].recover;
           coincidence[send_bufnr][1].nrtrigger=evbuf_scan[j][k].nrtrigger;
           coincidence[send_bufnr][1].nrtrigger_11=evbuf_scan[j][k].nrtrigger_11;

           coincidence_sec[send_bufnr][1]=evbuf_scan[j][k].sec;
           coincidence_ms[send_bufnr][1]=evbuf_scan[j][k].ms;

           coincidence[send_bufnr][2].mac5=0xFFFF;
           coincidence[send_bufnr][2].flags=2;
           //coincidence[send_bufnr][2].lostcpu=nrwm1[mac];
           //coincidence[send_bufnr][2].lostfpga=nrwm2[mac];
           //coincidence[send_bufnr][2].ts0=find_min_sec(send_bufnr, 2);
           //coincidence[send_bufnr][2].ts1=find_min_ms(send_bufnr, 2);
           coincidence[send_bufnr][2].ts0=evbuf_scan[mac][i].sec;
           coincidence[send_bufnr][2].ts1=evbuf_scan[j][k].sec;
           for(int amp=0;amp<32;amp++) coincidence[send_bufnr][2].adc[amp]=0;
           coincidence[send_bufnr][2].adc[0]=evbuf_scan[mac][i].ms;
           coincidence[send_bufnr][2].adc[1]=evbuf_scan[j][k].ms;
           coincidence[send_bufnr][2].recover=abs(i-j);

           int buf_to_send=send_bufnr;
           store_data_pairs(buf_to_send, 3);
           //send_coinc2(buf_to_send, 3);
           //send_bufnr=(send_bufnr+1)%10;
           //usleep(WAIT);
           //coinc_counter++;
         }
        } //end loop through all events of feb j
       }
      }//end loop over all febs
 }
  ready_to_scan=SCANBUF_READY_TO_FILL;
  //prozessed_hit_counter+=ev_counter_scan[mac]+1;
}

int find_min_sec( int bufnr, int hit_number){ //find the smallest second number of all hits in an event
  int min=coincidence_sec[bufnr][0];
  for(int i=1;i<hit_number;i++){
    if(coincidence_sec[bufnr][i]<min) min=coincidence_sec[bufnr][i];
  }
  //printf("min= %d", min);
  return min;
}

int find_min_ms( int bufnr, int hit_number){ //find the smallest second number of all hits in an event
  int min=coincidence_ms[bufnr][0];
  for(int i=1;i<hit_number;i++){
    if(coincidence_ms[bufnr][i]<min || min==0) min=coincidence_ms[bufnr][i];
  }
  //printf("min= %d", min);
  return min;
}

int find_max_sec( int bufnr, int hit_number){ //find the bigest second number of all hits in an event
  int max=coincidence_sec[bufnr][0];
  for(int i=1;i<hit_number;i++){
    if(coincidence_sec[bufnr][i]>max) max=coincidence_sec[bufnr][i];
  }
  //printf("max= %d\n", max);
  return max;
}
void store_data_pairs(int bufnr, int found_coinc){
  // printf("store_data_pairs\n");
  fwrite(coincidence[bufnr], sizeof(EVENT_t_send)*found_coinc, 1, file_store);
  // exit(-1);
}


//scanned all hits of one module and looks for time coincidences in the second of all other modules/////////////////////////////////////////
void scan_filter_buffer(int mac){  //scan over all events of one plane over one sec and search all possible coincidences
  // printf("scan_filter_buffer starts for mac %i\n", mac);
  // printf("\tev_counter_filter_scan is %i\n", ev_counter_filter_scan[mac]);
  //printf("scan feb nr %d\n", mac);
  long time1, time2;
  long delta;
  int coinc_counter=1;
  // if (mac == 94) printf("scan_filter_buffer --------------> ev_counter_filter_scan: %i\n", ev_counter_filter_scan[mac]);
  for(int i=0;i<ev_counter_filter_scan[mac];i++) {
    coinc_counter=1;
    time1=evbuf_filter_scan[mac][i].ts0_scaled;
    // printf("time1= %ld\n",time1);
    for(int j=0;j<MAXFEBNR;j++){
      // printf("at feb %i\n", j);
      if(j!=mac){
        for(int k=0;k<ev_counter_filter_scan[j];k++) {
      // printf("at k %i\n", k);
          //time2=(evbuf_filter_scan[j][k].sec-evbuf_filter_scan[mac][i].sec)*1e9+evbuf_filter_scan[j][k].ts0_scaled;
          time2=evbuf_filter_scan[j][k].ts0_scaled;
          delta=time2-time1;
          // if (mac == 94) printf("time2= %ld, delta= %ld\n",time2, delta);

          // 3 is 11, so we have presence of T0 and T1
          if((abs(delta)<MAX_TIME_DIFFERENCE)&&(evbuf_filter_scan[j][k].flags==3 && evbuf_filter_scan[mac][i].flags==3)){
            // printf("---> !!!\n");
            while(ready_to_send[send_bufnr]) {
              printf("wait bufnr: %d writting!\n", send_bufnr);
              // waiting_counter++;
            }
            // printf("saving...");
            //printf("coincidence found: %ld delta\n",delta);
            //printf("mac %d, flag: %d, ts0: %d ts0_scaled %d, adc[1] %d, sec: %d\n", evbuf_filter_scan[mac][i].mac5, evbuf_filter_scan[mac][i].flags,evbuf_filter_scan[mac][i].ts0, evbuf_filter_scan[mac][i].ts0_scaled, evbuf_filter_scan[mac][i].adc[1],evbuf_filter_scan[mac][i].sec);
            //printf("mac %d, flag: %d, ts0 %d, ts0_scaled %d, adc[1] %d, sec: %d\n", evbuf_filter_scan[j][k].mac5, evbuf_filter_scan[j][k].flags,evbuf_filter_scan[j][k].ts0, evbuf_filter_scan[j][k].ts0_scaled, evbuf_filter_scan[j][k].adc[1],evbuf_filter_scan[j][k].sec);
            coincidence[send_bufnr][0].mac5=evbuf_filter_scan[mac][i].mac5;
            coincidence[send_bufnr][0].flags=evbuf_filter_scan[mac][i].flags;
            coincidence[send_bufnr][0].lostcpu=evbuf_filter_scan[mac][i].lostcpu;
            coincidence[send_bufnr][0].lostfpga=evbuf_filter_scan[mac][i].lostfpga;
            coincidence[send_bufnr][0].ts0=evbuf_filter_scan[mac][i].ts0_scaled;
            coincidence[send_bufnr][0].ts1=evbuf_filter_scan[mac][i].ts1;
            for(int amp=0;amp<32;amp++) coincidence[send_bufnr][0].adc[amp]=evbuf_filter_scan[mac][i].adc[amp];
            coincidence[send_bufnr][0].recover=evbuf_filter_scan[mac][i].recover;
            coincidence[send_bufnr][0].nrtrigger=evbuf_filter_scan[mac][i].nrtrigger;
            coincidence[send_bufnr][0].nrtrigger_11=evbuf_filter_scan[mac][i].nrtrigger_11;
            // if (coincidence[send_bufnr][0].ts1 < 0) printf("9999999999999999999>>>>>>> \tts1: %i\n", i, coincidence[send_bufnr][0].ts0, coincidence[send_bufnr][0].ts1);
            // printf("9999999999999999999>>>>>>> \t0 mac5: %u\n", coincidence[send_bufnr][0].mac5);
            // printf("9999999999999999999>>>>>>> \t0 ts1: %i\n", coincidence[send_bufnr][0].ts1);
            coincidence_sec[send_bufnr][0]=evbuf_filter_scan[mac][i].sec;
            coincidence_ms[send_bufnr][0]=evbuf_filter_scan[mac][i].ms;

            coincidence[send_bufnr][1].mac5=evbuf_filter_scan[j][k].mac5;
            coincidence[send_bufnr][1].flags=evbuf_filter_scan[j][k].flags;
            coincidence[send_bufnr][1].lostcpu=evbuf_filter_scan[j][k].lostcpu;
            coincidence[send_bufnr][1].lostfpga=evbuf_filter_scan[j][k].lostfpga;
            coincidence[send_bufnr][1].ts0=evbuf_filter_scan[j][k].ts0_scaled;
            coincidence[send_bufnr][1].ts1=evbuf_filter_scan[j][k].ts1;
            for(int amp=0;amp<32;amp++) coincidence[send_bufnr][1].adc[amp]=evbuf_filter_scan[j][k].adc[amp];
            coincidence[send_bufnr][1].recover=evbuf_filter_scan[j][k].recover;
            coincidence[send_bufnr][1].nrtrigger=evbuf_filter_scan[j][k].nrtrigger;
            coincidence[send_bufnr][1].nrtrigger_11=evbuf_filter_scan[j][k].nrtrigger_11;
            // printf("9999999999999999999>>>>>>> \t1 ts1: %i\n", coincidence[send_bufnr][1].ts1);

            coincidence_sec[send_bufnr][1]=evbuf_filter_scan[j][k].sec;
            coincidence_ms[send_bufnr][1]=evbuf_filter_scan[j][k].ms;

            coincidence[send_bufnr][2].mac5=0xFFFF;
            coincidence[send_bufnr][2].flags=2;
            //coincidence[send_bufnr][2].lostcpu=nrwm1[mac];
            //coincidence[send_bufnr][2].lostfpga=nrwm2[mac];
            //coincidence[send_bufnr][2].ts0=find_min_sec(send_bufnr, 2);
            //coincidence[send_bufnr][2].ts1=find_min_ms(send_bufnr, 2);
            coincidence[send_bufnr][2].ts0=evbuf_scan[mac][i].sec;
            coincidence[send_bufnr][2].ts1=evbuf_scan[j][k].sec;
            for(int amp=0;amp<32;amp++) coincidence[send_bufnr][2].adc[amp]=0;
            coincidence[send_bufnr][2].adc[0]=evbuf_scan[mac][i].ms;
            coincidence[send_bufnr][2].adc[1]=evbuf_scan[j][k].ms;
            coincidence[send_bufnr][2].recover=abs(i-j);
            // printf("9999999999999999999>>>>>>> \t2 ts1: %u\n", coincidence[send_bufnr][2].ts1);
            int buf_to_send=send_bufnr;
            if (DEBUG) printf("9999999999999999999>>>>>>> store_data_pairs wit macs %i  %i \n", mac, j);
            store_data_pairs(buf_to_send, 3);
            int bs = (int)(evbuf_filter_scan[mac][i].ts0_scaled/1e6);
            bs = bs -35;
            // printf("9999999999999999999>>>>>>> store_data_pairs, s: %i, ms: %i, c: %i \n", evbuf_scan[mac][i].sec, evbuf_scan[mac][i].ms, bs);
            // exit(0);
            //send_coinc2(buf_to_send, 3);
            //send_bufnr=(send_bufnr+1)%10;
            //usleep(WAIT);
            //coinc_counter++;
          }
        } //end loop through all events of feb j
      }
    }//end loop over all febs
  }
  ready_to_scan=SCANBUF_READY_TO_FILL;
  //prozessed_hit_counter+=ev_counter_filter_scan[mac]+1;
  // printf("scan_filter_buffer ends for mac %i\n", mac);
}




