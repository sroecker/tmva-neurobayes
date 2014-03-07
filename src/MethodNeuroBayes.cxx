/****************************************************************
 * This is a test class to evaluate the needs and musts to add my own
 * Classifier (e.g. NeuroBayes) to the TMVA Toolkit.
 *
 * Daniel Martschei 2007, Oct Karlsruhe, Germany, EKP
 *
 * 
 *
 * *************************************************************/

#include <iostream>
#include <fstream>
#include <TSystem.h>
#include <TString.h>
#include <TObjString.h>
#include <TDirectory.h>
#include "TMVA/Ranking.h"
#include "TMVA/Tools.h"
#include "TMVA/Timer.h"
#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#include "TMVA/ClassifierFactory.h"
#endif
#include <RVersion.h>

#include "MethodNeuroBayes.h"

ClassImp(TMVA::MethodNeuroBayes)

int TMVA::MethodNeuroBayes::CountInstanzes = 0;

TMVA::MethodNeuroBayes::MethodNeuroBayes(DataSetInfo& theData, 
                                       const TString& theWeightFile,  
                                       TDirectory* theTargetDir)
	:TMVA::MethodBase( Types::kPlugins, theData, theWeightFile, theTargetDir ){
	fTask=0;

	InitNeuroBayes(fTask);
	Log() << kINFO << "Expert Constructor was called" << Endl;
} 

TMVA::MethodNeuroBayes::MethodNeuroBayes( const TString& jobName,
					  const TString& methodTitle, 
					  DataSetInfo& theData, 
					  const TString& theOption, 
					  TDirectory* theTargetDir )
	   : TMVA::MethodBase( jobName, Types::kPlugins, methodTitle, theData, theOption, theTargetDir )
{
	fTask = 1;
	InitNeuroBayes(fTask);
	MyID = CountInstanzes;
	//Log() << kINFO << methodTitle << " got ID " << MyID << " theTargetDir =  " << theTargetDir << Endl;

	CountInstanzes++;
	if(MyID > 0) {
		Log() << kWARNING << "This NeuroBayes instance was not created as first one."<<Endl;
		Log() << kWARNING << "Because Teacher is a singleton you won't get useful results from this Method" << Endl;
		Log() << kWARNING << "Use only one Teacher at a time until this is fixed" << Endl;
	}

	TeacherConfigured = false;
	Log() << kINFO << "Teacher Constructor was called" << Endl;
}

TMVA::MethodNeuroBayes::~MethodNeuroBayes(){
	delete Net;
}

Bool_t TMVA::MethodNeuroBayes::HasAnalysisType( Types::EAnalysisType type, UInt_t numberClasses, UInt_t numberTargets )
{
	//NeuroBayes handles Classification
	if( type == Types::kClassification && numberClasses == 2 ) return kTRUE;
	return kFALSE;
}

//_______________________________________________________________________
void TMVA::MethodNeuroBayes::Init(){}

void TMVA::MethodNeuroBayes::InitNeuroBayes( int task ) //task = 1 means Teacher, task = 0 means Expert 
{
	TString filedir = "weights";
	NBOutputFile = filedir + "/" +  GetJobName() + "_" + GetMethodName() + ".NB_weights" ;
	
	if( task == 1 ) {
		//set an instance of Teacher
		nb = NeuroBayesTeacher::Instance();
	
		nb->SetOutputFile(NBOutputFile + ".nb");  	// expert file

  		//setup network topology
  		const int  nvar = GetNvar();    //number of input variables
  		nb->NB_DEF_NODE1(nvar+1);       // nodes in input layer 
  		nb->NB_DEF_NODE2(nvar+2);      	// nodes in hidden layer 
  		nb->NB_DEF_NODE3(1);       	// nodes in output layer
  
  		nb->NB_DEF_TASK("CLA");    	// binominal classification

  		int i= 4701;
  		int j=21; 
  		nb->NB_RANVIN(i,j,2);   	// random number seed initialisation,
						// i has to be an odd number, the third argument is a debugging flag
	}
	else if ( task == 0 ) {
	}
}


