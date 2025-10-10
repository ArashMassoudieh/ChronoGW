#include "BTCSet.h"
#include "string.h"
#include <fstream>
#include "StringOP.h"
#include "DistributionNUnif.h"
#include <iostream>
#include "Vector.h"
#include "Matrix.h"

using namespace std;

CBTCSet::CBTCSet(void)
{
	nvars = 0;
	BTC.resize(nvars);
	unif = true;
}

CBTCSet::~CBTCSet(void)
{

}

CBTCSet::CBTCSet(int n)
{
	nvars = n;
	BTC.resize(nvars);
	for (int i=0; i<nvars; i++) BTC[i] = CBTC();
	unif = true;
}

CBTCSet merge(CBTCSet A, CBTCSet &B)
{
	CBTCSet C = A;
	for (int i=0; i<B.nvars; i++)
	{	if (B.names.size()>i) C.names.push_back(B.names[i]);
		C.BTC.push_back(B.BTC[i]);
		C.nvars++;
	}
	return C;
}

CBTCSet merge(vector<CBTCSet> &A)
{
	CBTCSet C = A[0];
	for (int i=1; i<A.size(); i++)
		C = merge(C,A[i]);
	return C;
}

void CBTCSet::writetofile(char outputfile[])
{
	FILE *Fil;
	Fil = fopen(outputfile, "w");
	for (int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n"); 
	for (int j=0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
			if (i<BTC[i].n)
				fprintf(Fil, "%lf, %le,", BTC[i].t[j], BTC[i].C[j]);
			else
				fprintf(Fil, ", ,");

		}
		fprintf(Fil, "\n");
	}

	fclose(Fil);

}

void CBTCSet::writetofile(string outputfile)
{
	FILE *Fil;
	Fil = fopen(outputfile.c_str() , "w");
	fprintf(Fil , "//");
	for (int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n"); 
	for (int j=0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
			if (j<BTC[i].n)
				fprintf(Fil, "%lf, %le,", BTC[i].t[j], BTC[i].C[j]);
			else
				fprintf(Fil, ", ,");

		}
		fprintf(Fil, "\n");
	}

	fclose(Fil);

}

void CBTCSet::writetofile(string outputfile, int outputwriteinterval)
{
	FILE *Fil;
	Fil = fopen(outputfile.c_str() , "w");
	for (int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n"); 
	for (int j=0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
			if (j%outputwriteinterval==0)
				if (j<BTC[i].n)
					fprintf(Fil, "%lf, %le,", BTC[i].t[j], BTC[i].C[j]);
				else
					fprintf(Fil, ", ,");

		}
		if (j%outputwriteinterval==0)
			fprintf(Fil, "\n");
	}
}

