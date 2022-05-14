CRTRawhit *hit;
CRTEvent *evt;
CRT2Dhit *e2d;
CRTRawhitBuffer *rhbuf;
CRTCalibs*  cal;
CRTRun * run;
CRTRun * run1;
CRTRun * run2;


void GroupAndClassify(const char *fname1="",const char *ofname="")
{
  run=new CRTRun();
  run1=new CRTRun();
  if(strlen(fname1)==0) {printf("run1: No input file name is given.\n");return;};
  if(strlen(ofname)==0) {printf("No output file name is given.\n");return;};
  if(run1->OpenExistingDataRun(fname1)==0){printf("run1: Extracting CRTRun object is failed.\n");return;};

  run=new CRTRun();
  run->CreateNewDataRun(ofname);
  run->AddPOTAndNSpills(run1->rheader->POT, run1->rheader->NSpills);
  run->rheader->POT = run1->rheader->POT;
  run->rheader->NSpills = run1->rheader->NSpills;

  run->GroupAndClassify(run1, 50000); // was 100, using 50 ms
  run->BuildIndex("h2d[0].s","int(h2d[0].t0)");
  run->PrintSummary();

  run->Close();
}




