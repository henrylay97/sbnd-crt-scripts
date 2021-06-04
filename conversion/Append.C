CRTRawhit *hit;
CRTEvent *evt;
CRT2Dhit *e2d;
CRTRawhitBuffer *rhbuf;
CRTCalibs*  cal;
CRTRun * run;
CRTRun * run1;
CRTRun * run2;


void  Append(const char *fname1="",const char *ofname="")
{
  run=new CRTRun();
  run1=new CRTRun();
  if(strlen(fname1)==0) {printf("run1: No input file name is given.\n");return;};
  if(strlen(ofname)==0) {printf("No output file name is given.\n");return;};
  if(run1->OpenExistingDataRun(fname1)==0){printf("run1: Extracting CRTRun object is failed.\n");return;};

  if(run->OpenExistingDataRun(ofname, "UPDATE")==0) {printf("Output run does not exist. Creating new one.\n"); run->CreateNewDataRun(ofname);}
  else {printf("Opened run for update: \n"); run->PrintSummary();}
  run->AppendSortedByTime(run1);
  if(run->rheader->cal==0) run->SetCalibs(run1->rheader->cal);
  run->BuildIndex("h2d.s","int(h2d.t0)");
  run->PrintSummary();

  run->Close();
}