void CBTCSet::writetofile_unif(string outputfile, int outputwriteinterval)
{
	FILE *Fil;
	Fil = fopen(outputfile.c_str() , "w");
	for (int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n"); 
	for (int j=0; j<maxnumpoints(); j++)
	{
		if (j%outputwriteinterval==0) fprintf(Fil, "%lf, ", BTC[0].t[j]);
		for (int i=0; i<nvars; i++)
		{
				if (j%outputwriteinterval==0)
					if (j<BTC[i].n)
						fprintf(Fil, "%le,", BTC[i].C[j]);
					else
						fprintf(Fil, ",");

		}
		if (j%outputwriteinterval==0)
			fprintf(Fil, "\n");
	}

	fclose(Fil);

}

int CBTCSet::maxnumpoints()
{
	int m = 0;
	for (int i=0; i<nvars; i++)
		if (BTC[i].n>m) m = BTC[i].n;
	
	return m;
}

CBTCSet::CBTCSet(const CBTCSet &B)
{
	nvars = B.nvars;
	BTC.resize(nvars);
	names = B.names;
	for (int i=0; i<nvars; i++)
		BTC[i] = B.BTC[i]; 
	unif = B.unif;

}

CBTCSet& CBTCSet::operator = (const CBTCSet &B)
{
	nvars = B.nvars;
	BTC.resize(nvars);
	names = B.names;
	for (int i=0; i<nvars; i++)
		BTC[i] = B.BTC[i]; 
	unif = B.unif;
	return *this;

}

vector<double> CBTCSet::interpolate(double t)
{
	vector<double> out;
	out.resize(nvars); 
	for (int i=0; i<nvars; i++) 
		out[i] = BTC[i].interpol(t);

	return out;
}

CBTCSet CBTCSet::Extract(int interval)
{
	CBTCSet X(nvars);
	X.names = names;
	for (int i=0; i<nvars; i++)
		for (int j=0; j<BTC[i].n; j+=interval)
			X.BTC[i].append(BTC[i].t[j], BTC[i].C[j]);

	return X;
}

CBTCSet::CBTCSet(string filename, bool varytime)
{
	unif = false;
	ifstream file(filename);
	vector<string> s;
	nvars = 0;
	if (varytime==false)
		while (file.eof()== false)
		{
			s = getline(file);
			if (s.size()>0)
			if (s[0].substr(0,2)!="//")
			{
				if (nvars==0) {nvars = s.size()-1; BTC.resize(nvars);}
				if (s.size()==nvars+1)
					for (int i=0; i<nvars; i++)
					{
						BTC[i].t.push_back(atof(s[0].c_str()));
						BTC[i].C.push_back(atof(s[i+1].c_str()));
						BTC[i].n++;
						if (BTC[i].t.size()>2)
								if ((BTC[i].t[BTC[i].t.size()-1]-BTC[i].t[BTC[i].t.size()-2])!=(BTC[i].t[BTC[i].t.size()-2]-BTC[i].t[BTC[i].t.size()-3]))
									BTC[i].structured = false;
					
					}
				
			}
		}
	else
		while (file.eof()== false)
		{
			s = getline(file);
			if (s.size()>0)
			if (s[0].substr(0,2)!="//")
			{
				if (nvars==0) {nvars = s.size()/2; BTC.resize(nvars);}
				if (s.size()==2*nvars)
					for (int i=0; i<nvars; i++)
					{
						if ((trim(s[2*i])!="") && (trim(s[2*i+1])!=""))
						{	BTC[i].t.push_back(atof(s[2*i].c_str()));
							BTC[i].C.push_back(atof(s[2*i+1].c_str()));
							BTC[i].n++;
							if (BTC[i].t.size()>2)
								if ((BTC[i].t[BTC[i].t.size()-1]-BTC[i].t[BTC[i].t.size()-2])!=(BTC[i].t[BTC[i].t.size()-2]-BTC[i].t[BTC[i].t.size()-3]))
									BTC[i].structured = false;
						}
					}
			}
		}
	file.close();
}



void CBTCSet::getfromfile(string filename, bool varytime)
{
	unif = false;
	ifstream file(filename);
	vector<string> s;
	nvars = 0;
	if (varytime==false)
		while (file.eof()== false)
		{
			s = getline(file);
			if (s.size()>0)
			if (s[0].substr(0,2)!="//")
			{
				if (nvars==0) {nvars = s.size()-1; BTC.resize(nvars); for (int i=0; i<nvars; i++) BTC[i].structured = true;}
				if (s.size()==nvars+1)
					for (int i=0; i<nvars; i++)
					{
						BTC[i].t.push_back(atof(s[0].c_str()));
						BTC[i].C.push_back(atof(s[i+1].c_str()));
						BTC[i].n++;
						if (BTC[i].n>2)
								if ((BTC[i].t[BTC[i].n-1]-BTC[i].t[BTC[i].n-2]) != (BTC[i].t[BTC[i].n-2]-BTC[i].t[BTC[i].n-3]))
									BTC[i].structured = false;
					}
				
			}
		}
	else
		while (file.eof()== false)
		{
			s = getline(file);
			if (s.size()>0)
			if (s[0].substr(0,2)!="//")
			{
				if (nvars==0) {nvars = s.size()/2; BTC.resize(nvars); for (int i=0; i<nvars; i++) BTC[i].structured = true;}
				
				int n_line = s.size()/2;
				for (int i=0; i<n_line; i++)
				{
					if ((trim(s[2*i])!="") && (trim(s[2*i+1])!=""))
					{	BTC[i].t.push_back(atof(s[2*i].c_str()));
						BTC[i].C.push_back(atof(s[2*i+1].c_str()));
						BTC[i].n++;
						if (BTC[i].n>2)
							if ((BTC[i].t[BTC[i].n-1]-BTC[i].t[BTC[i].n-2]) != (BTC[i].t[BTC[i].n-2]-BTC[i].t[BTC[i].n-3]))
								BTC[i].structured = false;
					}
				}
			}
		}
	file.close();
}

double CBTCSet::maxtime()
{
	return BTC[0].t[BTC[0].n-1];

}

double CBTCSet::mintime()
{
	return BTC[0].t[0];

}

double diff(CBTCSet B1, CBTCSet B2)
{
	int sum = 0;
	for (int i=0; i<B1.nvars; i++)
		sum += diff_norm(B1.BTC[i],B2.BTC[i]);

	return sum;

}

CBTCSet operator>(CBTCSet B1, CBTCSet B2)
{
	CBTCSet X(min(B1.nvars,B2.nvars));
	for (int i=0; i<min(B1.nvars,B2.nvars); i++)
		X.BTC[i] = B1.BTC[i]>B2.BTC[i];

	return X;

}

CBTCSet operator * (const CBTCSet &BTC, const double &C)
{
	CBTCSet A = BTC;
	A.BTC[0] = A.BTC[0]*C;
	return A;
}

vector<double> CBTCSet::getrandom()
{
	int a = int(GetRndUniF(0,BTC[0].n));
	vector<double> res(nvars);
	for (int i=0; i<nvars; i++)
		res[i] = BTC[i].C[a];

	return res;
}

vector<double> CBTCSet::getrandom(int burnin)
{
	int a = int(GetRndUniF(0,BTC[0].n-burnin));
	vector<double> res(nvars);
	for (int i=0; i<nvars; i++)
		res[i] = BTC[i].C[a+burnin];

	return res;
}

vector<double> CBTCSet::percentile(double x)
{
	vector<double> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].percentile(x));

	return v;
}

