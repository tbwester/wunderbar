#include <iostream>
#include <vector>
#include <iterator>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <algorithm>

#include "TGraph.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"

using namespace std;

void viewer(string filename="data/wunderbar_1_3400_30_-2016-09-01.09_41_26.root") {
    
    TCanvas* c1 = new TCanvas("c1","c1",1000,800);
    c1->cd();
    c1->Draw();
    TFile* f = TFile::Open(filename.c_str());

    int tSamples;
    TTree *tInfo = (TTree *)f->Get("waveforminfo");
    tInfo->SetBranchAddress("samples_per_waveform",&tSamples);
    tInfo->GetEntry();
    const int samples = tSamples;

    double ch3wfms[samples];
    double ch4wfms[samples];

    TTree *tWave = (TTree *)f->Get("waveformdata");
    tWave->SetBranchAddress("ch3wfms",ch3wfms);
    tWave->SetBranchAddress("ch4wfms",ch4wfms);

    int nentries = tWave->GetEntries();

    vector<double> xvec;
    for (int i = 0; i < samples; i++) {
        xvec.push_back((double)i);
    }
    double* x = &xvec[0];

    int event = -1;
    string line;
    while (line != "q") {
        cout << "Press (enter) to go to next event or specify event (number). (q) to quit: ";
        getline(cin, line);
        if (line == "") { 
            event++;
        }
        else {
            event = atoi(line.c_str());
        }
        
        //If out of range, get 0th entry
        if (event > nentries - 1) {
            event = 0;
        }

        TGraph *gr = new TGraph(samples, x, ch4wfms);
        stringstream ss;
        ss.str("");
        ss << "Event ";
        while (true) {
            tWave->GetEntry(event);
                
            if (*std::max_element(ch3wfms,ch3wfms+samples) > 0.05) {
                break;
            }
            event++;
        }
        ss << event;
        gr->SetTitle(ss.str().c_str());
        gr->Draw("alp");
        c1->Update();
    }

}
