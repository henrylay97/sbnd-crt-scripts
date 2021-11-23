CRTRun * run;
Int_t entries;
Int_t sec, msec,isbnb;
Int_t hit_t1, hit_i, hit_s, hit_ms;
TFile * f2;
TTree *bnb_tree=0;
TTreeIndex* bnb_index=0;
Int_t firstentry,lastentry;
Int_t dt_s,dt_ms;
Double_t dt_pot;
Int_t delta_s, delta_ms;
Float_t mindms,minds;

TTree *ot;
TTree *flagt;
TFile *of;

#define CORR_MSEC 35

void correlate_with_bnb_database(const char *fcrt, const char *fbnb, const char *fout)
{
  if(strlen(fcrt)==0) {printf("CRT run: No input file name is given.\n");return;};
  if(strlen(fbnb)==0) {printf("BNB spill DB: No input file name is given.\n");return;};
  run=new CRTRun();
  entries=run->OpenExistingDataRun(fcrt);
  if(entries==0){printf("CRT run: Extracting CRTRun object is failed.\n");return;};

  f2=new TFile(fbnb);
  bnb_tree=(TTree*)(f2->Get("dt"));
  if(bnb_tree) printf("\nBNB spill DB tree is opened, %lld entries. Building index..\n\n",bnb_tree->GetEntries());
  bnb_tree->SetBranchAddress("s",&dt_s);
  bnb_tree->SetBranchAddress("ms",&dt_ms);
  bnb_tree->SetBranchAddress("pot",&dt_pot);
  bnb_tree->BuildIndex("s","ms");
  bnb_index=(TTreeIndex*)(bnb_tree->GetTreeIndex());
  if(!bnb_index) { printf("BNB spill DB tree has no index! Exiting.\n"); return; }

  of=new TFile(fout, "RECREATE");
  ot=new TTree("ot","Correlation tree");
  flagt=new TTree("flagt","BNB flag friend tree");
  ot->Branch("s",&dt_s,"s/I");
  ot->Branch("ms",&dt_ms,"ms/I");
  ot->Branch("ds",&delta_s,"ds/I");
  ot->Branch("dms",&delta_ms,"dms/I");
  ot->Branch("pot",&dt_pot,"pot/D");
  ot->Branch("isbnb",&isbnb,"isbnb/I");
  ot->Branch("hit_i",&hit_i,"hit_i/I");
  ot->Branch("hit_t1",&hit_t1,"hit_t1/I");
  ot->Branch("hit_s",&hit_s,"hit_s/I");
  ot->Branch("hit_ms",&hit_ms,"hit_ms/I");
  flagt->Branch("isbnb",&isbnb,"isbnb/I");
  flagt->Branch("hit_i",&hit_i,"hit_i/I");
  flagt->Branch("mindms",&mindms,"mindms/F");
  flagt->Branch("minds",&minds,"minds/F");



  printf("Looping over %d CRT hit entries..\n",entries);

  for(hit_i=0; hit_i<entries; hit_i++)
  {
   // run->GetEntrySortedByTime(i);

    // Get the 2D hit time in both seconds and milliseconds
    run->GetEntry(hit_i);
    sec=((CRT2Dhit*)(run->h2d->At(0)))->s;
    msec=int(((CRT2Dhit*)(run->h2d->At(0)))->t0/1e6)-CORR_MSEC;
    if(msec<0) {
      msec=msec+1000; sec=sec-1;
      printf("ms < 0 for entry %i \n", hit_i);
    }

    // int ms = ((CRTRawhit)(CRT2Dhit*)(run->h2d->At(0))->rhit[0])->ms;
    // printf("ms %i, %i \n", msec, ms);
    hit_s = sec;
    hit_ms = msec;
    hit_t1=((CRT2Dhit*)(run->h2d->At(0)))->t1;

    // Find the entries in the BNB tree that can be close to this
    // 2D hit in time.
    firstentry=bnb_index->GetEntryNumberWithBestIndex(sec-1,0); if(firstentry<0) firstentry=0;
    lastentry=bnb_index->GetEntryNumberWithBestIndex(sec+3,0); if(lastentry<0) lastentry=0;

    // Loop over these entries in the BNB tree and check
    // if there is one with the time close to the 2D hit
    isbnb=0;
    mindms=1e6; minds=1e6;
    for(int ren=firstentry; ren<=lastentry; ren++)
    {
      bnb_tree->GetEntry(ren);
      delta_s = sec - dt_s; // Time difference in seconds
      delta_ms = msec - dt_ms; // Time difference in ms

      // Find the minimum time difference
      if(abs(delta_ms) < mindms ) { mindms=abs(delta_ms); minds=delta_s;}
      // If the time difference is less then 3 ms and 2 seconds, this is a match
      if(abs(delta_ms)<3 && abs(delta_s)<2) isbnb=1;
      // In any case save all entries that are 5 second close
      if(abs(delta_s) < 5) {
        ot->Fill();
      }
      isbnb=0;
    }

    // If the time difference is less then 3 ms and 2 seconds, this is a match
    if(abs(mindms) < 3 && abs(minds)<2) {
      isbnb=1;
    } else {
      isbnb=0;
    }

    flagt->Fill();

    if(hit_i%10000==1) printf("CRT run entry %d s=%d ms=%d, looping over %d BNB spills..\n",hit_i,sec,msec,lastentry-firstentry);

  }
  ot->Write();
  flagt->Write();
  of->Close();
  f2->Close();
}
