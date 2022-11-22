#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>

#include <TH1D.h>
#include <THStack.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRint.h>
#include <TLegend.h>
#include <TColor.h>
#include <TMath.h>

#include "samplePDFDUNE/samplePDFDUNENDBase.h"
#include "manager/manager.h"


std::string getNameNoExt(std::string name, std::string ext)  
{                                                            
  std::size_t pos ;                                          
  pos = name.find(ext);                                      
  name = name.substr(0,pos);                                 
  return name ;                                              
}                                                            
                                                             
void saveCanvas(TCanvas* canvas, std::string name, std::string legend)                                                                  
{                                                            
  name = getNameNoExt(name, ".root") ;                       
  name = name + legend + ".root" ;                     
  canvas -> SaveAs(name.c_str()) ;                           
                                                             
  name = getNameNoExt(name, ".root") ;                       
  name = name + ".png" ;                                     
  canvas -> SaveAs(name.c_str()) ;                           
                                                             
  name = getNameNoExt(name, ".png") ;                        
  name = name + ".pdf" ;                                     
  canvas -> SaveAs(name.c_str()) ;                           
                                                             
  name = getNameNoExt(name, ".pdf") ;                        
  name = name + ".eps" ;                                     
  canvas -> SaveAs(name.c_str()) ;                           
                                                             
} 

std::vector<double> get_default_CAFana_bins(){
     // From CAFana - probability binning -
     const int kNumTrueEnergyBins = 100;
  
     // N+1 bin low edges
     std::vector<double> edges(kNumTrueEnergyBins+1);
  
     const double Emin = 0.5; // 500 MeV: there's really no events below there
  
     // How many edges to generate. Allow room for 0-Emin bi            const double N = kNumTrueEnergyBins-1;
     const double N = kNumTrueEnergyBins-1;
     const double A = N*Emin;
  
     edges[0] = 0;
  
     for(int i = 1; i <= N; ++i){
       edges[kNumTrueEnergyBins-i] = A/i;
     }
  
     edges[kNumTrueEnergyBins] = 120; // Replace the infinity that would be here
     return edges;
                                 
}


