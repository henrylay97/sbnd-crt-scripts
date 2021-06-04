
std::vector<double> _l;
std::vector<double> _tof;

std::vector<double> _hit1_t0;
std::vector<double> _hit1_t1;
std::vector<double> _hit1_s;
std::vector<double> _hit1_x;
std::vector<double> _hit1_y;
std::vector<double> _hit1_z;
std::vector<double> _hit1_nhits1;
std::vector<double> _hit1_nhits2;
std::vector<double> _hit1_edep;

std::vector<double> _hit2_t0;
std::vector<double> _hit2_t1;
std::vector<double> _hit2_s;
std::vector<double> _hit2_x;
std::vector<double> _hit2_y;
std::vector<double> _hit2_z;
std::vector<double> _hit2_nhits1;
std::vector<double> _hit2_nhits2;
std::vector<double> _hit2_edep;


void clear_vectors() {
    _l.clear();
    _tof.clear();

    _hit1_t0.clear();
    _hit1_t1.clear();
    _hit1_s.clear();
    _hit1_x.clear();
    _hit1_y.clear();
    _hit1_z.clear();
    _hit1_nhits1.clear();
    _hit1_nhits2.clear();
    _hit1_edep.clear();

    _hit2_t1.clear();
    _hit2_t0.clear();
    _hit2_s.clear();
    _hit2_x.clear();
    _hit2_y.clear();
    _hit2_z.clear();
    _hit2_nhits1.clear();
    _hit2_nhits2.clear();
    _hit2_edep.clear();
}



void crt_tracks() {

    TStopwatch t;
    t.Start();

    // gSystem->Load("/sbnd/app/users/mdeltutt/Projects/CRTData/libCRT/lib/libCRT.so");

    // Oputput
    TFile* _out_file = TFile::Open("crt_tracks_tree.root","RECREATE");

    TTree* _tree = new TTree("t", "CRT Flat TTree");
    // Standard variables
    _tree->Branch("l", "std::vector<double>", &_l);
    _tree->Branch("tof", "std::vector<double>", &_tof);

    _tree->Branch("hit1_t0", "std::vector<double>", &_hit1_t0);
    _tree->Branch("hit1_t1", "std::vector<double>", &_hit1_t1);
    _tree->Branch("hit1_s", "std::vector<double>", &_hit1_s);
    _tree->Branch("hit1_x", "std::vector<double>", &_hit1_x);
    _tree->Branch("hit1_y", "std::vector<double>", &_hit1_y);
    _tree->Branch("hit1_z", "std::vector<double>", &_hit1_z);
    _tree->Branch("hit1_nhits1", "std::vector<double>", &_hit1_nhits1);
    _tree->Branch("hit1_nhits2", "std::vector<double>", &_hit1_nhits2);
    _tree->Branch("hit1_edep", "std::vector<double>", &_hit1_edep);

    _tree->Branch("hit2_t0", "std::vector<double>", &_hit2_t0);
    _tree->Branch("hit2_t1", "std::vector<double>", &_hit2_t1);
    _tree->Branch("hit2_s", "std::vector<double>", &_hit2_s);
    _tree->Branch("hit2_x", "std::vector<double>", &_hit2_x);
    _tree->Branch("hit2_y", "std::vector<double>", &_hit2_y);
    _tree->Branch("hit2_z", "std::vector<double>", &_hit2_z);
    _tree->Branch("hit2_nhits1", "std::vector<double>", &_hit2_nhits1);
    _tree->Branch("hit2_nhits2", "std::vector<double>", &_hit2_nhits2);
    _tree->Branch("hit2_edep", "std::vector<double>", &_hit2_edep);

    CRTCalibs* cal = new CRTCalibs("SBND_CableDelay-V3.txt",
                                   "SBND_CRTpositionsSiPM-nogaps.txt",
                                   "SBND_ADCCalib-V4.txt");

    CRTRun* run = new CRTRun();
    run->OpenExistingDataRun("/sbnd/data/users/mdeltutt/crt_data/beam_converted_merged/merged_beam_all_libcrt.passing.root");

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
        for (size_t j = 0; j < run->trk->GetEntries(); j++) {
            CRTTrack* track = (CRTTrack*) run->trk->At(j);
            _l.push_back(track->L);
            _tof.push_back(track->tof);

            CRT2Dhit h2d = track->hit2d[0];
            _hit1_t0.push_back(h2d.t0);
            _hit1_t1.push_back(h2d.t1);
            _hit1_s.push_back(h2d.s);
            _hit1_x.push_back(h2d.x);
            _hit1_y.push_back(h2d.y);
            _hit1_z.push_back(h2d.z);
            _hit1_nhits1.push_back(h2d.nhits1);
            _hit1_nhits2.push_back(h2d.nhits2);
            _hit1_edep.push_back(h2d.Edep);

            h2d = track->hit2d[1];
            _hit2_t0.push_back(h2d.t0);
            _hit2_t1.push_back(h2d.t1);
            _hit2_s.push_back(h2d.s);
            _hit2_x.push_back(h2d.x);
            _hit2_y.push_back(h2d.y);
            _hit2_z.push_back(h2d.z);
            _hit2_nhits1.push_back(h2d.nhits1);
            _hit2_nhits2.push_back(h2d.nhits2);
            _hit2_edep.push_back(h2d.Edep);
        }

        _tree->Fill();
    }


    // _tree->Print();
    _out_file->cd();
    _tree->Write();
    _out_file->Close();

    t.Stop();
    t.Print();
}