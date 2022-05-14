

int _n_tracks;
std::vector<double> _l;
std::vector<double> _tof;
std::vector<double> _dx;

std::vector<double> _hit2d_1_t0;
std::vector<double> _hit2d_1_t1;
std::vector<double> _hit2d_1_s;
std::vector<double> _hit2d_1_x;
std::vector<double> _hit2d_1_y;
std::vector<double> _hit2d_1_z;
std::vector<double> _hit2d_1_nhits1;
std::vector<double> _hit2d_1_nhits2;
std::vector<double> _hit2d_1_edep_igor;
std::vector<double> _hit2d_1_edep;
std::vector<double> _hit2d_1_edep_nogain;
std::vector<double> _hit2d_1_hit1_edep;
std::vector<double> _hit2d_1_hit2_edep;
std::vector<double> _hit2d_1_hit1_dist_to_sipm;
std::vector<double> _hit2d_1_hit2_dist_to_sipm;
std::vector<double> _hit2d_1_hit1_mac5;
std::vector<double> _hit2d_1_hit2_mac5;
std::vector<double> _hit2d_1_hit1_strip;
std::vector<double> _hit2d_1_hit2_strip;

std::vector<double> _hit2d_2_t0;
std::vector<double> _hit2d_2_t1;
std::vector<double> _hit2d_2_s;
std::vector<double> _hit2d_2_x;
std::vector<double> _hit2d_2_y;
std::vector<double> _hit2d_2_z;
std::vector<double> _hit2d_2_nhits1;
std::vector<double> _hit2d_2_nhits2;
std::vector<double> _hit2d_2_edep_igor;
std::vector<double> _hit2d_2_edep;
std::vector<double> _hit2d_2_edep_nogain;
std::vector<double> _hit2d_2_hit1_edep;
std::vector<double> _hit2d_2_hit2_edep;
std::vector<double> _hit2d_2_hit1_dist_to_sipm;
std::vector<double> _hit2d_2_hit2_dist_to_sipm;
std::vector<double> _hit2d_2_hit1_mac5;
std::vector<double> _hit2d_2_hit2_mac5;
std::vector<double> _hit2d_2_hit1_strip;
std::vector<double> _hit2d_2_hit2_strip;


void clear_vectors() {
    _l.clear();
    _tof.clear();
    _dx.clear();

    _hit2d_1_t0.clear();
    _hit2d_1_t1.clear();
    _hit2d_1_s.clear();
    _hit2d_1_x.clear();
    _hit2d_1_y.clear();
    _hit2d_1_z.clear();
    _hit2d_1_nhits1.clear();
    _hit2d_1_nhits2.clear();
    _hit2d_1_edep_igor.clear();
    _hit2d_1_edep.clear();
    _hit2d_1_edep_nogain.clear();
    _hit2d_1_hit1_edep.clear();
    _hit2d_1_hit2_edep.clear();
    _hit2d_1_hit1_dist_to_sipm.clear();
    _hit2d_1_hit2_dist_to_sipm.clear();
    _hit2d_1_hit1_mac5.clear();
    _hit2d_1_hit2_mac5.clear();
    _hit2d_1_hit1_strip.clear();
    _hit2d_1_hit2_strip.clear();

    _hit2d_2_t1.clear();
    _hit2d_2_t0.clear();
    _hit2d_2_s.clear();
    _hit2d_2_x.clear();
    _hit2d_2_y.clear();
    _hit2d_2_z.clear();
    _hit2d_2_nhits1.clear();
    _hit2d_2_nhits2.clear();
    _hit2d_2_edep_igor.clear();
    _hit2d_2_edep.clear();
    _hit2d_2_edep_nogain.clear();
    _hit2d_2_hit1_edep.clear();
    _hit2d_2_hit2_edep.clear();
    _hit2d_2_hit1_dist_to_sipm.clear();
    _hit2d_2_hit2_dist_to_sipm.clear();
    _hit2d_2_hit1_mac5.clear();
    _hit2d_2_hit2_mac5.clear();
    _hit2d_2_hit1_strip.clear();
    _hit2d_2_hit2_strip.clear();
}

TVector3 GetIntersection(double);



