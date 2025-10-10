#pragma once
#include<vector>
#include<string>
#include "DistributionNUnif.h"

using namespace std;

class CDistribution
{
public:
	CDistribution(void);
	CDistribution(string name);
	~CDistribution(void);
	vector<double> params;
	string name;
	double CDistribution::evaluate(double x);
	double CDistribution::evaluate_CDF(double x);
	double pi;
	int n;
	vector<double> s;
	vector<double> e;
	CDistribution::CDistribution(int nn);
	CDistribution(const CDistribution &C);
	CDistribution CDistribution::operator = (const CDistribution &C);
	int CDistribution::GetRand();
};

double erf(double x);
double erfc(double x);
double Gammapdf(double x, double k, double theta);
double gamma(double x);

