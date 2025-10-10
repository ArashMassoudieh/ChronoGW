#pragma once
#include "BTC.h"
#include <vector>
#include "Vector.h"

class CBTCSet
{
public:
	CBTCSet(void);
	CBTCSet(int n);
	CBTCSet(const CBTCSet &BTC);
	CBTCSet::CBTCSet(string filename, bool varytime);
	int nvars;
	vector<CBTC> BTC;
	void CBTCSet::writetofile(char outputfile[]);
	int CBTCSet::maxnumpoints();
	CBTCSet& CBTCSet::operator = (const CBTCSet &C);
	vector<string> names;
	bool unif;
	void CBTCSet::writetofile(string outputfile);
	void CBTCSet::writetofile(string outputfile, int writeinterval);
	void CBTCSet::writetofile_unif(string outputfile, int outputwriteinterval=1);
	vector<double> CBTCSet::interpolate(double t);
	void CBTCSet::getfromfile(string filename, bool varytime);
	double CBTCSet::maxtime();
	double CBTCSet::mintime();
	vector<double> CBTCSet::getrandom();
	vector<double> CBTCSet::percentile(double x);
	vector<double> CBTCSet::mean(int limit);
	vector<double> CBTCSet::std(int limit);
	CMatrix CBTCSet::correlation(int limit, int n);
	vector<double> CBTCSet::integrate();
	vector<double> CBTCSet::average();
	vector<double> CBTCSet::percentile(double x, int limit);
	vector<double> CBTCSet::getrandom(int burnin);
	void CBTCSet::append(double t, vector<double> c);
	CBTC CBTCSet::add(vector<int> ii);
	CBTC CBTCSet::add_mult(vector<int> ii, vector<double> mult);
	CBTC CBTCSet::add_mult(vector<int> ii, CBTCSet &mult);
	CBTC CBTCSet::divide(int ii, int jj);
	CBTCSet CBTCSet::make_uniform(double increment);
	CBTCSet CBTCSet::getpercentiles(vector<double> percents);
	CVector CBTCSet::out_of_limit(double limit);
	CBTCSet CBTCSet::distribution(int n_bins, int n_columns, int limit);
	CBTCSet CBTCSet::add_noise(vector<double> std, bool logd);
	CBTCSet CBTCSet::Extract(int interval);
public:
	~CBTCSet(void);
};

double diff(CBTCSet B1, CBTCSet B2);
CBTCSet operator * (const CBTCSet &BTC, const double &C);
CVector norm2dif(CBTCSet &A, CBTCSet &B);
CBTCSet merge(CBTCSet A, const CBTCSet &B);
CBTCSet merge(vector<CBTCSet> &A);
CBTCSet operator>(CBTCSet B1, CBTCSet B2);