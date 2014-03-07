// @(#)root/tmva $Id: MethodPlugins.cxx 29195 2009-06-24 10:39:49Z brun $ 
// Author: Andreas Hoecker, Joerg Stelzer, Helge Voss, Kai Voss 

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate Data analysis       *
 * Package: TMVA                                                                  *
 * Class  : TMVA::MethodPlugins                                                *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation (see header for description)                               *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *      Kai Voss        <Kai.Voss@cern.ch>       - U. of Victoria, Canada         *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         * 
 *      U. of Victoria, Canada                                                    * 
 *      MPI-K Heidelberg, Germany                                                 * 
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

//_______________________________________________________________________
/* Begin_Html

  Plugins analysis                       
  
  <p>
  The MethodPlugins is actually not a real method, but it is just a wrapper to call the TPluginsManager of ROOT and find 
  a external method which can be used to extend TMVA by another classifier. The only methods which are actually really implemented are the 
  constructors which fulfill the plugins handling. The others will produce FATAL warnings and stop TMVA execution.
  <p>
  Right after the constructor, the additional method 'getPlugedinMethod()' is called, which returns the method loaded by the plugin manager,
  and the MethodPlugins object is already deleted.

End_Html */
//_______________________________________________________________________

#include "TPluginManager.h"
#include "TROOT.h"

#include "TMVA/ClassifierFactory.h"
#include "TMVA/Tools.h"
#include "TMVA/Ranking.h"

namespace 
{ 
      TMVA::IMethod* CreateMethodPlugins(const TString& jobName, const TString& methodTitle, TMVA::DataSetInfo& theData, const TString& theOption)
      { 
        //std::cout << "CreateMethodPlugins is called with options : '" << jobName << "', '" << methodTitle<< "', " << theOption<< "'" << std::endl;
	TPluginManager *pluginManager(0);
	TPluginHandler *pluginHandler(0);
	pluginManager = gROOT->GetPluginManager();
	//An empty methodTitle is a Problem for the PluginHandler, so we need to fiddle it out of the weightsfilename
	TString myMethodTitle;
	if(jobName=="" && methodTitle=="") { //This is most likely a call to the Classifier (not for training)
		myMethodTitle = theOption.Copy();
		Ssiz_t firstUnderscore = myMethodTitle.First('_');
		Ssiz_t firstPoint      = myMethodTitle.Last('.');
		myMethodTitle.Remove(firstPoint,myMethodTitle.Length() - firstPoint);
		myMethodTitle.Remove(0,firstUnderscore-1); //leave the underscore
	}
	else myMethodTitle = methodTitle;
	pluginHandler = pluginManager->FindHandler("TMVA@@MethodBase", myMethodTitle);
	if(pluginHandler){
	//	std::cout << "pluginHandler found" << std::endl;
		if (pluginHandler->LoadPlugin() == 0) {
			if(jobName=="" && methodTitle=="") { 
				//std::cout << "Calling ExpertPlugin " << std::endl;
				return (TMVA::IMethod*) pluginHandler->ExecPlugin(2, &theData, &theOption);
			} else {  
				//std::cout << "Calling TeacherPlugin " << std::endl;
				return (TMVA::IMethod*) pluginHandler->ExecPlugin(4, &jobName, &methodTitle, &theData, &theOption);
			}  
		}
	} else {
		std::cerr <<  "Couldn't find plugin handler for TMVA@@MethodBase and " << methodTitle << std::endl; 
	}
      }

      Bool_t RegisteredMethod = TMVA::ClassifierFactory::Instance(). 
	      Register("Plugins", CreateMethodPlugins);                         
      Bool_t AddedTypeMapping = TMVA::Types::Instance().AddTypeMapping(TMVA::Types::kPlugins, "Plugins"); 
}
