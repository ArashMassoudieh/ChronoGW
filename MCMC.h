#pragma once
#include <vector>
#include "math.h"
#include <iostream>
#include "NormalDist.h"
#include "GWA.h"
#include "GA.h"
#include "Vector.h"

using namespace std;

struct Param
{
	int param_ID;
	int type; // 0: uniform, 1: normal, 2: lognormal
	double low, high;
	int loged;
	double mean, std;
};

class CMCMC
{
public:
	CMCMC(void);
	CMCMC(int nn, int nn_chains);
	CMCMC(const CGA &GA);
	~CMCMC(void);
	int n;
	int n_chains;
	int n_burnout;
	vector<Param> MCMCParam;
	vector<vector<double>> Params;
	vector<double> pertcoeff;
	vector<double> logp;
	vector<double> logp1;
	double posterior(vector<double> par);
	void initialize();
	void CMCMC::initialize(vector<double> par);
	bool CMCMC::step(int k);
	bool CMCMC::step(int k, int nsamps, string filename);
	vector<double> purturb(int k);
	CNormalDist ND;
	int stucklimit;
	double purtscale;
	void CMCMC::writeoutput(string filename);
	vector<int> params;
	CGWA G;
	CBTCSet MData;
	int nActParams;
	int numBTCs;
	int CMCMC::getparamno(int j);
	CGWA G_out;
	double CMCMC::posterior(vector<double> par, bool out);
	bool logtrans, fixedstd;
	void CMCMC::getfromGA(const CGA &GA);
	string outputfilename;
	int nsamples;
	int CMCMC::getparamno(int i,int ts);
	int CMCMC::get_act_paramno(int i);
	int CMCMC::get_time_series(int i);
	vector<bool> apply_to_all;
	int n_ts;
	int writeinterval;
	CVector CMCMC::sensitivity(double d, vector<double> par);
	CVector CMCMC::sensitivity_ln(double d, vector<double> par);
	CBTCSet CMCMC::model(vector<double> par);
	CBTCSet CMCMC::model_lumped(vector<double> par);
	CMatrix CMCMC::sensitivity_mat(double d, vector<double> par);
	CVector CMCMC::sensitivity_vec(double d, vector<double> par);
	CMatrix CMCMC::sensitivity_mat_lumped(double d, vector<double> par);
	CVector CMCMC::sensitivity_vec_lumped(double d, vector<double> par);
	CBTCSet CMCMC::prior_distribution(int n_bins);
	double purt_fac;
	bool mixederror;
	bool noinipurt;
	bool sensbasedpurt;
	vector<string> paramname;
	bool global_sensitivity;
	bool continue_mcmc;
	int CMCMC::readfromfile(string filename); 
};

