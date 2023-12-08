//____________________________________________________________________________..
//
// This is a template for a Fun4All SubsysReco module with all methods from the
// $OFFLINE_MAIN/include/fun4all/SubsysReco.h baseclass
// You do not have to implement all of them, you can just remove unused methods
// here and in HCalPedestalChannels.h.
//
// HCalPedestalChannels(const std::string &name = "HCalPedestalChannels")
// everything is keyed to HCalPedestalChannels, duplicate names do work but it makes
// e.g. finding culprits in logs difficult or getting a pointer to the module
// from the command line
//
// HCalPedestalChannels::~HCalPedestalChannels()
// this is called when the Fun4AllServer is deleted at the end of running. Be
// mindful what you delete - you do loose ownership of object you put on the node tree
//
// int HCalPedestalChannels::Init(PHCompositeNode *topNode)
// This method is called when the module is registered with the Fun4AllServer. You
// can create historgrams here or put objects on the node tree but be aware that
// modules which haven't been registered yet did not put antyhing on the node tree
//
// int HCalPedestalChannels::InitRun(PHCompositeNode *topNode)
// This method is called when the first event is read (or generated). At
// this point the run number is known (which is mainly interesting for raw data
// processing). Also all objects are on the node tree in case your module's action
// depends on what else is around. Last chance to put nodes under the DST Node
// We mix events during readback if branches are added after the first event
//
// int HCalPedestalChannels::process_event(PHCompositeNode *topNode)
// called for every event. Return codes trigger actions, you find them in
// $OFFLINE_MAIN/include/fun4all/Fun4AllReturnCodes.h
//   everything is good:
//     return Fun4AllReturnCodes::EVENT_OK
//   abort event reconstruction, clear everything and process next event:
//     return Fun4AllReturnCodes::ABORT_EVENT; 
//   proceed but do not save this event in output (needs output manager setting):
//     return Fun4AllReturnCodes::DISCARD_EVENT; 
//   abort processing:
//     return Fun4AllReturnCodes::ABORT_RUN
// all other integers will lead to an error and abort of processing
//
// int HCalPedestalChannels::ResetEvent(PHCompositeNode *topNode)
// If you have internal data structures (arrays, stl containers) which needs clearing
// after each event, this is the place to do that. The nodes under the DST node are cleared
// by the framework
//
// int HCalPedestalChannels::EndRun(const int runnumber)
// This method is called at the end of a run when an event from a new run is
// encountered. Useful when analyzing multiple runs (raw data). Also called at
// the end of processing (before the End() method)
//
// int HCalPedestalChannels::End(PHCompositeNode *topNode)
// This is called at the end of processing. It needs to be called by the macro
// by Fun4AllServer::End(), so do not forget this in your macro
//
// int HCalPedestalChannels::Reset(PHCompositeNode *topNode)
// not really used - it is called before the dtor is called
//
// void HCalPedestalChannels::Print(const std::string &what) const
// Called from the command line - useful to print information when you need it
//
//____________________________________________________________________________..

#include "HCalPedestalChannels.h"

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
int n_evt;
HCalPedestalChannels::HCalPedestalChannels(const std::string &name):
 SubsysReco(name)
{
  std::cout << "HCalPedestalChannels::HCalPedestalChannels(const std::string &name) Calling ctor" << std::endl;
  n_evt++;
  
}

HCalPedestalChannels::~HCalPedestalChannels()
{
  std::cout << "HCalPedestalChannels::~HCalPedestalChannels() Calling dtor" << std::endl;
}