vector<double> CBTCSet::mean(int limit)
{
	vector<double> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].mean(limit));
	return v;

}

vector<double> CBTCSet::std(int limit)
{
	vector<double> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].std(limit));
	return v;

}

CMatrix CBTCSet::correlation(int limit, int n)	
{
	CMatrix r_xy(n);

	for (int i=0; i<n; i++)
		for (int j=0; j<=i; j++)
			r_xy[i][j] = R(BTC[i], BTC[j], limit);
	
	return r_xy;

}

vector<double> CBTCSet::average()
{
	vector<double> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].average());
	return v;

}

vector<double> CBTCSet::integrate()
{
	vector<double> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].integrate());
	return v;

}

vector<double> CBTCSet::percentile(double x, int limit)
{
	vector<double> v;
	for (int i=0; i<nvars-1; i++)
		v.push_back(BTC[i].percentile(x,limit));

	return v;
}

CBTCSet CBTCSet::distribution(int n_bins, int n_columns, int limit)
{
	CBTCSet A(n_columns);		
	for (int i=0; i<n_columns; i++)	
		A.BTC[i] = BTC[i].distribution(n_bins, limit);		

	return A;
}

CVector norm2dif(CBTCSet &A, CBTCSet &B)
{
	CVector res;
	for (int i=0; i<min(A.nvars,B.nvars); i++)		
		res.append(diff(A.BTC[i].Log(1e-5),B.BTC[i].Log(1e-5))/B.BTC[i].n);	    
	
	return res;

}

void CBTCSet::append(double t, vector<double> c)
{	
	for (int i=0; i<min(int(c.size()), nvars); i++)
	{	BTC[i].structured = true;
		BTC[i].append(t,c[i]);
		if (BTC[i].n>2)
			if ((BTC[i].t[BTC[i].n-1]-BTC[i].t[BTC[i].n-2]) != (BTC[i].t[BTC[i].n-2]-BTC[i].t[BTC[i].n-3]))
				BTC[i].structured = false;
	}
}

CBTC CBTCSet::add(vector<int> ii)
{
	CBTC A = BTC[ii[0]];
	A.structured = BTC[ii[0]].structured;
	for (int i=1; i<ii.size(); i++)
	if (unif==false)	
	{	A+=BTC[ii[i]];
		A.structured = (A.structured && BTC[ii[i]].structured);
	}
	else
	{	A%=BTC[ii[i]];
		A.structured = (A.structured && BTC[ii[i]].structured);
	}

	return A;
}

