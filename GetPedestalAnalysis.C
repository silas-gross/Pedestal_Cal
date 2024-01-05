#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <HCalPedestalChannels.h>
#include "sPhenixStyle.h"
#include "sPhenixStyle.C"
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4allraw/Fun4AllPrdfInputManager.h>
#include <fun4allraw/Fun4AllPrdfInputPoolManager.h>
#include <fun4allraw/SinglePrdfInput.h>
#include <fun4all/SubsysReco.h>
#include <sstream>
#include <string.h>
#include <G4_Global.C>

R__LOAD_LIBRARY(libfun4all.so);
R__LOAD_LIBRARY(libfun4allraw.so);
R__LOAD_LIBRARY(libcalo_io.so);
R__LOAD_LIBRARY(libHCalPedestalChannels.so);
R__LOAD_LIBRARY(libffamodules.so);
R__LOAD_LIBRARY(libffarawmodules.so);

int GetPedestalAnalysis(std::string filename="/sphenix/tg/tg01/jets/ahodges/run23_production/21518/DST-00021518-0000.root")
{
	//taking in a data file and will run over all data 
	//std::cout<<"input of " <<argc <<std::endl;
	//std::string filename (argv[1]);
//	std::string filename="/sphenix/tg/tg01/jets/ahodges/run23_production/21518/DST-00021518-0000.root";
	std::cout<<"Input file is " <<filename <<std::endl;
	SetsPhenixStyle();
	Fun4AllServer *se=Fun4AllServer::instance();
	Fun4AllPrdfInputPoolManager *inp = new Fun4AllPrdfInputPoolManager("inp");
	Fun4AllDstInputManager *in = new Fun4AllDstInputManager("in");
	//in->AddFile(filename);
	HCalPedestalChannels* c =new HCalPedestalChannels("HCalPedestalChannels");
	int is_input=0;
	//load in the DST Segment and run number
	if(filename.find("prdf") != std::string::npos){
		inp->AddPrdfInputList(filename)->MakeReference(true);
		se->registerInputManager(inp);
	}
	se->registerSubsystem(c);
	se->run();
	std::cout<<"Writing to file"<<std::endl;
	return 0;				
}
#endif	
