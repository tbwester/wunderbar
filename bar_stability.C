#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <cmath>

#include "TGraph.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TLegend.h"
#include "TCanvas.h"

using namespace std;

//Function prototypes
TH1F* calibhist(string file);
vector<string> parse(string s);

void bar_stability(string issub="") {
    bool sub = false;
    if (issub == "sub") {
        sub = true;
    }
    TCanvas* c1 = new TCanvas("c1","c1",1000,800);
    c1->cd();

    //TFile *fout = TFile::Open("output.root", "RECREATE");

    ifstream fin;
    fin.open("caliblist.txt");

    int runCounter = 0;
    string line;
    ostringstream log;
    log.str("");
    log << "date" << "\t"
        << "mean" << "\t"
        << "meanerr" << "\t"
        << "sig" << "\t"
        << "sigerr" << "\t"
        << "rchsq" << "\n";
         
    ostringstream bglog;
    bglog.str("");
    bglog << "date" << "\t"
          << "mean10" << "\t"
          << "rms10" << "\t"
          << "mean100" << "\t"      
          << "rms100" << "\t"
          << "mean1000" << "\t"
          << "rms1000" << "\n";
            

    while (fin >> line) {
        //line contains data file and calibration file, comma separated
        string filename = parse(line).at(0);
        string datestr = filename.substr(25,19);
        log << datestr << "\t";
        bglog << datestr << "\t";
        cout << "Now reading " << filename << "... ";

        TFile* f = TFile::Open(filename.c_str());
        string outfilestr = "output/" + filename.substr(5,39);

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

        TH1F* hs = new TH1F("h_sipm","Peak distribution;peak (V)",200,-0.2,0.2);
        TH1F* hp = new TH1F("h_pmt","Peak distribution;peak (V)",50,-0.2,0.0);
        TH1F* hbg10 = new TH1F("h_bg10", "background distribution;bg (V)",25,-0.01,0.01);
        TH1F* hbg100 = new TH1F("h_bg100", "background distribution;bg (V)",25,-0.01,0.01);
        TH1F* hbg1000 = new TH1F("h_bg1000", "background distribution;bg (V)",25,-0.01,0.01);

        int nentries = tWave->GetEntries();
        for (int i = 0; i < nentries; ++i) {
            tWave->GetEntry(i);
            //Fill background histograms
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < pow(10, i+1); ++j) {
                    if (i == 0) {
                        hbg10->Fill(ch4wfms[j]);
                    }
                    else if (i == 1) {
                        hbg100->Fill(ch4wfms[j]);
                    }
                    else {
                        hbg1000->Fill(ch4wfms[j]);
                    }
                }
            }
            //SiPM channel
            double max3 = *std::max_element(ch3wfms,ch3wfms+samples);
            //PMT channel
            double min4 = *std::min_element(ch4wfms,ch4wfms+samples);
            hs->Fill(max3);

            //Subtraction mode => Fill hist, subtract noise later
            //Else, cut on sipm
            if (sub) {
                hp->Fill(min4);
            }
            else {
                if (max3 > 0.005) {       
                    hp->Fill(min4);
                }
            }
        }

        runCounter++;
        
        //Subtraction mode => subtract noise
        TH1F* hp_calib = calibhist(parse(line).at(1));
        if (sub) {
            hp->Scale(hp_calib->Integral()/hp->Integral());
            hp->Add(hp_calib,-1);
            outfilestr += "_sub_out.root";
        }
        else {
            outfilestr += "_out.root";
        }
        TFile *outf = TFile::Open((outfilestr).c_str(), "RECREATE");

        bglog << hbg10->GetMean() << "\t"
              << hbg10->GetRMS() << "\t"
              << hbg100->GetMean() << "\t"
              << hbg100->GetRMS() << "\t"
              << hbg100->GetMean() << "\t"
              << hbg1000->GetRMS() << "\n";

        //Fitter
        double mean;
        double meanerr;
        double sig;
        double sigerr;
        double rChsq = 100;
        double rChsqNew;
        double dx = 0.01;
        double dstart = 0;
        TF1* bestfit = NULL;

        while (dstart < 0.08) {
            TF1* gaus0 = new TF1("gaus0", "gaus(0)", -0.16 + dstart, -0.16 + dstart + dx);
            double par[3];
            par[0] = 1;
            par[1] = -0.02;
            par[2] = 0.3;
            gaus0->SetParameters(par);

            //Fit and extract values
            hp->Fit(gaus0, "RBq");
            TF1* fit = (TF1 *)hp->GetFunction("gaus0");

            //Best fit has reduced chisq closest to 1
            double diff1 = abs(rChsq - 1);
            rChsq = fit->GetChisquare() / fit->GetNDF();
            double diff2 = abs(rChsq - 1);

            //Require the fit to sufficiently surround the hist peak
            if (diff2 < diff1 && (abs(fit->GetParameter(1) - (-0.16 + dstart + dx)) > 0.01)) {
                mean = fit->GetParameter(1);
                meanerr = fit->GetParError(1);
                sig = fit->GetParameter(2);
                sigerr = fit->GetParError(2);
                rChsqNew = rChsq;
                bestfit = new TF1(*fit);
            }
            dx += 0.01;
            if (dx > 0.2) {
                dx = 0.01;
                dstart += 0.01;
            }
        }

        log << mean << "\t"
            << meanerr << "\t"
            << sig << "\t"
            << sigerr << "\t"
            << rChsqNew << "\n";
        
        hp->Draw("hist");

        bestfit->SetLineColor(2);
        bestfit->Draw("sames");

        hs->Write();
        hp->Write();
        hp_calib->Write();
        hbg10->Write();
        hbg100->Write();
        hbg1000->Write();
        c1->Write();

        outf->Close();
        cout << "Done!\n";
    }
    //Write the results of the best fit for both methods
    ofstream fout;
    if (sub) {
        fout.open("fitinfo2.txt");
        fout << log.str(); 
    }
    else {
        fout.open("fitinfo.txt");
        fout << log.str();
        //On the first pass, save the results of the background hists
        ofstream fout2;
        fout2.open("bginfo.txt");
        fout2 << bglog.str();
        fout2.close();
    } 
    fout.close();  
}

//Fill pmt histogram from calibration data file
TH1F* calibhist(string file) {
    TFile* f = TFile::Open(file.c_str());
    
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

    TH1F* hs = new TH1F("h_sipm_calib","Peak distribution;peak (V)",200,-0.2,0.2);
    TH1F* hp = new TH1F("h_pmt_calib","Peak distribution;peak (V)",50,-0.2,0.0);
    int nentries = tWave->GetEntries();
    for (int i = 0; i < nentries; ++i) {
        tWave->GetEntry(i);
        //SiPM channel
        double max3 = *std::max_element(ch3wfms,ch3wfms+samples);
        //PMT channel
        double min4 = *std::min_element(ch4wfms,ch4wfms+samples);
        hs->Fill(max3);
        hp->Fill(min4);
    }
    return hp;
}

//Split string at comma
vector<string> parse(string s) {
    vector<string> vect;
    stringstream ss(s);
    string d;

    while (getline(ss, d, ',')) {
        vect.push_back(d);
    }
    return vect;
}