int HCalPedestalChannels::Init(PHCompositeNode *topNode)
{
  std::cout << "HCalPedestalChannels::Init(PHCompositeNode *topNode) Initializing" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int HCalPedestalChannels::InitRun(PHCompositeNode *topNode)
{
  std::cout << "HCalPedestalChannels::InitRun(PHCompositeNode *topNode) Initializing for Run XXX" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int HCalPedestalChannels::process_event(PHCompositeNode *topNode)
{
	n_evt++;
	std::cout << "Processing Event" <<n_evt << std::endl;
  	Event* e = findNode::getClass<Event>(topNode, "PRDF");
	for(auto pid:packets){
		try{
			e->getPacket(pid);
		}
		catch(std::exception* e){ std::cout<<"no packet with number " <<pid <<std::endl;
		Packet* p=e->getPacket(pid);
		if(!p) continue;
	       #pragma opm for private(c, channel_data)
		for(int c=0; c<p->iValue(0, "CHANNELS"); c++)
		{
		 	std::vector<int> channel_data;	
			 for(auto s=0; s<31; s++)
			{
	 			evtval+=p->iValue(s, c);
				channel_data.push_back(p->iValue(s,c)); 	
			}
//		std::cout<<"have loaded in the data"<<std::endl;
			if (channel_data.size()<3) continue;
			int pedestal=getPedestal(channel_data);
			subtractPeak(channel_data, pedestal);
			hs.at(c).at(s)->Fill(channel_data.at(s)); 
		}
		 
  	return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCalPedestalChannels::ResetEvent(PHCompositeNode *topNode)
{
  std::cout << "HCalPedestalChannels::ResetEvent(PHCompositeNode *topNode) Resetting internal structures, prepare for next event" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}
void HCalPedestalChannels::subtractPeak(std::vector<int>* data, int pedestal, int channel)
{
	//subracts peak/waveform from data using a new fit if n_evt<100, template derived if n_evt>100
	std::pair<float, float> peak=findPeak(data, pedestal);
} 
std::pair<float,float> HCalPedestalChannels::findPeak(std::vector<int>* data, int pedestal)
{
	//First find the max sample value
	int maxval=0, maxpos=0;
	for(int i=0; i<data->size(); i++)
	{
		if(data->at(i) > maxval){
			maxval=data->at(i);
			maxpos=i;
		}
	}
	//now that I have that, need to figure out the peak value using approximates to the derivative look for change in slopes to restrict the region of intrest
	bool crossed_peak=false, sure_of_it=false; //look for going from mixed positive and negative to only negative and then verify 
	for(int i=0; i<data->size()-1; i++)
	{
		if(data->at(i) < pedestal || data->at(i) < 0.25*(maxval+3*pedestal)) continue; // throw out anything too small 
		if(crossed_peak && sure_of_it) break;
		bool has_neg=false, has_pos=false;
		for(int j=i+1; j<data->size(); j++)
		{
			float slope=(data->at(j)-data->at(i))/(j-i);
			if(slope <0) has_neg=true;
			if(slope > 0) has_pos=true;
			if(has_neg && has_pos) break;
		}
		if(has_pos) crossed_peak=false;
		else if(crossed_peak && has_neg) sure_of_it=true; //make sure it wasnt a temporary fluctuation issue
		else if(!crossed_peak && has_neg) crossed_peak=true; //if all forward slopes are negative that means we are near the peak
		if(crossed_peak && sure_of_it){
			 maxpos=i-1;
			 break;
		} 
	}
	//now I have a better peak value around which to expand, need to now get the aprroximation to the actual peak
		
}
float HCalPedestalChannels::Heuristic(std::vector<int> data, std::vector<int> wf, int npr)
{
	//use chi_squared/ndf as a fitting heuristic
	float chi=0;
	int ndf=data.size()-npr;
	for(int i=0; i<data.size(); i++) chi+=pow(data[i]-wf[i],2)/data[i];
	chi=chi/ndf; 
	return chi;
}
int HCalPedestalChannels::getPedestal(std::vector<int> chl_data)
{
	int pedetal=0;
	for(int i=0; i<chl_data.size(); i++)
	{
		if(i<4)	pedestal+=chl_data.at(i);
	}
	pedestal=pedestal/3;
	return pedestal;
	//use the first three sample method as a baseline
}
void HCalPedestalChannels::findaFit(function_template* T, std::vector<int> chl_data, float pos, int pedestal)
{
	//This is the A* search 
	//base level, this uses dual exponential to account for the rising/falling nature and then adds polynomial corrections as the higher order terms
	TF1* f=new TF1("f", "[0]*exp(x*[1])+[2]*exp(-x*[3])+[pedestal]", 0, chl_data->size());
	f->FixParameter("pedestal", pedestal);
	TF1* f_gaus=new TF1("f_gaus", "[0]*exp(x*[1])+[2]*exp(-x*[1])+[3]*exp(-x*x*[4])+[pedestal]", 0, chl_data->size());
	f_gaus->FixParameter("pedestal", pedestal);
	TH1F* ch=new TH1F("channel", "temp", chl_data.size(), -0.5, chl_data.size()+0.5);
	for(auto c:chl_data) ch->Fill(c);
	TFitResultPtr r=ch->Fit(f, "S");
	TFitResultPtr rg=ch->Fit(f_gaus, "S");
	int n_params=4, n_g_p=5; 
	std::map<float, TF1*> children_queue {{r->chi2/n_params, f}, {rg->chi2/n_p_g,f_gaus}} ;
	std::pair<float, TF1*> good_one {r->chi2/n_params, f};
	while(children_queue.size() != 0)
	{
		auto parent=children_queue.begin();
		float cn=parent->first; 
		if(cn<=1){
			 children_queue.erase(cn);
			continue;
		}
		else{
			good_one=std::make_pair(parent->first, parent->second);
		}
		TFormula* f1=parent->second->GetFormula();
		int fparams=f1->GetNpar();
		fparam+=-4;
		std::stringstream formula_string (f1->GetExpFormula());
		std::string substrs;
		std::vector<std::string> child_strings;
		
	}
}
float HCalPedestalChannels::FindWaveForm(std::vector <int> *chl_data, float pos, float peak, int channel, int pedestal)
{
	//Actually does the waveform finding, will either apply the template to the data or will create the template
	if(n_evt<20)
	{
		function_template* T=new function_template;
		findaFit(T, *chl_data, pos, pedestal)
		//do the A* search over all polynomial fit approaches+ landau
		if(n_evt==1) templates.push_back(*T);
		else{
			function_template* T_old=&templates.at(channel);
			T_old->peak_pos=1/n_evt*((n_evt-1) * T_old->peak_pos + T->peak_pos);
			if(T_old->n_params < T->n_params) T_old->function=T->fun ction;
			T_old->n_params=std::max(T_old->n_params, T->n_params);
			T_old->chi=1/n_evt*((n_evt-1) * T_old->chi + T->chi);
			for(int i=0; i<T->params.size(); i++)
			{
				if(i< T_old->params.size()) T_old->params.at(i)=1/n_evt*((n_evt-1) * T_old->params.at(i) + T->params.at(i));
				else T_old->params.push_back(T->params.at(i));
			}
			
		}
	}
	else{
		function_template T=templates.at(channel);
		T.peak_pos=pos;
		scaleToFit(&T, peak, getWidth(chl_data, peak));
	}
	
}
int getWidth(std::vector<int> chl_data, float peak)
{ 
	int diff=peak-pedestal;
	int low=1000, high=0;
	for(int i=0; i<chl_data.size(); i++)
	{
		if(chl_data.at(i) > pedestal+0.46*diff && chl_data.at(i) < pedestal+0.56*diff){
			if( i < low) low=i;
			if ( i > low && i > high) high=i;
		}
	}
	int width=high-low;
	return width;
}
void scaleToFit(function_template* T,float peak, int width)
{
	int n_params=T->n_params;
	std::vector<float> prs=T->params; 
	TF1* f=T->function;
		
}
//____________________________________________________________________________..
int HCalPedestalChannels::EndRun(const int runnumber)
{
  std::cout << "HCalPedestalChannels::EndRun(const int runnumber) Ending Run for Run " << runnumber << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCalPedestalChannels::End(PHCompositeNode *topNode)
{
  std::cout << "HCalPedestalChannels::End(PHCompositeNode *topNode) This is the End..." << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCalPedestalChannels::Reset(PHCompositeNode *topNode)
{
 std::cout << "HCalPedestalChannels::Reset(PHCompositeNode *topNode) being Reset" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
void HCalPedestalChannels::Print(const std::string &what) const
{
  std::cout << "HCalPedestalChannels::Print(const std::string &what) const Printing info for " << what << std::endl;
}
