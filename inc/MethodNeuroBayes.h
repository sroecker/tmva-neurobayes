/****************************************************************
 * This is a test class to evaluate the needs and musts to add my own
 * Classifier (e.g. NeuroBayes) to the TMVA Toolkit.
 *
 * Daniel Martschei 2009, Oct Karlsruhe, Germany, EKP
 *
 * 
 *
 * *************************************************************/

#ifndef ROOT_TMVA_MethodNeuroBayes
#define ROOT_TMVA_MethodNeuroBayes
#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#include "TMVA/IMethod.h"
//#include "TMVA/MethodLikelihood.h"
#endif

#include "NeuroBayesTeacher.hh" //NeuroBayes Header
#include "NeuroBayesExpert.hh"  //NeuroBayes Header


namespace TMVA {

	class MethodNeuroBayes : public MethodBase {

	public: 
		MethodNeuroBayes(DataSetInfo&, const TString& theWeightFile, TDirectory* theTargetDir=0);
		MethodNeuroBayes(const TString& , const TString& , DataSetInfo& , const TString& theOption, TDirectory* theTargetDir = 0);

		virtual ~MethodNeuroBayes();
	
		//Methods a class derived from MethodBase should implement
		// training method
	
	      	virtual void Train( void );

	      	// write weights to file
	      	virtual void WriteWeightsToStream( ostream& o ) const;

	      	// read weights from file
	      	virtual void ReadWeightsFromStream( istream& istr );

	      	void GetHelpMessage() const;
		virtual void DeclareOptions();

		//Process OptionString and Setup NeuroBayes
		void ProcessOptions();

	      	// ranking of input variables
		const Ranking* CreateRanking();

	      	// calculate the MVA value. Different function as of ROOT-5.28.00
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,28,0)
		virtual Double_t GetMvaValue( Double_t* errLower = 0, Double_t* errUpper = 0);
#else
		virtual Double_t GetMvaValue( Double_t* errLower = 0);
#endif
		virtual void Init();
		virtual void AddWeightsXMLTo(void*) const;
		virtual void ReadWeightsFromXML(void*);
		virtual Bool_t HasAnalysisType(TMVA::Types::EAnalysisType, UInt_t, UInt_t);
	      	//end virtual methods
		// default initialisation method called by all constructors
		void     InitNeuroBayes( int );
		static int CountInstanzes;
		
		static void RegisterNeuroBayes();


	private:
		int MyID; //Only NeuroBayes with myID=0 is trained
		void InitEventSample( void );
		void SetMethodxxx();
		std::vector<Event*>             fEventSample;     // the training events
      		std::vector<Event*>             fValidationSample;// the Validation events

		NeuroBayesTeacher* nb;
		Expert* Net;
		TString NBOutputFile;
		Int_t fTask;
		
		//Option Variables
		TString fRegularisation;
		TString fPreprocessingString;
		Int_t fPreprocessing;
		Int_t fPruning;
		TString fLossFunc;
		TString fShapeTreat;
		Float_t fMomentum;
		Int_t fWeightUpdate;
		Float_t fTrainTestRatio;
		Int_t fNtrainingIter;
		Float_t fLearingSpeed;
		Float_t fLimitLearingSpeed;
		TString fTrainingMethod;
		TString fNBIndiPreproFlagList;
		TString fNBIndiPreproFlagVarname;
		Bool_t frunAnalysis;

		Bool_t TeacherConfigured;

		TString* preproFlagsarray;

		void runAnalysis();
		void dumpPseudoCodegen();
		void ParseIndiviPreproFlagFromList();
		void ParseIndiviPreproFlagByVarname();
		void SetIndividualPreproParam(Int_t ivar, TObjArray*);
		
		static TMVA::IMethod* CreateMethodNeuroBayes(const TString& job, const TString& title, TMVA::DataSetInfo& dsi, const TString& option);

		ClassDef(MethodNeuroBayes,1) //NeuroBase interface implemented at EKP Uni Karlsruhe 
	    ;
	};
}

#endif