//_______________________________________________________________________
void TMVA::MethodNeuroBayes::InitEventSample( void )
{
   	if (!HasTrainingTree()) Log() << kFATAL << "<Init> Data().TrainingTree() is zero pointer" << Endl;

   	Int_t nevents = Data()->GetNTrainingEvents();
	Int_t nsignal=0;

	// event loop on Training Data
 	for (Int_t ievt=0; ievt<nevents; ievt++) {

		//ReadTrainingEvent( ievt,  Types::kTrueType );
		//const Event* origEv = Data()->GetEvent(ievt);
		Event* event = new Event( *GetTrainingEvent(ievt) );

   		int nvar = GetNvar();

		float InputArray[nvar];   
		memset(InputArray,-999,nvar*sizeof(float)); //reset

		nb->SetWeight(event->GetWeight());  //set weight of event

      		// set Target
		Double_t type = 0.0;
		if( DataInfo().IsSignal(event) ) { type = 1.0; nsignal++; }
      		nb->SetTarget( type ) ; // Type is 1 for Signal, 0 for Background 

		for (UInt_t ivar=0; ivar<GetNvar(); ivar++) {
	        	Float_t value  = event->GetValue(ivar);
			InputArray[ivar] = value;
		}
		//pass input to NeuroBayses
          	nb->SetNextInput(nvar,InputArray);
   	}
	Log() << kINFO << "<InitEventSample> : found " << 
		nsignal << " Signal Events and " << nevents - nsignal << " Background Events " << Endl;
}

void TMVA::MethodNeuroBayes::DeclareOptions() 
{
	// define the options (their key words) that can be set in the option string 
	// know options:

	Log() << kINFO <<  "Declare NeuroBayes Options" << Endl;
	DeclareOptionRef(frunAnalysis=kTRUE, "Analysis", "You may chose whether you want to run the NeuroBayes analysis macro or not (default=yes)");
	DeclareOptionRef(fRegularisation="REG", "Regularisation", "Type of regularisation: Possible choices are: OFF , REG (default), ARD , ASR, ALL.");
	AddPreDefVal(TString("OFF"));
	AddPreDefVal(TString("REG"));
	AddPreDefVal(TString("ARD"));
	AddPreDefVal(TString("ASR"));
	AddPreDefVal(TString("ALL"));

	DeclareOptionRef(fPreprocessing=112, "Preprocessing", "Type of preprocessing: Refer to NeuroBayes-Howto A.1 for Details.");

	DeclareOptionRef(fLossFunc="ENTROPY", "LossFunc", "Type of loss-function: Possible choices are ENTROPY (default), QUADRATIC and COMBINED .");
	AddPreDefVal(TString("ENTROPY"));
	AddPreDefVal(TString("QUADRATIC"));
	AddPreDefVal(TString("COMBINED"));

	DeclareOptionRef(fShapeTreat="OFF", "ShapeTreat", "Shape treatment: Possible choices are OFF, INCL, MARGINAL, DIAG and TOL.");
	AddPreDefVal(TString("OFF"));
	AddPreDefVal(TString("INCL"));
	AddPreDefVal(TString("DIAG"));
	AddPreDefVal(TString("TOL"));

	DeclareOptionRef(fMomentum=0, "Momentum", "Optionally, a momentum can be specified for the training.");

	DeclareOptionRef(fWeightUpdate=200, "WeightUpdate", "Weight update: Normally, the weights are updated every 200 events.");

	DeclareOptionRef(fNtrainingIter=100, "NtrainingIter", "Number of training iterations: This parameter defines the number of training iterations.");
	DeclareOptionRef(fLearingSpeed=1.0, "LearingSpeed", "Increase learning speed by a multiplicative factor.");

	DeclareOptionRef(fLimitLearingSpeed=1.0, "LimitLearingSpeed", "Limit learning speed: The maximal learning speed may be limited");

	DeclareOptionRef(fTrainingMethod="NOBFGS", "TrainingMethod", "Training Method: It is possible to use the BFGS algorithm");
	AddPreDefVal(TString("BFGS"));
	AddPreDefVal(TString("NOBFGS"));

	DeclareOptionRef(fNBIndiPreproFlagList="", "NBIndiPreproFlagList", "Set individual preprocessing flags in a coma seperated string,  e.g. 12,12,12,12. See NeuroBayes-HowTo for infos");
	DeclareOptionRef(fNBIndiPreproFlagVarname="", "NBIndiPreproFlagByVarname", "Set individual preprocessing flags in a coma seperated string, e.g. varname1=<PreproFlag>.<1stPreproparam>.<2ndPreproparam>...,varname2=19,...");
}

