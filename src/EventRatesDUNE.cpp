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

#include "samplePDFDUNE/samplePDFDUNEBase.h"
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
  double POT = fitMan->raw()["General"]["POT"].as<double>(); 

  // Asimov fit
  bool asimovfit = false;//fitMan->GetAsimovFitFlag();
  
 
  // ----------------------- COVARIANCE AND SAMPLEPDF OBJECTS ---------------------------------------- //

  gStyle -> SetPalette(1);

  // make file to save plots
  TFile *Outfile = new TFile(fitMan->raw()["General"]["Output"]["FileName"].as<std::string>().c_str() , "RECREATE");

  //initialise xsec
  covarianceXsec *xsec = new covarianceXsec(XsecMatrixName.c_str(), XsecMatrixFile.c_str());

  std::cout << "---------- Printing off nominal parameter values ----------" << std::endl;
  std::cout << "Cross section parameters:" << std::endl;
  xsec->printNominal();

  bool XsecParsAtGen = fitMan->raw()["General"]["Systematics"]["XsecAtGen"].as<bool>();

  std::cout << "---------- Finished printing nominal parameter values ----------" << std::endl;

  covarianceOsc *osc = new covarianceOsc(OscMatrixName.c_str(), OscMatrixFile.c_str());

  // oscpars from manager in order:
  // sin2th_12, sin2th_23, sin2th_13, delm2_12, delm2_23, delta_cp
  //std::vector<double> oscpars =fitMan->getOscParameters();   
  std::vector<double> oscpars{0.307,0.528,0.0218,7.53e-5, 2.509e-3,-1.601}; // Asimov A
  //std::vector<double> oscpars{0.3097,0.580,0.02241,7.39e-5, 2.4511e-3, 3.752}; // NuFit
  //OSCPARAM = [0.3097,0.580,0.02241,7.39e-5,2.4511e-3, 3.752]

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

  std::vector<samplePDFDUNEBase*> SamplePDFs;

  // Set some sample....
  samplePDFDUNEBase * numu_pdf = new samplePDFDUNEBase(POT, "configs/SamplePDFDune_FHC_numuselec.yaml", xsec);
  SamplePDFs.push_back(numu_pdf);
  //samplePDFDUNEBase * nue_pdf = new samplePDFDUNEBase(POT, "configs/SamplePDFDune_FHC_nueselec.yaml", xsec);
  //SamplePDFs.push_back(nue_pdf);

  // Oscillated
  osc -> setParameters(oscpars);
  std::cout << "oscpars[0] = " << (osc -> getPropPars())[0] << std::endl
	    << "oscpars[1] = " << (osc -> getPropPars())[1] << std::endl
	    << "oscpars[2] = " << (osc -> getPropPars())[2] << std::endl
	    << "oscpars[3] = " << (osc -> getPropPars())[3] << std::endl
	    << "oscpars[4] = " << (osc -> getPropPars())[4] << std::endl
	    << "oscpars[5] = " << (osc -> getPropPars())[5] << std::endl;


  // unosc
  std::vector<double> oscpars_un(oscpars);
  oscpars_un[0] = 0;
  oscpars_un[1] = 0;
  oscpars_un[2] = 0;
  oscpars_un[3] = 0;
  oscpars_un[4] = 0;

  //Setup the cross-section parameters
  //This should get the prior values.
  std::vector<double> XsecParVals = xsec->getNominalArray();

  if(XsecParsAtGen){
	TFile* XsecFile = new TFile(XsecMatrixFile.c_str(), "READ");
	TVectorD* XsecGeneratedParamArray = (TVectorD*)XsecFile->Get("xsec_param_nom");
	std::cout << "Setting xsec systs to their generated values " << std::endl;
	for (unsigned param_i = 0 ; param_i < XsecParVals.size() ; ++param_i) {
	  std::cout << "Generated value for param " << param_i << " is " << (*XsecGeneratedParamArray)(param_i) << std::endl;
	  XsecParVals[param_i] = (*XsecGeneratedParamArray)(param_i);
	  std::cout << "Set parameter " << param_i << " to value " << XsecParVals[param_i] << std::endl;
	}
  }
  else{
	std::cout << "Keeping xsec parameters at their prior values" << std::endl;
  }

  xsec->setParameters(XsecParVals);
  //xsec->setStepScale(fitMan->getXsecStepScale());
  xsec->setStepScale(0.01);


  //Some place to store the histograms
  std::vector<TH1D*> oscillated_hists;
  std::vector<TH1D*> unoscillated_hists;
  std::vector<std::string> sample_names;

  for (unsigned sample_i = 0 ; sample_i < SamplePDFs.size() ; ++sample_i) {

	std::string name = SamplePDFs[sample_i]->GetSampleName();
	sample_names.push_back(name);
	TString NameTString = TString(name.c_str());
	// Unoscillated
	osc -> setParameters(oscpars_un);
	osc -> acceptStep();

	SamplePDFs[sample_i] -> reweight(osc->getPropPars());
	TH1D *numu_unosc = (TH1D*)SamplePDFs[sample_i] -> get1DHist() -> Clone(NameTString+"_unosc");
	unoscillated_hists.push_back(numu_unosc);

	TCanvas *nomcanv = new TCanvas("nomcanv","",1200,600);
	numu_unosc -> Draw("HIST");

	std::string plotname = "Dummy_Hist" ;
	saveCanvas(nomcanv, plotname,"_nominal_spectra") ;

	Outfile->cd();
	numu_unosc->Write("numu_unosc");

	osc->setParameters(oscpars);
	osc->acceptStep();
	// Oscillated
	std::cout << "oscpars[0] = " << (osc -> getPropPars())[0] << std::endl
	  << "oscpars[1] = " << (osc -> getPropPars())[1] << std::endl
	  << "oscpars[2] = " << (osc -> getPropPars())[2] << std::endl
	  << "oscpars[3] = " << (osc -> getPropPars())[3] << std::endl
	  << "oscpars[4] = " << (osc -> getPropPars())[4] << std::endl
	  << "oscpars[5] = " << (osc -> getPropPars())[5] << std::endl;


	SamplePDFs[sample_i] -> reweight(osc->getPropPars());
	TH1D *numu_osc = (TH1D*)SamplePDFs[sample_i] -> get1DHist()->Clone(NameTString+"_osc");
	oscillated_hists.push_back(numu_osc);

  }

  //Now print out some event rates, we'll make a nice latex table at some point 
  for (unsigned sample_i = 0; sample_i < SamplePDFs.size() ; ++sample_i) {
	std::cout << "Integrals of nominal hists: " << std::endl;
	std::cout << sample_names[sample_i].c_str() << " unosc:      " << unoscillated_hists[sample_i]-> Integral() << std::endl;
	std::cout << sample_names[sample_i].c_str() << "   osc:      " << oscillated_hists[sample_i]-> Integral() << std::endl; 
	std::cout << "~~~~~~~~~~~~~~~~" << std::endl;
  }

  return 0;
 }
