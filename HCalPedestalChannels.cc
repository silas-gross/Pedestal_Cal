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
R__LOAD_LIBRARY(libfun4all.so);
R__LOAD_LIBRARY(libfun4allraw.so);
HCalPedestalChannels::HCalPedestalChannels(const std::string &name):
 SubsysReco(name)
{
  std::cout << "HCalPedestalChannels::HCalPedestalChannels(const std::string &name) Calling ctor" << std::endl;
  n_evt=-20;
  for(int i=1; i<9; i++){
	packets.push_back(7000+i);
	packets.push_back(8000+i);
  }
  
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
		if(n_evt<=0) break;
		try{
			e->getPacket(pid);
		}
		catch(std::exception* e){ std::cout<<"no packet with number " <<pid <<std::endl;}
		Packet* p=e->getPacket(pid);
		if(!p) continue;
//	       #pragma opm for private(c, channel_data)
		for(int c=0; c<p->iValue(0, "CHANNELS"); c++)
		{
		 	std::vector<int> channel_data;	
			 for(auto s=0; s<31; s++)
			{
				try{
					channel_data.push_back(p->iValue(s,c)); 
				}
				catch(std::exception& e){continue;}
			}
//		std::cout<<"have loaded in the data"<<std::endl;
			if (channel_data.size()<3) continue;
			int pedestal=getPedestal(channel_data);
			subtractPeak(&channel_data, pedestal, c);
			for(int s=0; s< (int) channel_data.size(); s++) hs.at(c).at(s)->Fill(channel_data.at(s)); 
		}
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
	FindWaveForm(data, peak.first, peak.second, channel, pedestal); 	
} 
std::pair<float,float> HCalPedestalChannels::findPeak(std::vector<int>* data, int pedestal)
{
	//First find the max sample value
	int maxval=0, maxpos=0;
	for(int i=0; i<(int) data->size(); i++)
	{
		if(data->at(i) > maxval){
			maxval=data->at(i);
			maxpos=i;
		}
	}
	//now that I have that, need to figure out the peak value using approximates to the derivative look for change in slopes to restrict the region of intrest
	bool crossed_peak=false, sure_of_it=false; //look for going from mixed positive and negative to only negative and then verify 
	for(int i=0; i<(int) data->size()-1; i++)
	{
		if(data->at(i) < pedestal || data->at(i) < 0.25*(maxval+3*pedestal)) continue; // throw out anything too small 
		if(crossed_peak && sure_of_it) break;
		bool has_neg=false, has_pos=false;
		for(int j=i+1; j<(int) data->size(); j++)
		{
			float slope=(data->at(j)-data->at(i))/(j-i);
			if(slope < 0) has_neg=true;
			if(slope > 0) has_pos=true;
			if(has_neg && has_pos) j=data->size()-1;
		}
		if(has_pos) crossed_peak=false;
		else if(crossed_peak && has_neg) sure_of_it=true; //make sure it wasnt a temporary fluctuation issue
		else if(!crossed_peak && has_neg) crossed_peak=true; //if all forward slopes are negative that means we are near the peak
		if(crossed_peak && sure_of_it){
			 maxpos=i-1;
			 break;
		} 
	}
	//have good appriximation to the peak from int values, now do a weighted average to nearest neighbor to find peak position
	float maxpeakpos, maxpeakval; 
	int comp=-1;
	if(data->at(maxpos-1) > data->at(maxpos+1)) comp=maxpos-1; 
	else if(data->at(maxpos-1) < data->at(maxpos+1)) comp=maxpos+1;
	else{
		 maxpeakpos=(float)maxpos;
		 maxpeakval=(float)maxval;
	}
	if(comp!= -1) maxpeakpos=(float)(maxval*maxpos+data->at(comp)*(comp))/(maxval+data->at(comp));
	if(comp!= -1) maxpeakval=(float)(maxval*maxpos+data->at(comp)*(comp))/(maxpos+comp);
	return std::make_pair(maxpeakpos, maxpeakval);
}
float HCalPedestalChannels::Heuristic(std::vector<int> data, std::vector<int> wf, int npr)
{
	//use chi_squared/ndf as a fitting heuristic
	float chi=0;
	int ndf=data.size()-npr;
	for(int i=0; i< (int) data.size(); i++) chi+=pow(data[i]-wf[i],2)/data[i];
	chi=chi/ndf; 
	return chi;
}
int HCalPedestalChannels::getPedestal(std::vector<int> chl_data)
{
	int pedestal=0;
	for(int i=0; i<(int) chl_data.size(); i++)
	{
		if(i<4)	pedestal+=chl_data.at(i);
	}
	pedestal=pedestal/3;
	return pedestal;
	//use the first three sample method as a baseline
}
void HCalPedestalChannels::findaFit(function_templates* T, std::vector<int> chl_data, float pos, int pedestal)
{
	//This is the A* search 
	//base level, this uses dual exponential to accound tof the rising/falling nature and then adds polynomial corrections as the higher order terms
	TF1* f=new TF1("f", "[4]*exp([1]*x)+[2]*exp(-[3]*x)+[0]", 0, chl_data.size());
	f->FixParameter(0, pedestal);
	TF1* f_gaus=new TF1("f_gaus", "[5]*exp([1]*x)+[2]*exp(-[1]*x)+[3]*exp(-[4]*x*x)+[0]", 0, chl_data.size());
	f_gaus->FixParameter(0, pedestal);
	TH1F* ch=new TH1F("channel", "temp", chl_data.size(), -0.5, chl_data.size()+0.5);
	for(auto c:chl_data) ch->Fill(c);
	TFitResultPtr r=ch->Fit(f, "SB");
	TFitResultPtr rg=ch->Fit(f_gaus, "SB");
	int nparams=4, n_g_p=5; 
	std::map<float, TF1*> children_queue {std::make_pair(r->Chi2()/(chl_data.size() - nparams), f), std::make_pair(rg->Chi2()/(chl_data.size() - n_g_p),f_gaus)} ;
	std::pair<float, TF1*> good_one =std::make_pair(r->Chi2()/(chl_data.size()-nparams), f);
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
			children_queue.erase(cn);
		}
		TFormula* f1=parent->second->GetFormula();
		int fparams=f1->GetNpar();
		fparams++;
		std::string formula_string (f1->GetExpFormula());
		std::string substrs;
		size_t pos =0;
		if(fparams+1 > (int) chl_data.size()/2) break; //DOF>1/2 data set
		std::vector<std::string> child_strings;
		while((pos=formula_string.find("x)"))!= std::string::npos){
			std::string paramnumb =std::to_string(fparams);
			substrs="*(1+[paranumb]*x)";
			formula_string.insert(pos, substrs);
			child_strings.push_back(formula_string);
		}
		for(int i=0; i<(int) child_strings.size(); i++)
		{
			std::string chldn= std::to_string(i);
			chldn+="_funct";
			TF1* fc=new TF1(chldn.c_str(), child_strings.at(i).c_str(), 0, chl_data.size());
			fc->FixParameter(0, pedestal);
			TFitResultPtr rfc=ch->Fit(fc, "SB");
			if(rfc->Chi2()/fparams <= 1 || rfc->Chi2()/fparams > cn) continue;
			else children_queue[rfc->Chi2()/(chl_data.size()-fparams)]=fc; 
		}
		good_one=*(children_queue.begin());
	} //A* search built, inserts increasing number of polynomial terms after each one 
	T->template_funct=good_one.second;
	T->nparams=good_one.second->GetFormula()->GetNpar();	
	T->chisquare=good_one.first*T->nparams;
	T->peak_pos=pos;
	T->template_funct->FixParameter(0, pedestal);
	TFitResultPtr rf1=ch->Fit(T->template_funct, "SB");
	T->params=rf1->Parameters();
}
float HCalPedestalChannels::FindWaveForm(std::vector <int> *chl_data, float pos, float peak, int channel, int pedestal)
{
	//Actually does the waveform finding, will either apply the template to the data or will create the template
	function_templates* T1=new function_templates;
	if(n_evt<20)
	{
		function_templates* T=new function_templates;
		findaFit(T, *chl_data, pos, pedestal);
		//do the A* search over all polynomial fit approaches+ landau
		if(n_evt==1) templates.push_back(*T);
		else{
			function_templates* T_old=&templates.at(channel);
			T_old->peak_pos=1/n_evt*((n_evt-1) * T_old->peak_pos + T->peak_pos);
			if(T_old->nparams < T->nparams) T_old->template_funct=T->template_funct;
			T_old->nparams=std::max(T_old->nparams, T->nparams);
			T_old->chisquare=1/n_evt*((n_evt-1) * T_old->chisquare + T->chisquare);
			for(int i=0; i<(int) T->params.size(); i++)
			{
				if(i< (int)T_old->params.size()) T_old->params.at(i)=1/n_evt*((n_evt-1) * T_old->params.at(i) + T->params.at(i));
				else T_old->params.push_back(T->params.at(i));
			}
			
		}
	}
	else{
		function_templates* T = &templates.at(channel);
		T->peak_pos=pos;
		scaleToFit(T, peak, getWidth(*chl_data, peak, pedestal));
	
	}
	T1=&templates.at(channel);
	TF1* f=(TF1*)T1->template_funct->Clone();
	for(int i=0; i<f->GetNpar(); i++)
	{
		f->SetParameter(i, T1->params.at(i));
	}
	std::vector<int> evaled;
	for(int i=0; i < (int) chl_data->size(); i++)
	{
		evaled.push_back(f->Eval(i));
	}
	float hr= Heuristic(*chl_data, evaled, f->GetNpar());  
	for(int i=0; i<(int) chl_data->size(); i++) chl_data->at(i)+=-evaled.at(i);
	return hr;
}
int HCalPedestalChannels::getWidth(std::vector<int> chl_data, float peak, int pedestal)
{ 
	int diff=peak-pedestal;
	int low=10000, high=0;
	for(int i=0; i< (int) chl_data.size(); i++)
	{
		if(chl_data.at(i) > pedestal+0.46*diff && chl_data.at(i) < pedestal+0.56*diff){
			if( i < low) low=i;
			if ( i > low && i > high) high=i;
		}
	}
	int width=high-low;
	return width;
}

void HCalPedestalChannels::scaleToFit(function_templates* T, float peak, int width)
{
	int nparams=T->nparams;
	std::vector<double> prs=T->params; 
	TF1* f=(TF1*)T->template_funct->Clone();
	for(int i=0; i< f->GetNpar(); i++) f->SetParameter(i, prs.at(i));
	float max=f->GetMaximum();
	float ratio=peak/max;
	float integral=f->Integral(0, 2*width);
	float avg=f->Mean(0, 2*width);
	float wide=integral/avg;
	float rat2=2 * (float)width/wide;
	ratio=(ratio+rat2)/2;
	for(int i=0; i<nparams-1; i++) T->params.at(i)=prs.at(i)*ratio; //this may do the thing?, Scale up to hit the peak and over to fit. Really could also just hit the data with a fit which is proablb better but way slowwer

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