void TMVA::MethodNeuroBayes::ProcessOptions()
{
   Log()<< "Processing Options" << Endl;
   // decode the options in the option string
	if(fTask == 1 && TeacherConfigured==false) {
		//MethodBase::ProcessOptions();
   		//possible way to set NeuroBayes Layout and PreproFlags

 		nb->NB_DEF_PRE(fPreprocessing);			// Global Preprocessing Flag 
  		nb->NB_DEF_REG(fRegularisation);           	// 'OFF','REG' (def) ,'ARD','ASR','ALL'
  		nb->NB_DEF_LOSS(fLossFunc);      		// 'ENTROPY'(def),'QUADRATIC'
  		nb->NB_DEF_SHAPE(fShapeTreat);        		// 'OFF', 'INCL', 'TOTL'

  		nb->NB_DEF_EPOCH(fWeightUpdate);           	// weight update after n events
  		nb->NB_DEF_MOM(fMomentum);	           	// Momentum 
  
  		nb->NB_DEF_SPEED(fLearingSpeed);           	// multiplicative factor to enhance global learning speed
  		nb->NB_DEF_MAXLEARN(fLimitLearingSpeed);     	// multiplicative factor to limit the global learning speed
								// in any direction, this number should be smaller than NB_DEF_SPEED

  		nb->NB_DEF_ITER(fNtrainingIter);             	// number of training iteration
  		nb->NB_DEF_METHOD(fTrainingMethod);            	// Training Method
		//nb->NB_DEF_INITIALPRUNE(fPruning);
		//nb->NB_DEF_RTRAIN(fTrainTestRatio);		// Ratio of Events to use for Trainig, Rest is used for Testing

		preproFlagsarray = new TString [GetNvar()];
		for (UInt_t ivar=0; ivar<GetNvar(); ivar++) preproFlagsarray[ivar] = "";
		if( fNBIndiPreproFlagList!="" ) {
			ParseIndiviPreproFlagFromList();

		}
		else if( fNBIndiPreproFlagVarname!="" ) {
			ParseIndiviPreproFlagByVarname();
		}
		TeacherConfigured = true;
	}
	else Log() << kWARNING << "Teacher already configured or not in Trainingmode. No configurations done!" << Endl;
}

//Train the Net
void TMVA::MethodNeuroBayes::Train( void ){
	InitNeuroBayes(fTask);
	InitEventSample();
	//perform training
	Log() << kINFO << "To see NeuroBayes output have a look at \"nb_teacher.log\"" << Endl;
	Timer timer( 1, GetName() );
	timer.DrawProgressBar( 0 );
	int original = dup(fileno(stdout));
   	fflush(stdout);
   	freopen("nb_teacher.log", "w", stdout);
 	nb->TrainNet();

	fflush(stdout);
   	dup2(original, fileno(stdout));
   	close(original);
	timer.DrawProgressBar( 1 );

	if(frunAnalysis) {
		runAnalysis();
	}
	//Setup Expert, it might be needed...
	Net = new Expert(TString(NBOutputFile + ".nb").Data());
}

// write weights to file
void TMVA::MethodNeuroBayes::WriteWeightsToStream( ostream& o )const {
	o << "# NeuroBayes stores its weights in its own file :" << NBOutputFile + ".nb" << std::endl;
}

void TMVA::MethodNeuroBayes::AddWeightsXMLTo(void* parent) const {
	void* expertise = gTools().xmlengine().NewChild(parent, 0, "Weights");
	gTools().AddAttr(expertise, "NVariables", GetNvar());
	void* filenode = gTools().xmlengine().NewChild(expertise, 0, "Expertise");

	gTools().AddAttr(filenode, "File", NBOutputFile + ".nb");
}

//Is abused to setup the Expert
void TMVA::MethodNeuroBayes::ReadWeightsFromStream( istream& istr ){
	char buf[512];
	istr.getline(buf,512);
	TString filename = ((TObjString *)TString(buf).Tokenize(":") ->At(1))->GetString();

	Log() << kINFO << "Setting up NB Expert" << Endl;
	if(filename.CompareTo("noFile.nb") == 0) Log() << kWARNING << GetMethodName() << 
		" is not trained because it was not the first booked NeuroBayesTeacher. Please repeat training." << Endl;
	else Net = new Expert(filename);
	Log() << kINFO << "Set up NB Expert done" << Endl;
}

void TMVA::MethodNeuroBayes::ReadWeightsFromXML(void* weightnode){
	Log() << kINFO << "Setting up NB Expert" << Endl;
	TString expertiseFile;
	void* filenode = gTools().xmlengine().GetChild(weightnode);
	gTools().ReadAttr(filenode, "File",expertiseFile);
	Log() << kINFO << "Setting up NB Expert " << expertiseFile << Endl;
	if(expertiseFile.CompareTo("noFile.nb") == 0) Log() << kWARNING << GetMethodName() << 
		" is not trained because it was not the first booked NeuroBayesTeacher. Please repeat training." << Endl;
	else Net = new Expert(expertiseFile.Data());
	Log() << kINFO << "Set up NB Expert done" << Endl;
}

