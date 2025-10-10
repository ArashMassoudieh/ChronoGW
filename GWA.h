#pragma once
#include "Distribution.h"
#include "Tracer.h"
#include <vector>
#include <string>
#include <iostream>
#include "StringOP.h"
#include "Well.h"
#include "LIDconfig.h"
#include "math.h"

using namespace std;

struct range //properties of unknown parameters
{
	string name;
	double low;
	double high;
	double tempcorr;
	bool fixed;
	int param_no;
	int log;
	bool applytoall;
	vector<string> location;
	vector<string> quan;
	vector<int> location_type; //0: well, 1: tracer
	
};

struct measured_chrc //Properties of observed data
{
	string name;
	int loc_type; 
	int quan;
	int id;
	int std_no;
	int error_structure; 
	int std_to_param;
	string observed_filename;
	CBTC observed_data;
	bool detect_limit;
	double detect_limit_value;
	int max_data_no;
	bool count_max;
};

class CGWA
{
public:
	CGWA(void);
	~CGWA(void);
	CGWA(string filename);
	CGWA(const CGWA &m);
	CGWA& CGWA::operator=(const CGWA &m);
	void CGWA::getconfigfromfile(string filename);
	vector<CTracer> Tracer;
	vector<CWell> Well;
	CLIDconfig lid_config;
	vector<range> parameters;
	vector<double> params_vals;
	string pathname;
	string outpathname;
	vector<measured_chrc> measured_quan;
	void CGWA::setparams(int i, double v);
	double CGWA::getlogp();
	void CGWA::getmodeled();
	CBTCSet modeled;  
	CBTCSet projected;
	double CGWA::getoldest();
	vector<double> obs_std;
	string PE_info_filename;
	string detoutfilename;
	string realizedparamfilename;
	bool single_vz_delay;
	void CGWA::set_constant_inputs();
	bool project;
	double project_start, project_finish;
	double project_interval;
	CBTCSet CGWA::Do_project();
	bool inverse;
	bool fixed_old_tracer;
	int CGWA::lookup_parameters(string S);
	int CGWA::lookup_tracers(string S);
	int CGWA::lookup_wells(string S);
	void CGWA::load_parameters();
	void CGWA::load_wells();
	void CGWA::load_tracers();
	void CGWA::load_observed();
	void CGWA::load_settings();
};

