#pragma once
#include "BTCSet.h"
#include "Vector.h"
#include <vector>
#include "Distribution.h"
#include "QuickSort.h"

using namespace std;
class CTracer
{
public:
	CTracer(void);
	CTracer(const CTracer &m);
	~CTracer(void);
	CBTC input;
	double input_multiplier;
	double obs_std;
	string source;
	double fm_max;
	double co;
	double retard;
	double cm;
	bool linear_prod;	
	CTracer *SourceTr;
	string name;
	double CTracer::calc_conc(double t, CBTC &young_dist, double f, double vz_delay=0, bool fixed_old_tracer=0, double age_old=100000, double fm=0);
	CTracer& CTracer::operator=(const CTracer &m);
	bool vz_delay;
	double constant_input_val; bool constant_input;
	void CTracer::set_val(string S, double val);
//GUI
	void calcMatrix();
	vector<vector<CMatrix>> conc;
	double CTracer::calc_concFromMatrix(double t, vector<double> params, double f, double _vz_delay, bool fixed_old_conc, double age_old, double fm);
	double interpolationMatrix(double t, vector<double> params);
	double decay_rate;

private:
	double mu(int index);
	double lambda(int index);
	int CTracer::muInv(double mu);
	int CTracer::lambdaInv(double lambda);
	float CTracer::year(int index);
	float CTracer::yearInv(float year);
	int CTracer::decayrate(int index);
	float CTracer::decayrateInv(int decayrate);
	int startYear = 1990;
	int endYear = 2016;
	int numberofSteps = endYear - startYear;
	int nmu = 100;
	int nlambda = 100;
	float min_lambda = 0.62; //0.620467821
	float max_lambda = 661.5; //661.4811628

	float min_mu = 0.245; //0.248187128
	float max_mu = 265; //264.5924651

	float min_decayrate = 0.01;
	float max_decayrate = 10;

	int ndecay_rate = 1;
};

