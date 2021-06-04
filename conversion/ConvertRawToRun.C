
CRTRawhit *hit;
CRTRun * run;
FILE *data = 0;
char buf[EVLEN]; //EVLEN is defined in libCRT headers
char ofname[256];
Int_t sec=0;
void ConvertRawToRun(char *fname="")
{
  if(strlen(fname)==0) return;
  rhbuf=new CRTRawhitBuffer();
  run=new CRTRun();
  sprintf(ofname,"%s.root",fname);
  run->CreateNewDataRun(ofname);
//  run->CreateNewDataRun("test.root");
  hit=new CRTRawhit(); 
  cal=new CRTCalibs("FEB_CableDelay-V8.txt","CRTpositionsSiPM-V8.txt","AdcCalibration-V4.txt");
  cal->SetVersion(8);
  run->SetCalibs(cal); 
  int evsread=0;
  data = fopen(fname,"r");
  if(data<=0) return;	
  else printf("Opened raw data file %s.\n",fname);
  int EndOfFile=0;
  int hitsinev=0;

  while(!EndOfFile)
 // while(evsread<20000)
     {  
      if(fread(buf,EVLEN,1,data)<=0) {EndOfFile=1; break;};
      hit->ReadFromBuffer(buf);
      evsread++;
      hitsinev++;
      if(!hit->IsEOPEvent()) rhbuf->AddRawhit(hit);
      else 
      {
        sec=hit->GetSec(); 
        for(int i=0; i<rhbuf->Nhits; i++) ((CRTRawhit*)(rhbuf->hits->At(i)))->s=sec;
        run->AddRawhitBuffer(rhbuf); rhbuf->Clear();
      } 
 //     run->AddRawhit(hit); run->Fill();run->ClearEntry();
      
      if(hitsinev%10000==0) printf("%d events processed..\n",evsread);
      
     }
 run->PrintSummary();
 run->Close();
}




