CRTRawhit *hit;
CRTEvent *evt;
CRT2Dhit *e2d;
CRTRawhitBuffer *rhbuf;
CRTCalibs*  cal;
CRTRun * run;
CRTRun * run1;
CRTRun * run2;


void Merge(const char *fname1="", const char *fname2="",const char *ofname="")
{
  run=new CRTRun();
  run1=new CRTRun();
  run2=new CRTRun();
  if(strlen(fname1)==0) {printf("run1: No input file name is given.\n");return;};
  if(strlen(fname2)==0) {printf("run2: No input file name is given.\n");return;};
  if(strlen(ofname)==0) {printf("No output file name is given.\n");return;};
  if(run1->OpenExistingDataRun(fname1)==0){printf("run1: Extracting CRTRun object is failed.\n");return;};
  if(run2->OpenExistingDataRun(fname2)==0){printf("run2: Extracting CRTRun object is failed.\n");return;};

  run=new CRTRun();
  run->CreateNewDataRun(ofname);
  run->MergeSortedByTime(run1,run2);
  run->BuildIndex("h2d.s","int(h2d.t0)");
  run->PrintSummary();

  run->Close();
}