void crt_tracks() {

    TStopwatch t;
    t.Start();

    // gSystem->Load("/sbnd/app/users/mdeltutt/Projects/CRTData/libCRT/lib/libCRT.so");

    // Oputput
    TFile* _out_file = TFile::Open("crt_tracks_tree_nogain.root","RECREATE");

    TTree* _tree = new TTree("t", "CRT Flat TTree");
    // Standard variables
    _tree->Branch("n_tracks", &_n_tracks, "n_tracks/I");
    _tree->Branch("l", "std::vector<double>", &_l);
    _tree->Branch("tof", "std::vector<double>", &_tof);
    _tree->Branch("dx", "std::vector<double>", &_dx);

    _tree->Branch("hit2d_1_t0", "std::vector<double>", &_hit2d_1_t0);
    _tree->Branch("hit2d_1_t1", "std::vector<double>", &_hit2d_1_t1);
    _tree->Branch("hit2d_1_s", "std::vector<double>", &_hit2d_1_s);
    _tree->Branch("hit2d_1_x", "std::vector<double>", &_hit2d_1_x);
    _tree->Branch("hit2d_1_y", "std::vector<double>", &_hit2d_1_y);
    _tree->Branch("hit2d_1_z", "std::vector<double>", &_hit2d_1_z);
    _tree->Branch("hit2d_1_nhits1", "std::vector<double>", &_hit2d_1_nhits1);
    _tree->Branch("hit2d_1_nhits2", "std::vector<double>", &_hit2d_1_nhits2);
    _tree->Branch("hit2d_1_edep_igor", "std::vector<double>", &_hit2d_1_edep_igor);
    _tree->Branch("hit2d_1_edep", "std::vector<double>", &_hit2d_1_edep);
    _tree->Branch("hit2d_1_edep_nogain", "std::vector<double>", &_hit2d_1_edep_nogain);
    _tree->Branch("hit2d_1_hit1_edep", "std::vector<double>", &_hit2d_1_hit1_edep);
    _tree->Branch("hit2d_1_hit2_edep", "std::vector<double>", &_hit2d_1_hit2_edep);
    _tree->Branch("hit2d_1_hit1_dist_to_sipm", "std::vector<double>", &_hit2d_1_hit1_dist_to_sipm);
    _tree->Branch("hit2d_1_hit2_dist_to_sipm", "std::vector<double>", &_hit2d_1_hit2_dist_to_sipm);
    _tree->Branch("hit2d_1_hit1_mac5", "std::vector<double>", &_hit2d_1_hit1_mac5);
    _tree->Branch("hit2d_1_hit2_mac5", "std::vector<double>", &_hit2d_1_hit2_mac5);
    _tree->Branch("hit2d_1_hit1_strip", "std::vector<double>", &_hit2d_1_hit1_strip);
    _tree->Branch("hit2d_1_hit2_strip", "std::vector<double>", &_hit2d_1_hit2_strip);

    _tree->Branch("hit2d_2_t0", "std::vector<double>", &_hit2d_2_t0);
    _tree->Branch("hit2d_2_t1", "std::vector<double>", &_hit2d_2_t1);
    _tree->Branch("hit2d_2_s", "std::vector<double>", &_hit2d_2_s);
    _tree->Branch("hit2d_2_x", "std::vector<double>", &_hit2d_2_x);
    _tree->Branch("hit2d_2_y", "std::vector<double>", &_hit2d_2_y);
    _tree->Branch("hit2d_2_z", "std::vector<double>", &_hit2d_2_z);
    _tree->Branch("hit2d_2_nhits1", "std::vector<double>", &_hit2d_2_nhits1);
    _tree->Branch("hit2d_2_nhits2", "std::vector<double>", &_hit2d_2_nhits2);
    _tree->Branch("hit2d_2_edep_igor", "std::vector<double>", &_hit2d_2_edep_igor);
    _tree->Branch("hit2d_2_edep", "std::vector<double>", &_hit2d_2_edep);
    _tree->Branch("hit2d_2_edep_nogain", "std::vector<double>", &_hit2d_2_edep_nogain);
    _tree->Branch("hit2d_2_hit1_edep", "std::vector<double>", &_hit2d_2_hit1_edep);
    _tree->Branch("hit2d_2_hit2_edep", "std::vector<double>", &_hit2d_2_hit2_edep);
    _tree->Branch("hit2d_2_hit1_dist_to_sipm", "std::vector<double>", &_hit2d_2_hit1_dist_to_sipm);
    _tree->Branch("hit2d_2_hit2_dist_to_sipm", "std::vector<double>", &_hit2d_2_hit2_dist_to_sipm);
    _tree->Branch("hit2d_2_hit1_mac5", "std::vector<double>", &_hit2d_2_hit1_mac5);
    _tree->Branch("hit2d_2_hit2_mac5", "std::vector<double>", &_hit2d_2_hit2_mac5);
    _tree->Branch("hit2d_2_hit1_strip", "std::vector<double>", &_hit2d_2_hit1_strip);
    _tree->Branch("hit2d_2_hit2_strip", "std::vector<double>", &_hit2d_2_hit2_strip);

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

        _n_tracks = run->trk->GetEntries();

        // Loop over all the CRT tracks
        for (size_t j = 0; j < run->trk->GetEntries(); j++) {
            CRTTrack* track = (CRTTrack*) run->trk->At(j);
            _l.push_back(track->L);
            _tof.push_back(track->tof);

            CRT2Dhit h2d_1 = track->hit2d[0];
            CRT2Dhit h2d_2 = track->hit2d[1];

            // Make sure the first 2d hit is the upstream one
            if (h2d_1.y > h2d_2.y) {
                std::swap(h2d_1, h2d_2);
            }


            // Get the first 2D hit (one point of the track)
            _hit2d_1_t0.push_back(h2d_1.t0);
            _hit2d_1_t1.push_back(h2d_1.t1);
            _hit2d_1_s.push_back(h2d_1.s);
            _hit2d_1_x.push_back(h2d_1.x);
            _hit2d_1_y.push_back(h2d_1.y);
            _hit2d_1_z.push_back(h2d_1.z);
            _hit2d_1_nhits1.push_back(h2d_1.nhits1);
            _hit2d_1_nhits2.push_back(h2d_1.nhits2);
            _hit2d_1_edep_igor.push_back(h2d_1.Edep);

            // Get the associated 1D hits that make this 2D hit
            CRTRawhit raw_hit_1 = h2d_1.rhit[0];
            CRTRawhit raw_hit_2 = h2d_1.rhit[1];
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
            _hit2d_1_edep.push_back(Edep1 + Edep2);
            _hit2d_1_edep_nogain.push_back((raw_hit1_max_adc - raw_hit1_pedestal) + (raw_hit2_max_adc - raw_hit2_pedestal));
            _hit2d_1_hit1_edep.push_back(Edep1);
            _hit2d_1_hit2_edep.push_back(Edep2);

            double dist_to_sipm1 = cal->getDistanceToSIPM1(raw_hit_1.mac5, raw_hit1_max_adc_idx,
                                                           raw_hit_2.mac5, raw_hit2_max_adc_idx);
            double dist_to_sipm2 = cal->getDistanceToSIPM2(raw_hit_1.mac5, raw_hit1_max_adc_idx,
                                                           raw_hit_2.mac5, raw_hit2_max_adc_idx);
            _hit2d_1_hit1_dist_to_sipm.push_back(dist_to_sipm1);
            _hit2d_1_hit2_dist_to_sipm.push_back(dist_to_sipm2);

            _hit2d_1_hit1_mac5.push_back(raw_hit_1.mac5);
            _hit2d_1_hit2_mac5.push_back(raw_hit_2.mac5);
            _hit2d_1_hit1_strip.push_back(raw_hit1_max_adc_idx);
            _hit2d_1_hit2_strip.push_back(raw_hit2_max_adc_idx);




            // Get the second 2D hit (the other point that makes the track)
            _hit2d_2_t0.push_back(h2d_2.t0);
            _hit2d_2_t1.push_back(h2d_2.t1);
            _hit2d_2_s.push_back(h2d_2.s);
            _hit2d_2_x.push_back(h2d_2.x);
            _hit2d_2_y.push_back(h2d_2.y);
            _hit2d_2_z.push_back(h2d_2.z);
            _hit2d_2_nhits1.push_back(h2d_2.nhits1);
            _hit2d_2_nhits2.push_back(h2d_2.nhits2);
            _hit2d_2_edep_igor.push_back(h2d_2.Edep);

            // Get the associated 1D hits that make this 2D hit
            raw_hit_1 = h2d_2.rhit[0];
            raw_hit_2 = h2d_2.rhit[1];
            raw_hit1_max_adc = raw_hit_1.GetMaxStripValue();
            raw_hit2_max_adc = raw_hit_2.GetMaxStripValue();
            raw_hit1_max_adc_idx = raw_hit_1.GetMaxStripIndex();
            raw_hit2_max_adc_idx = raw_hit_2.GetMaxStripIndex();
            raw_hit1_pedestal = cal->ADCPedestal[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            raw_hit2_pedestal = cal->ADCPedestal[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            raw_hit1_gain = cal->ADCGain[raw_hit_1.mac5][raw_hit1_max_adc_idx];
            raw_hit2_gain = cal->ADCGain[raw_hit_2.mac5][raw_hit2_max_adc_idx];
            Edep1 = (raw_hit1_max_adc - raw_hit1_pedestal)/((double)raw_hit1_gain);
            Edep2 = (raw_hit2_max_adc - raw_hit2_pedestal)/((double)raw_hit2_gain);
            _hit2d_2_edep_nogain.push_back((raw_hit1_max_adc - raw_hit1_pedestal) + (raw_hit2_max_adc - raw_hit2_pedestal));
            _hit2d_2_edep.push_back(Edep1 + Edep2);
            _hit2d_2_hit1_edep.push_back(Edep1);
            _hit2d_2_hit2_edep.push_back(Edep2);

            dist_to_sipm1 = cal->getDistanceToSIPM1(raw_hit_1.mac5, raw_hit1_max_adc_idx,
                                                    raw_hit_2.mac5, raw_hit2_max_adc_idx);
            dist_to_sipm2 = cal->getDistanceToSIPM2(raw_hit_1.mac5, raw_hit1_max_adc_idx,
                                                    raw_hit_2.mac5, raw_hit2_max_adc_idx);
            _hit2d_2_hit1_dist_to_sipm.push_back(dist_to_sipm1);
            _hit2d_2_hit2_dist_to_sipm.push_back(dist_to_sipm2);

            _hit2d_2_hit1_mac5.push_back(raw_hit_1.mac5);
            _hit2d_2_hit2_mac5.push_back(raw_hit_2.mac5);
            _hit2d_2_hit1_strip.push_back(raw_hit1_max_adc_idx);
            _hit2d_2_hit2_strip.push_back(raw_hit2_max_adc_idx);

            TVector3 int1 = GetIntersection(1046.25 + 1);
            TVector3 int2 = GetIntersection(1046.25 - 1);
            _dx.push_back((int1 - int2).Mag());

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

TVector3 GetIntersection(double y_position)
{
    TVector3 ray_vector(_hit2d_2_x.back() - _hit2d_1_x.back(),
                        _hit2d_2_y.back() - _hit2d_1_y.back(),
                        _hit2d_2_z.back() - _hit2d_1_z.back());
    ray_vector = ray_vector.Unit();
    TVector3 ray_point(_hit2d_1_x.back(), _hit2d_1_y.back(), _hit2d_1_z.back());
    TVector3 plane_point(0, y_position, 0);
    TVector3 plane_normal(0, -1, 0);

    TVector3 diff = plane_point - ray_point;
    double prod1 = diff.Dot(plane_normal);
    double prod2 = ray_vector.Dot(plane_normal);
    double prod3 = prod1 / prod2;
    TVector3 intersection = ray_point + ray_vector * prod3;
    // std::cout << "ray_vector: " << ray_vector.X() << " " << ray_vector.Y() << " " << ray_vector.Z() << std::endl;
    // std::cout << "ray_point: " << ray_point.X() << " " << ray_point.Y() << " " << ray_point.Z() << std::endl;
    // std::cout << "plane_point: " << plane_point.X() << " " << plane_point.Y() << " " << plane_point.Z() << std::endl;
    // std::cout << "plane_normal: " << plane_normal.X() << " " << plane_normal.Y() << " " << plane_normal.Z() << std::endl;
    // std::cout << "intersection: " << intersection.X() << " " << intersection.Y() << " " << intersection.Z() << std::endl;
    return intersection;
}