void TMVA::MethodNeuroBayes::GetHelpMessage()const {
	Log() << "NeuroBayes help can be found at www.neurobayes.de" << Endl;
}

// ranking of input variables
const TMVA::Ranking* TMVA::MethodNeuroBayes::CreateRanking(){
	//Give a ranking of InputVariables back, descending in importance for classification	
	//Not working
	return 0;
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(5,28,0)
Double_t TMVA::MethodNeuroBayes::GetMvaValue( Double_t* errLower, Double_t* errUpper )
#else
Double_t TMVA::MethodNeuroBayes::GetMvaValue( Double_t* errLower)
#endif
{
	 Double_t myMVA = 0;
	 Double_t var[GetNvar()];
	 const Event* ev = Data()->GetEvent();
	 for (UInt_t ivar=0; ivar<GetNvar(); ivar++) {
	 	var[ivar] = ev->GetValue(ivar);
	 }

	 myMVA = Net->nb_expert(var);
	 return myMVA;
}

/* Helper Function to parse individual Preproflags */

void TMVA::MethodNeuroBayes::ParseIndiviPreproFlagFromList(){

	TObjArray* PreproFlags = fNBIndiPreproFlagList.Tokenize(","); // Split String at commata
	UInt_t NPrePros = PreproFlags->GetEntriesFast();

	if( NPrePros != GetNvar() ) { 
		Log() << kFATAL << "You set " << NPrePros << " preprocssing flag(s), but there are " << GetNvar() << " variables. "
		<< " Please set flag \"0\" if you don't want special preprocessing for a certain variable. "  << Endl;
	}

	Log() << kINFO << "You set the following Preprocessing Flags for the Variables"<< Endl;
	for (UInt_t ivar=0; ivar<GetNvar(); ivar++) {

		TObjArray* FlagParaArray = ((TObjString *)PreproFlags->At(ivar))->GetString().Tokenize(".");
		Int_t preproflag = ((TObjString *)FlagParaArray->At(0))->GetString().Atoi();

		if(preproflag != 0 ) {

			Log() << kINFO << "Preprocessing for variable " << GetInternalVarName(ivar) << " is set to " 
			<< preproflag << Endl;
				
			nb->SetIndividualPreproFlag(ivar,preproflag); //Set Individual Preprocessing Flag
			preproFlagsarray[ivar] += ((TObjString *)FlagParaArray->At(0))->GetString()  + " ";
			if(FlagParaArray->GetEntriesFast() > 1) SetIndividualPreproParam(ivar, FlagParaArray);
		}
		else { 
			Log() << kINFO << "No individual preprocessing for variable " << GetInternalVarName(ivar) << Endl; 
			preproFlagsarray[ivar] = "";
		}
	}
}
void TMVA::MethodNeuroBayes::ParseIndiviPreproFlagByVarname(){
	TObjArray* PreproFlagPairs = fNBIndiPreproFlagVarname.Tokenize(","); // Split String at commata
	Int_t NPrePros = PreproFlagPairs->GetEntriesFast();

	Log() << kINFO << "You set the following Preprocessing Flags for the Variables"<< Endl;
	for (Int_t iflag=0; iflag<NPrePros; iflag++) {

		TObjArray* PreproFlagPair =  ((TObjString *)PreproFlagPairs->At(iflag))->GetString().Tokenize("="); // Split String at commata
		TString PreproVarname       = ((TObjString *)PreproFlagPair->At(0))->GetString();
		TString PreproVarnameIntern = gTools().ReplaceRegularExpressions( PreproVarname, "_" ); 
		TObjArray* FlagParaArray = ((TObjString *)PreproFlagPair->At(1))->GetString().Tokenize(".");
		Int_t preproflag = ((TObjString *)FlagParaArray->At(0))->GetString().Atoi();
		if(preproflag != 0 ) {
			UInt_t ivar = 0;
			for (ivar=0; ivar<GetNvar()+1; ivar++) {
				if(ivar<GetNvar()) if (PreproVarnameIntern == GetInternalVarName(ivar)) break;
			}
			if (ivar>=GetNvar()) Log() << kWARNING << PreproVarname << " not found in variable list " << ivar << Endl;
			else {
				Log() << kINFO << "Preprocessing for variable ( " << PreproVarname << " #" << ivar << "  intern= " << PreproVarnameIntern << " ) is set to " 
			<< preproflag << Endl;
			
			nb->SetIndividualPreproFlag(ivar,preproflag); //Set Individual Preprocessing Flag
			preproFlagsarray[ivar] += ((TObjString *)FlagParaArray->At(0))->GetString() + " ";
			if(FlagParaArray->GetEntriesFast() > 1) SetIndividualPreproParam(ivar, FlagParaArray);
			}
		}
	}
}

void TMVA::MethodNeuroBayes::SetIndividualPreproParam(Int_t ivar, TObjArray* ParamArray){
	Log() << kINFO << "Found Prepro parameter for variable " << GetInternalVarName(ivar) << Endl; 
	for (Int_t ipara=1; ipara < ParamArray->GetEntriesFast(); ipara++) {
		int preproparam = ((TObjString *)ParamArray->At(ipara))->GetString().Atoi();
		nb->SetIndividualPreproParameter(ivar, ipara-1, preproparam);
	Log() << kINFO << "Setting parameter # " << ipara  << " for variable " << GetInternalVarName(ivar) 
	<< " to " << preproparam  << Endl; 
		preproFlagsarray[ivar] += preproparam + " ";
	}
}

/*-------Methods for generating analysis.ps-------------------*/
void TMVA::MethodNeuroBayes::runAnalysis() {
		std::cout << "fInputVars size" << fInputVars->size() << std::endl;
		std::cout << GetInternalVarName(0) << std::endl;
	// check if log file exists. Should also check if it is new enough
	std::ifstream log("nb_teacher.log");
	if (log.is_open()) { log.close();}
	else 
	{
		Log() << kWARNING << "the logfile \"nb_teacher.log\" was not found! " << Endl;
		frunAnalysis = kFALSE;
		log.close();
		return;
	}
	//generate the correl_signi files
	char** c_varnames;
	c_varnames = new char*[GetNvar()];
	for(unsigned int ivar=0; ivar< GetNvar(); ivar++) {
		/*
		std::cout << "Nvar = " << GetNvar() << std::endl;
		std::cout << GetInternalVarName(ivar) << std::endl;
		*/
		c_varnames[ivar] = new char[GetInternalVarName(ivar).Length()+8];
		std::stringstream tmpstring;
		tmpstring << GetInternalVarName(ivar) << " " << preproFlagsarray[ivar];
		strcpy(c_varnames[ivar], tmpstring.str().c_str() );
	}
	nb->nb_correl_signi(c_varnames,"./"+GetJobName()+"correl_signi.txt","./"+GetJobName()+"correl_signi.html");

	Log() << kINFO << "Executing NeuroBayes analysis macro" << Endl;
	std::string path = "";
  	if(gSystem->Getenv("NEUROBAYES"))  path = gSystem->Getenv("NEUROBAYES");
	else Log() << kFATAL << "Variable $NEUROBAYES not found, please check your environment setup" << Endl; 

	TString PSFileName = GetJobName() + "_" + GetMethodName() + ".pdf";
	std::stringstream analysis_exec;
	analysis_exec << "root -b -q $NEUROBAYES/external/analysis.C'(\""+GetJobName()+"ahist.txt\",\"";
	analysis_exec << PSFileName;
	analysis_exec << "\",1,\""+GetJobName()+"correl_signi.txt\")'";
	//gSystem->Exec("root -b -q $NEUROBAYES/external/analysis.C'(\"ahist.txt\",\"analysis.ps\",1,\"correl_signi.txt\")' && echo \"analysis.ps was generated\""); 
	gSystem->Exec(analysis_exec.str().c_str()); 
}

void TMVA::MethodNeuroBayes::dumpPseudoCodegen(){
	std::ofstream outfile("pseudocodegen");

	outfile << "VARSET 1" << std::endl;
	for (UInt_t ivar=0; ivar<GetNvar(); ivar++) {
		outfile  << GetInternalVarName(ivar); 
		outfile << "  " << preproFlagsarray[ivar];
		outfile << std::endl;
	}
	outfile << "ENDVARSET" << std::endl;
	outfile << "NETWORK classify 1" << std::endl;
	outfile.close();
}

TMVA::IMethod* TMVA::MethodNeuroBayes::CreateMethodNeuroBayes(const TString& job, const TString& title, TMVA::DataSetInfo& dsi, const TString& option)
{
	if(job=="" && title=="") {
		return (TMVA::IMethod*) new TMVA::MethodNeuroBayes(dsi, option);
	} else {
		return (TMVA::IMethod*) new TMVA::MethodNeuroBayes(job, title, dsi, option);
	}
}

void TMVA::MethodNeuroBayes::RegisterNeuroBayes() {
	TMVA::ClassifierFactory::Instance().Register("NeuroBayes", TMVA::MethodNeuroBayes::CreateMethodNeuroBayes);
	TMVA::Types::Instance().AddTypeMapping(TMVA::Types::kPlugins, "NeuroBayes");
}

