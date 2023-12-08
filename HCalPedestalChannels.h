// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef HCALPEDESTALCHANNELS_H
#define HCALPEDESTALCHANNELS_H

#include <fun4all/SubsysReco.h>

#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <omp.h>
#include <map>
#include <algorithm>

#include <TMath.h>
#include <TFormula.h>
#include <TNtuple.h>
#include <TH1.h>
#include <TF1.h>
#include <TFitResult.h>

#include <fun4all/Fun4AllBase.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4allraw/Fun4AllPrdfInputManager.h>
#include <fun4allraw/Fun4AllPrdfInputPoolManager.h>
#include <fun4all/SubsysReco.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHDataNode.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/getClass.h>
//#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>


class PHCompositeNode;

class HCalPedestalChannels : public SubsysReco
{
 public:
	struct function_templates{
		float peak_pos;
		int nparams;
		float chisquare;
		std::vector<double> params;
		TF1* template_funct;
	};
	struct towerinfo {
		bool inner_outer; //false for inner, true for outer
		bool north_south; //false for North, true for south
		int sector; 	//Sector 0-31
		int channel;	//Channels 0-23
		int packet; 	//packet is shared between 4 sectors
		int etabin;	//pseudorapidity bin
		int phibin;	//phi bin
		float eta;	//psedorapidity value
		float phi;	//phi value
		std::string label;	//label for tower to quick parse
	};
  
  HCalPedestalChannels(const std::string &name = "HCalPedestalChannels");

  ~HCalPedestalChannels() override;

  /** Called during initialization.
      Typically this is where you can book histograms, and e.g.
      register them to Fun4AllServer (so they can be output to file
      using Fun4AllServer::dumpHistos() method).
   */
  int Init(PHCompositeNode *topNode) override;

  /** Called for first event when run number is known.
      Typically this is where you may want to fetch data from
      database, because you know the run number. A place
      to book histograms which have to know the run number.
   */
  int InitRun(PHCompositeNode *topNode) override;

  /** Called for each event.
      This is where you do the real work.
   */
  int process_event(PHCompositeNode *topNode) override;

  /// Clean up internals after each event.
  int ResetEvent(PHCompositeNode *topNode) override;

  /// Called at the end of each run.
  int EndRun(const int runnumber) override;

  /// Called at the end of all processing.
  int End(PHCompositeNode *topNode) override;

  /// Reset
  int Reset(PHCompositeNode * /*topNode*/) override;

  void Print(const std::string &what = "ALL") const override;

 private:
	int getPedestal(std::vector<int>);
	std::pair<float, float> findPeak(std::vector<int>*, int);
	float FindWaveForm(std::vector<int>*, float, float, int, int);
	float Heuristic(std::vector<int>, std::vector<int>, int);
	void scaleToFit(function_templates*, float, int);
	std::vector<std::vector<TH1F*>> hs;
	std::vector<function_templates> templates;
	std::vector<towerinfo> towers;
	std::vector<int> packets; 
	void subtractPeak(std::vector<int>*, int, int);
	int getWidth(std::vector<int>, float, int);
	void findaFit(function_templates*, std::vector<int>, float, int);
};

#endif // HCALPEDESTALCHANNELS_H
