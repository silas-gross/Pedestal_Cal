// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef HCALPEDESTALCHANNELS_H
#define HCALPEDESTALCHANNELS_H

#include <fun4all/SubsysReco.h>

#include <math.h>
#include <string>
#include <vector>
#include <utility>

class PHCompositeNode;

class HCalPedestalChannels : public SubsysReco
{
 public:
	struct function_templates{
		float peak_pos;
		int nparams;
		float chisquare;
		std::vector<float> params;
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
	std::pair<float, float> getPeak(std::vector<int>*, int);
	float FindWaveForm(std::vector<int>*, int, int);
	float Heuristic(std::vector<int>, std::vector<int>, int);
	void scaleToFit(function_templates*, float, int);
	std::vector<std::vector<TH1F*>> hs;
	std::vector<function_templates> templates;
	std::vector<towerinfo> towers; 
};

#endif // HCALPEDESTALCHANNELS_H
