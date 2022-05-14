
int _n_2dhits;
std::vector<double> _t0;
std::vector<double> _t1;
std::vector<double> _dt0;
std::vector<double> _dt1;
std::vector<int> _s;
std::vector<double> _x;
std::vector<double> _y;
std::vector<double> _z;
std::vector<int> _nhits1;
std::vector<int> _nhits2;
std::vector<bool> _calibrated;
std::vector<double> _Edep_igor;
std::vector<double> _edep;
std::vector<double> _edep_nogain;
std::vector<double> _edep_h1;
std::vector<double> _edep_h2;

// Int_t _hit1_max_adc;
// Int_t _hit2_max_adc;
// Int_t _hit1_max_adc_idx;
// Int_t _hit2_max_adc_idx;
// Int_t _hit1_pedestal;
// Int_t _hit2_pedestal;
// Int_t _hit1_gain;
// Int_t _hit2_gain;
// Double_t _edep_v1;
// Double_t _edep_v2;

void clear_vectors() {
    _t0.clear();
    _t1.clear();
    _dt0.clear();
    _dt1.clear();
    _s.clear();
    _x.clear();
    _y.clear();
    _z.clear();
    _nhits1.clear();
    _nhits2.clear();
    _calibrated.clear();
    _Edep_igor.clear();
    _edep.clear();
    _edep_nogain.clear();
    _edep_h1.clear();
    _edep_h2.clear();
}