int main(int argc, char * argv[]) {

  // ----------------------- OPTIONS ---------------------------------------- //

  if(argc == 1){
    std::cout << "Usage: bin/mini_MCMC config.cfg" << std::endl;
    return 1;
  }

  manager *fitMan = new manager(argv[1]);

  std::string  XsecMatrixFile = fitMan->raw()["General"]["Systematics"]["XsecCovFile"].as<std::string>(); 
  std::string  XsecMatrixName = fitMan->raw()["General"]["Systematics"]["XsecCovName"].as<std::string>();
  std::string  OscMatrixFile = fitMan->raw()["General"]["Systematics"]["OscCovFile"].as<std::string>(); 
  std::string  OscMatrixName = fitMan->raw()["General"]["Systematics"]["OscCovName"].as<std::string>();
 
  std::string OutfileName = fitMan->raw()["General"]["Output"]["OUTPUTNAME"].as<std::string>();

  // there's a check inside the manager class that does this; left here for demonstrative purposes
  // if (fitMan->GetGoodConfig() == false) {
  //   std::cerr << "Didn't find a good config in input configuration" << std::endl;
  //   throw;
  // }

  std::string  fluxMatrixFile = ""; //fitMan -> GetFluxCovMatrix();
  std::string  fluxMatrixName = ""; //fitMan -> GetFluxCovName();
  std::string  xsecMatrixFile = ""; //fitMan -> GetXsecCovMatrix();
  std::string  xsecMatrixName = ""; //fitMan -> GetXsecCovName();

  // Asimov fit
  bool asimovfit = false;//fitMan->GetAsimovFitFlag();
  
  


  // ----------------------- COVARIANCE AND SAMPLEPDF OBJECTS ---------------------------------------- //

  gStyle -> SetPalette(1);
  gStyle -> SetOptStat(0);

  std::cout << "OUTFILE NAME: " << OutfileName.c_str() << std::endl;

  // make file to save plots
  TFile *Outfile = new TFile(OutfileName.c_str() , "RECREATE");

  covarianceXsec *xsec;
  covarianceOsc *osc;


  xsec = new covarianceXsec(XsecMatrixName.c_str(), XsecMatrixFile.c_str());


  std::cout << "---------- Printing off nominal parameter values ----------" << std::endl;
  std::cout << "Cross section parameters:" << std::endl;
  xsec->printNominal();

  std::cout << "---------- Finished printing nominal parameter values ----------" << std::endl;


  xsec->setParameters();

  //xsec->setStepScale(fitMan->getXsecStepScale());
  xsec->setStepScale(0.01);

  osc = new covarianceOsc(OscMatrixName.c_str(), OscMatrixFile.c_str());

  // oscpars from manager in order:
  // sin2th_12, sin2th_23, sin2th_13, delm2_12, delm2_23, delta_cp
  //std::vector<double> oscpars =fitMan->getOscParameters();   
  std::vector<double> oscpars{0.307,0.528,0.0218,7.53e-5, 2.509e-3,-1.601}; // Asimov A

  std::cout<<"Using these oscillation parameters: ";
  for(unsigned ipar=0;ipar<oscpars.size();ipar++) std::cout<<" "<<oscpars.at(ipar);
  std::cout << std::endl;

  osc->setFlipDeltaM23(true);

  // Ask config file whether to use reactor constraint
  //std::cout << "use reactor prior is : " << useRC << std::endl ;

  // Use prior for 12 parameters only
  //osc->setEvalLikelihood(0,false);
  osc->setEvalLikelihood(1,false);
  osc->setEvalLikelihood(2,false);
  //osc->setEvalLikelihood(3,false);
  osc->setEvalLikelihood(4,false);
  osc->setEvalLikelihood(5,false);
  // This line gives a crash and stack trace...
  osc->setParameters(oscpars);
  osc->acceptStep();

  // Set some sample....
  std::vector<samplePDFDUNENDBase*> pdfs;
  //!!add pdfs vector to make things easier
  samplePDFDUNENDBase *nd_sample = new samplePDFDUNENDBase(1.3628319e+23, "configs/SamplePDFDune_ND_test.yaml", xsec);
  
  pdfs.push_back(nd_sample);

  for(unsigned ipdf=0;ipdf<pdfs.size();ipdf++){
    pdfs[ipdf]->setUseBeta(false);   // Set whether to use beta in oscillation probability calculation
    pdfs[ipdf]->setApplyBetaNue(false);   // Set option to apply beta to nue appearance probability instead of nuebar appearance probability
    pdfs[ipdf]->setApplyBetaDiag(false);        // Set option to apply (1/beta) to nue appearance probability and beta to nuebar appearance probability
    pdfs[ipdf]->useNonDoubledAngles(true);
  }

  // unosc
  std::vector<double> oscpars_un(oscpars);
  oscpars_un[0] = 0;
  oscpars_un[1] = 0;
  oscpars_un[2] = 0;

  // Oscillated
  osc -> setParameters(oscpars_un);
  std::cout << "oscpars[0] = " << (osc -> getPropPars())[0] << std::endl
	    << "oscpars[1] = " << (osc -> getPropPars())[1] << std::endl
	    << "oscpars[2] = " << (osc -> getPropPars())[2] << std::endl
	    << "oscpars[3] = " << (osc -> getPropPars())[3] << std::endl
	    << "oscpars[4] = " << (osc -> getPropPars())[4] << std::endl
	    << "oscpars[5] = " << (osc -> getPropPars())[5] << std::endl;

  //std::vector<double> CAFana_default_edges = get_default_CAFana_bins();
  //numu_pdf -> useBinnedOscReweighting(true, CAFana_default_edges.size()-1, &CAFana_default_edges[0]);

  //TH1D	*numu_nominal_hist	= (TH1D*)numu_pdf      -> get1DHist() -> Clone("numu_nom");


  // Unoscillated
  /*
  osc -> setParameters(oscpars_un);
  vector<double> xsecpar = xsec->getNominalArray();
  xsec->setParameters(xsecpar);
  numu_pdf->reweight(osc->getPropPars(), osc->getPropPars());
  TH1D	*numu_nominal_hist	= (TH1D*)numu_pdf      -> get1DHist() -> Clone("numu_nom");
  nue_pdf->reweight(osc->getPropPars(), osc->getPropPars());
  TH1D	*nue_nominal_hist	= (TH1D*)nue_pdf      -> get1DHist() -> Clone("numu_nom");


  xsecpar[0] = xsecpar[0] + sqrt((*xsec->getCovMatrix())(0,0));
  xsec->setParameters(xsecpar);
  numu_pdf->reweight(osc->getPropPars(), osc->getPropPars());
  TH1D	*numu_shift_pos	= (TH1D*)numu_pdf      -> get1DHist() -> Clone("numu_maqe_shift");

  xsecpar = xsec->getNominalArray();
  xsecpar[0] = xsecpar[0] - sqrt((*xsec->getCovMatrix())(0,0));
  xsec->setParameters(xsecpar);
  numu_pdf->reweight(osc->getPropPars(), osc->getPropPars());
  TH1D	*numu_shift_neg	= (TH1D*)numu_pdf      -> get1DHist() -> Clone("numu_maqe_shift");

  TLegend *leg = new TLegend(0.6, 0.7, 0.9, 0.9);
  leg->AddEntry(numu_nominal_hist, "Nominal", "l");
  leg->AddEntry(numu_shift_pos, "MaQE +1#sigma", "l");
  leg->AddEntry(numu_shift_neg, "MaQE -1#sigma", "l");
  leg->SetTextSize(0.03);

  TCanvas *nomcanv = new TCanvas("nomcanv","",1200,600);
  //nomcanv->Divide(2,1);
  //nomcanv->cd(1);
  

  numu_shift_pos->SetTitle("MaQE Sigma Shifts");
  numu_shift_pos->GetXaxis()->SetTitle("reco energy");

  numu_shift_pos->SetLineColor(kBlue);
  numu_shift_neg->SetLineColor(kRed);
  

  numu_shift_pos->Draw("HIST");
  numu_nominal_hist->Draw("HIST SAME");
  //nomcanv->cd(2);
  numu_shift_neg->Draw("HIST SAME");
  leg->Draw();

  std::string plotname = "Dummy_Hist" ;
  saveCanvas(nomcanv, plotname,"_maqe") ;

  Outfile -> cd();
  numu_nominal_hist			-> Write("numu_nominal_hist");
  nue_nominal_hist				-> Write("nue_nominal_hist");


  std::cout	<< "Integrals of nominal hists: " << std::endl
    		<< "Numu before shift:        " << nue_nominal_hist		-> Integral() << std::endl
    		<< "Numu before shift:        " << numu_nominal_hist		-> Integral() << std::endl; */


  return 0;
 }
