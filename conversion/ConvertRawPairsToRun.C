#include <time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define T1DELAY 40000 // trigger delay in ns, set in HighLand Delay unit

#define DEBUG false

typedef struct {
  uint16_t mac5;
  uint16_t flags;
  uint16_t lostcpu;
  uint16_t lostfpga;
  uint32_t ts0;
  uint32_t ts1;
  uint16_t adc[32];
    //int s;
    //int ms;
    //int nrtrack;
    //int XY;
    //double tof;
    //int nhit;
  uint16_t recover;
  uint32_t nrtrigger;
  uint32_t nrtrigger_11;
} EVENT_t_pac;

EVENT_t_pac evbufr[4];

CRTRawhit evbuf[4];
CRTRawhit *hit;
//CRTRawhit *hit3;
CRT2Dhit *h2d;
CRTRun * run;
CRTRawhitBuffer *rhbuf;
CRTCalibs *cal;

FILE *data_file = 0;
//char buf[EVLEN]; //EVLEN is defined in libCRT headers
#define EVENTLEN 92  //Thomas pair event has 92 bytes
// #define EVENTLEN 88
// #define EVENTLEN 80


char buf[EVENTLEN];
char ofname[256];
char fname_full[256];
char potfname[256];
Int_t sec=0;
int32_t tts1;
void ConvertRawPairsToRun(const char *datadir="", const char *fname="", const char *outdir="")
{
  std::cout << "ConvertRawPairsToRun starts ..." << std::endl;
  if(strlen(fname)==0) return;
  rhbuf=new CRTRawhitBuffer();
  run=new CRTRun();
  sprintf(ofname, "%s/%s.libcrt.root", outdir, fname);
  run->CreateNewDataRun(ofname);
  hit=new CRTRawhit();
  std::cout << "Preparing calibs ..." << std::endl;
  cal=new CRTCalibs("SBND_CableDelay-V3.txt","SBND_CRTpositionsSiPM-nogaps.txt","SBND_ADCCalib-V4.txt");
  std::cout << "Calibs ready" << std::endl;
  cal->SetVersion(3);
  run->SetCalibs(cal);
  int evsread=0;
  sprintf(fname_full, "%s/%s", datadir, fname);
  data_file = fopen(fname_full, "r");
  if(data_file==0) return;
  else printf("Opened raw data file %s.\n", fname_full);
  int EndOfFile=0;
  int hitsinev=0;

  // POT
  sprintf(potfname, "%s/%s.spills.txt.totalpot.txt", datadir, fname);
  std::cout << "Using POT file: " << potfname << std::endl;
  FILE *potfile = fopen(potfname, "r");
  double totalpot = 0;
  double totalspills = 0;
  fscanf(potfile, "%lf", &totalpot);
  fscanf(potfile, "%lf", &totalspills);
  std::cout << "Total POT: " << totalpot << std::endl;
  std::cout << "Total SPILLS: " << totalspills << std::endl;

  run->AddPOTAndNSpills(totalpot, totalspills);
  // run->AddNSpills(totalspills);
  run->rheader->POT = totalpot;
  run->rheader->NSpills = totalspills;

  // // *** An additional, simplified tree, to put things in larsoft
  // char simple_ofname[256];
  // sprintf(simple_ofname, "%s/%s.flat.root", outdir, fname);
  // TFile* out_file = TFile::Open(simple_ofname,"RECREATE");
  // TTree* tree = new TTree("tree", "CRT Flat TTree");
  // UShort_t _mac5;
  // Int_t _ts0;
  // Int_t _ts1;
  // std::vector<UShort_t> _adc;
  // Int_t _s;
  // Int_t _ms;
  // tree->Branch("mac5", &_mac5, "mac5/s");
  // tree->Branch("ts0", &_ts0, "ts0/I");
  // tree->Branch("ts1", &_ts1, "ts1/s");
  // tree->Branch("adc", "std::vector<UShort_t>", &_adc);
  // tree->Branch("s", &_s, "s/s");
  // tree->Branch("ms", &_ms, "ms/s");

  // TTree* treeaux = new TTree("aux", "CRT Flat TTree Aux");
  // Double_t _pot, _spills;
  // treeaux->Branch("pot", &_pot, "pot/D");
  // treeaux->Branch("spills", &_spills, "spills/D");
  // _pot = totalpot;
  // _spills = totalspills;
  // treeaux->Fill();
  // // ***

  while(!EndOfFile)
 // while(evsread<20000)
  {
    if (DEBUG) { std::cout << std::endl; }
    if(fread(&(evbufr[0]), EVENTLEN, 1, data_file) <= 0) {EndOfFile=1; break;};
    if(fread(&(evbufr[1]), EVENTLEN, 1, data_file) <= 0) {EndOfFile=1; break;};
    if(fread(&(evbufr[2]), EVENTLEN, 1, data_file) <= 0) {EndOfFile=1; break;};
    // if (evbufr[0].ts1 < 0) std::cout << "------------ " << evbuf[0].ts1 << std::endl;

    for(int i=0;i<3;i++)
    {
      evbuf[i].mac5=evbufr[i].mac5;
      evbuf[i].flags=evbufr[i].flags;
      evbuf[i].lostcpu=evbufr[i].lostcpu;
      evbuf[i].lostfpga=evbufr[i].lostfpga;
      evbuf[i].ts0=evbufr[i].ts0;

      if(evbufr[i].ts1>4e9){ evbuf[i].ts1=-(evbufr[i].ts1-4e9);}
      else evbuf[i].ts1=evbufr[i].ts1;
      // printf("t1=%i t1=%u\n",evbufr[i].ts1,evbuf[i].ts1);

      memcpy(&(evbuf[i].adc[0]),&(evbufr[i].adc[0]),32*2);
    }

    if (DEBUG) {
      std::cout << "BEFORE: " << std::endl;
      for(int i=0; i<3; i++) {
        std::cout << "mac5   " << evbuf[i].mac5 << std::endl;
        std::cout << "s   " << evbuf[i].s << std::endl;
        std::cout << "ms  " << evbuf[i].ms << std::endl;
        std::cout << "ts0 " << evbuf[i].ts0 << std::endl;
        std::cout << "ts1 " << evbuf[i].ts1 << std::endl;
        for (auto adc : evbuf[i].adc) std::cout << adc << ", ";
        std::cout << std::endl << std::endl;
      }
    }

    evsread++;
    hitsinev++;

     if(evbuf[1].mac5<evbuf[0].mac5){
      evbuf[3].Copy(&(evbuf[1]));
      evbuf[1].Copy(&(evbuf[0]));
      evbuf[0].Copy(&(evbuf[3]));
    }


    evbuf[0].s   = evbuf[2].ts0;
    evbuf[0].ms  = evbuf[2].adc[0];
    evbuf[1].s   = evbuf[2].ts1;
    evbuf[1].ms  = evbuf[2].adc[1];

    evbuf[0].ts1 = evbuf[0].ts1 + T1DELAY;
    evbuf[1].ts1 = evbuf[1].ts1 + T1DELAY;

    if (DEBUG) {
      std::cout << "AFTER: " << std::endl;
      for(int i=0; i<3; i++) {
        std::cout << "mac5   " << evbuf[i].mac5 << std::endl;
        std::cout << "s   " << evbuf[i].s << std::endl;
        std::cout << "ms  " << evbuf[i].ms << std::endl;
        std::cout << "ts0 " << evbuf[i].ts0 << std::endl;
        std::cout << "ts1 " << evbuf[i].ts1 << std::endl;
        std::cout << "ADC: ";
        for (auto adc : evbuf[i].adc) std::cout << adc << ", ";
        std::cout << std::endl << std::endl;
      }
    }

    // // *** for simple tree
    // _mac5 = evbuf[0].mac5;
    // _ts0 = evbuf[0].ts0;
    // _ts1 = evbuf[0].ts1;
    // _s = evbuf[0].s;
    // _ms = evbuf[0].ms;
    // _adc.resize(32);
    // for (int i = 0; i < 32; i++) _adc[i] = evbuf[0].adc[i];
    // tree->Fill();

    // _mac5 = evbuf[1].mac5;
    // _ts0 = evbuf[1].ts0;
    // _ts1 = evbuf[1].ts1;
    // _s = evbuf[1].s;
    // _ms = evbuf[1].ms;
    // _adc.resize(32);
    // for (int i = 0; i < 32; i++) _adc[i] = evbuf[1].adc[i];
    // tree->Fill();
    // // *** for simple tree

    run->AddRawhit(&(evbuf[0]));
    run->AddRawhit(&(evbuf[1]));

    // std::cout << "Here 5" << std::endl;
    h2d = new CRT2Dhit(&(evbuf[0]), &(evbuf[1]), cal);
    if (DEBUG) {
      std::cout << "--> CRT2Dhit Timestamp seconds: " << h2d->s << std::endl;
      std::cout << "--> CRT2Dhit t0: " << h2d->t0 << std::endl;
      std::cout << "--> CRT2Dhit t1: " << h2d->t1 << std::endl;
      std::cout << "--> CRT2Dhit x: " << h2d->x << std::endl;
      std::cout << "--> CRT2Dhit y: " << h2d->y << std::endl;
      std::cout << "--> CRT2Dhit z: " << h2d->z << std::endl;
    }
    run->Add2Dhit(h2d);
    // std::cout << "Here 7" << std::endl;
    // if(evbufr[0].nrtrigger>run->rheader->Nt1refs[evbufr[0].mac5]) run->rheader->Nt1refs[evbufr[0].mac5]=evbufr[0].nrtrigger;
    // if(evbufr[1].nrtrigger>run->rheader->Nt1refs[evbufr[1].mac5]) run->rheader->Nt1refs[evbufr[1].mac5]=evbufr[1].nrtrigger;
    // if(evbufr[0].nrtrigger_11>run->rheader->Nt0refs[evbufr[0].mac5]) run->rheader->Nt0refs[evbufr[0].mac5]=evbufr[0].nrtrigger_11;
    // if(evbufr[1].nrtrigger_11>run->rheader->Nt0refs[evbufr[1].mac5]) run->rheader->Nt0refs[evbufr[1].mac5]=evbufr[1].nrtrigger_11;
    // std::cout << "Here 8" << std::endl;
    run->Fill();
    // std::cout << "Here 9" << std::endl;
    run->ClearEntry();
    // if (DEBUG) { if (hitsinev > 10) exit(0); }
    if(hitsinev%1000==0) printf("%d events processed..\n",evsread);
  }
  std::cout << "Done." << std::endl;
  run->BuildIndex("h2d.s","int(h2d.t0)");
  run->PrintSummary();
  run->Close();

  // // *** Simple tree
  // out_file->cd();
  // tree->Write("tree",TObject::kOverwrite);
  // treeaux->Write("treeaux ",TObject::kOverwrite);
  // out_file->Purge();
  // out_file->Close();
  // // *** Simple tree
}

/*
{
  std::cout<< "Entries:" <<  ptree->GetEntries()<< std::endl;
  ptree->BuildIndex("time");
  TTreeIndex *I=(TTreeIndex*)ptree->GetTreeIndex(); // get the tree index
  Long64_t* index=I->GetIndex(); //create an array of entries in sorted order
  TLeaf* time=ptree->GetBranch("event")->GetLeaf("time");
  for (int i=0;i<ptree->GetEntries();i++){
    ptree->GetEntry(index[i]);
    std::cout <<time->GetValue() << std::endl; //print the (hopefully sorted) time
  }
}
*/