void flat_crt_tree() {

    TStopwatch t;
    t.Start();

    // gSystem->Load("/sbnd/app/users/mdeltutt/Projects/CRTData/libCRT/lib/libCRT.so");

    // Oputput
    TFile* _out_file = TFile::Open("out_flat_crt_tree_nogain.root","RECREATE");

    TTree* _tree = new TTree("t", "CRT Flat TTree");
    // Standard variables
    _tree->Branch("n_2dhits", &_n_2dhits, "n_2dhits/I");
    _tree->Branch("t0", "std::vector<double>", &_t0);
    _tree->Branch("t1", "std::vector<double>", &_t1);
    _tree->Branch("dt0", "std::vector<double>", &_dt0);
    _tree->Branch("dt1", "std::vector<double>", &_dt1);
    _tree->Branch("s", "std::vector<int>", &_s);
    _tree->Branch("x", "std::vector<double>", &_x);
    _tree->Branch("y", "std::vector<double>", &_y);
    _tree->Branch("z", "std::vector<double>", &_z);
    _tree->Branch("nhits1", "std::vector<int>", &_nhits1);
    _tree->Branch("nhits2", "std::vector<int>", &_nhits2);
    _tree->Branch("calibrated", "std::vector<bool>", &_calibrated);
    _tree->Branch("Edep_igor", "std::vector<double>", &_Edep_igor);
    _tree->Branch("edep", "std::vector<double>", &_edep);
    _tree->Branch("edep_nogain", "std::vector<double>", &_edep_nogain);
    _tree->Branch("edep_h1", "std::vector<double>", &_edep_h1);
    _tree->Branch("edep_h2", "std::vector<double>", &_edep_h2);


    // _tree->Branch("t0", &_t0, "t0/D");
    // _tree->Branch("t1", &_t1, "t1/D");
    // _tree->Branch("dt0", &_dt0, "dt0/D");
    // _tree->Branch("dt1", &_dt1, "dt1/D");
    // _tree->Branch("s", &_s, "s/I");
    // _tree->Branch("x", &_x, "x/D");
    // _tree->Branch("y", &_y, "y/D");
    // _tree->Branch("z", &_z, "z/D");
    // _tree->Branch("nhits1", &_nhits1, "nhits1/I");
    // _tree->Branch("nhits2", &_nhits2, "nhits2/I");
    // _tree->Branch("calibrated", &_calibrated, "calibrated/O");
    // _tree->Branch("Edep", &_Edep, "Edep/D");
    // Additional variables
    // _tree->Branch("hit1_max_adc", &_hit1_max_adc, "hit1_max_adc/I");
    // _tree->Branch("hit2_max_adc", &_hit2_max_adc, "hit2_max_adc/I");
    // _tree->Branch("hit1_max_adc_idx", &_hit1_max_adc_idx, "hit1_max_adc_idx/I");
    // _tree->Branch("hit2_max_adc_idx", &_hit2_max_adc_idx, "hit2_max_adc_idx/I");
    // _tree->Branch("hit1_pedestal", &_hit1_pedestal, "hit1_pedestal/I");
    // _tree->Branch("hit2_pedestal", &_hit2_pedestal, "hit2_pedestal/I");
    // _tree->Branch("hit1_gain", &_hit1_gain, "hit1_gain/I");
    // _tree->Branch("hit2_gain", &_hit2_gain, "hit2_gain/I");
    // _tree->Branch("edep_v1", &_edep_v1, "edep_v1/D");
    // _tree->Branch("edep_v2", &_edep_v2, "edep_v2/D");


    CRTCalibs* cal = new CRTCalibs("SBND_CableDelay-V3.txt",
                                   "SBND_CRTpositionsSiPM-nogaps.txt",
                                   "SBND_ADCCalib-V4.txt");

    CRTRun* run = new CRTRun();
    // run->OpenExistingDataRun("/sbnd/data/users/mdeltutt/crt_data/beam_converted_merged/merged_beam_Dec2017_libcrt.root");
    run->OpenExistingDataRun("/sbnd/data/users/mdeltutt/crt_data/beam_converted_merged/merged_beam_all_libcrt.classified.root");

    size_t n_entries = run->t->GetEntries();
    std::cout << "Number of entries: " << n_entries << std::endl;

    for (size_t i = 0; i < n_entries; i++) {

        clear_vectors();

        if (i % 10000 == 0) {
            std::cout << "At entry " << i << std::endl;
        }
        // if (i == 100) {
        //     break;
        // }


        // run->GetEntrySortedByTime(i);
        run->GetEntry(i);


        // if (run->h2d->GetEntries() != 1) {
        //     std::cout << "More than 1 2d hits! " << run->h2d->GetEntries() << std::endl;
        // }
        // std::cout << "number of 2d hits " << run->h2d->GetEntries() << std::endl;
        // CRT2Dhit* h2d = (CRT2Dhit*) run->h2d->At(0);

        // Copy 2D hits variables
        _n_2dhits = run->h2d->GetEntries();
        for (size_t j = 0; j < run->h2d->GetEntries(); j++) {
            CRT2Dhit* h2d = (CRT2Dhit*) run->h2d->At(j);
            _t0.push_back(h2d->t0);
            _t1.push_back(h2d->t1);
            _dt0.push_back(h2d->dt0);
            _dt1.push_back(h2d->dt1);
            _s.push_back(h2d->s);
            _x.push_back(h2d->x);
            _y.push_back(h2d->y);
            _z.push_back(h2d->z);
            _nhits1.push_back(h2d->nhits1);
            _nhits2.push_back(h2d->nhits2);
            _calibrated.push_back(h2d->calibrated);
            _Edep_igor.push_back(h2d->Edep);

            // Get the associated 1D hits
            CRTRawhit raw_hit_1 = h2d->rhit[0];
            CRTRawhit raw_hit_2 = h2d->rhit[1];
            int raw_hit1_max_adc = raw_hit_1.GetMaxStripValue();
            int raw_hit2_max_adc = raw_hit_2.GetMaxStripValue();
            int raw_hit1_max_adc_idx = raw_hit_1.GetMaxStripIndex();
            int raw_hit2_max_adc_idx = raw_hit_2.GetMaxStripIndex();
            int raw_hit1_pedestal = cal->ADCPedestal[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            int raw_hit2_pedestal = cal->ADCPedestal[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            int raw_hit1_gain = cal->ADCGain[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            int raw_hit2_gain = cal->ADCGain[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            double Edep1 = (raw_hit1_max_adc - raw_hit1_pedestal)/((double)raw_hit1_gain);
            double Edep2 = (raw_hit2_max_adc - raw_hit2_pedestal)/((double)raw_hit2_gain);
            _edep.push_back(Edep1 + Edep2);
            _edep_nogain.push_back((raw_hit1_max_adc - raw_hit1_pedestal) + (raw_hit2_max_adc - raw_hit2_pedestal));
            _edep_h1.push_back(Edep1);
            _edep_h2.push_back(Edep2);
        }
        // _t0 = h2d->t0;
        // _t1 = h2d->t1;
        // _dt0 = h2d->dt0;
        // _dt1 = h2d->dt1;
        // _s = h2d->s;
        // _x = h2d->x;
        // _y = h2d->y;
        // _z = h2d->z;
        // _nhits1 = h2d->nhits1;
        // _nhits2 = h2d->nhits2;
        // _calibrated = h2d->calibrated;
        // _Edep = h2d->Edep;

            // // Get the associated 1D hits
            // CRTRawhit raw_hit_1 = h2d->rhit[0];
            // CRTRawhit raw_hit_2 = h2d->rhit[1];
            // int raw_hit1_max_adc = raw_hit_1.GetMaxStripValue();
            // int raw_hit2_max_adc = raw_hit_2.GetMaxStripValue();
            // int raw_hit1_max_adc_idx = raw_hit_1.GetMaxStripIndex();
            // int raw_hit2_max_adc_idx = raw_hit_2.GetMaxStripIndex();
            // int raw_hit1_pedestal = cal->ADCPedestal[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            // int raw_hit2_pedestal = cal->ADCPedestal[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            // int raw_hit1_gain = cal->ADCGain[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            // int raw_hit2_gain = cal->ADCGain[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            // double Edep1 = (raw_hit1_max_adc - raw_hit1_pedestal)/((double)raw_hit1_gain);
            // double Edep2 = (raw_hit2_max_adc - raw_hit2_pedestal)/((double)raw_hit2_gain);
            // _hit2d_1_edep.push_back(Edep1 + Edep2);
            // _hit2d_1_hit1_edep.push_back(Edep1);
            // _hit2d_1_hit2_edep.push_back(Edep2);

        // // float Edep = 2.1 * (raw_hit_1.GetMaxStripValue() + raw_hit_2.GetMaxStripValue()) / 2. / 1000.;

        // _hit1_max_adc = raw_hit_1.GetMaxStripValue();
        // _hit2_max_adc = raw_hit_2.GetMaxStripValue();

        // _hit1_max_adc_idx = raw_hit_1.GetMaxStripIndex();
        // _hit2_max_adc_idx = raw_hit_2.GetMaxStripIndex();

        // _hit1_pedestal = cal->ADCPedestal[raw_hit_1.mac5][_hit1_max_adc_idx];
        // _hit2_pedestal = cal->ADCPedestal[raw_hit_2.mac5][_hit2_max_adc_idx];

        // _hit1_gain = cal->ADCGain[raw_hit_1.mac5][_hit1_max_adc_idx];
        // _hit2_gain = cal->ADCGain[raw_hit_2.mac5][_hit2_max_adc_idx];

        // // Deposited energy v1: taken from a commented line in libCRT
        // _edep_v1 = 0;
        // for (Int_t i = 0; i < 32; i++) {
        //     _edep_v1 = _edep_v1+2.1*(raw_hit_1.adc[i] - cal->ADCPedestal[raw_hit_1.mac5][i])/(1.*cal->ADCGain[raw_hit_1.mac5][i]);
        //     _edep_v1 = _edep_v1+2.1*(raw_hit_1.adc[i] - cal->ADCPedestal[raw_hit_1.mac5][i])/(1.*cal->ADCGain[raw_hit_1.mac5][i]);
        // }

        // // Deposited energy v2: a test
        // _edep_v2 = 0;
        // _edep_v2 += ((Double_t)_hit1_max_adc - (Double_t)cal->ADCPedestal[raw_hit_1.mac5][_hit1_max_adc_idx]) / (Double_t)cal->ADCGain[raw_hit_1.mac5][_hit1_max_adc_idx];
        // _edep_v2 += ((Double_t)_hit2_max_adc - (Double_t)cal->ADCPedestal[raw_hit_2.mac5][_hit2_max_adc_idx]) / (Double_t)cal->ADCGain[raw_hit_2.mac5][_hit2_max_adc_idx];

        // std::cout << "Edep from tree " << h2d->Edep << "   by myself " << Edep << ", " << edep_hit1_calib << std::endl;

        _tree->Fill();
    }


    // _tree->Print();
    _out_file->cd();
    _tree->Write();
    _out_file->Close();

    t.Stop();
    t.Print();
}