CBTC CBTCSet::add_mult(vector<int> ii, vector<double> mult)
{
	CBTC A;
	if (ii.size()>0)
	{	A = mult[0]*BTC[ii[0]];
		A.structured = BTC[ii[0]].structured;
		for (int i=1; i<ii.size(); i++)
		if (unif==false)	
		{	A+=mult[i]*BTC[ii[i]];
			A.structured = (A.structured && BTC[ii[i]].structured);
		}
		else
		{	A%=mult[i]*BTC[ii[i]];
			A.structured = (A.structured && BTC[ii[i]].structured);
		}
	}
	else
	{
		A.setnumpoints(2);
		A.t[0] = mintime();
		A.t[1] = maxtime();
	}
	return A;
}

CBTC CBTCSet::add_mult(vector<int> ii, CBTCSet &mult)
{
	CBTC A;
	if (ii.size()>0)
	{	A = mult.BTC[0]*BTC[ii[0]];
		A.structured = BTC[ii[0]].structured;
		for (int i=1; i<ii.size(); i++)
		if (unif==false)	
		{	A+=BTC[ii[i]]*mult.BTC[i];
			A.structured = (A.structured && BTC[ii[i]].structured && mult.BTC[i].structured);
		}
		else
		{	A%=BTC[ii[i]]*mult.BTC[i];
			A.structured = (A.structured && BTC[ii[i]].structured && mult.BTC[i].structured);
		}
	}
	else
	{
		A.setnumpoints(2);
		A.t[0] = mintime();
		A.t[1] = maxtime();
	}
	return A;
}


CBTC CBTCSet::divide(int ii, int jj)
{
	CBTC A;
	A.structured = (BTC[ii].structured && BTC[jj].structured);
	if (unif==false)	
		A=BTC[ii]/BTC[jj];
	else
		A=BTC[ii]%BTC[jj];

	return A;


}

CBTCSet CBTCSet::make_uniform(double increment)
{
	CBTCSet out(nvars);
	out.names = names;
	
	if (unif == true)
	{	for (int i=0; i<nvars; i++)
		out.BTC[i].append(BTC[i].t[0],BTC[i].C[0]);
		for (int i=0; i<BTC[0].n-1; i++)
		{
			int i1 = int((BTC[0].t[i]-BTC[0].t[0])/increment);
			int i2 = int((BTC[0].t[i+1]-BTC[0].t[0])/increment);
			for (int j=i1+1; j<=i2; j++)
			{
				double x = j*increment+BTC[0].t[0];
				for (int k=0; k<nvars; k++)
				{
					double CC = (x-BTC[k].t[i])/(BTC[k].t[i+1]-BTC[k].t[i])*(BTC[k].C[i+1]-BTC[k].C[i])+BTC[k].C[i];
					out.BTC[k].append(x,CC);
				}
			}
		}
	}
	else
	{
		for (int k=0; k<nvars; k++)
			out.BTC[k] = BTC[k].make_uniform(increment);
	}
	return out;

}

CBTCSet CBTCSet::getpercentiles(vector<double> percents)
{
	CBTCSet X(1+percents.size());
	
	X.names.clear();
	char buffer[33];
	X.names.push_back("Mean");
	for (int j=0; j<percents.size(); j++)
	{
		string Xname = to_string(percents[j]*100)+" %";
		X.names.push_back(Xname);
	}
		
	vector<double> XX(1+percents.size());
	vector<double> XX_prc(percents.size());
	
	double meanX;
	for (int i=0; i<BTC[0].n; i++)
	{
		vector<double> x;		
		int count = 0;
		for (int j=0; j<nvars; j++)
			if (i<BTC[j].n)
			{	x.push_back(BTC[j].C[i]);
				count++;
			}

		meanX = CVector(x).sum()/count;

		XX[0] = meanX;
		XX_prc = prcntl(x,percents);
		for (int j=0; j<percents.size(); j++)
			XX[j+1] = XX_prc[j];
		
		X.append(BTC[0].t[i],XX);
	}

	return X;
}

CVector CBTCSet::out_of_limit(double limit)
{
	CVector v(nvars);
	for (int i=0; i<nvars; i++)
	{
		double n_tot = BTC[i].n;
		double n_exceed = 0;
		for (int j=0; j<BTC[i].n; j++)
		{		if (BTC[i].C[j] > limit)
				n_exceed++;		}

		v[i] = n_exceed/n_tot;
	}

	return v;
}

CBTCSet CBTCSet::add_noise(vector<double> std, bool logd)
{
	CBTCSet X(nvars);
	for (int i=0; i<nvars; i++)
		X.BTC[i] = BTC[i].add_noise(std[i],logd);

	return X;
}