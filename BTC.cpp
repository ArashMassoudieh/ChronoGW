// BTC.cpp: implementation of the CBTC class.
//
//////////////////////////////////////////////////////////////////////

#include "BTC.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include <fstream>
#include "StringOP.h"
#include "NormalDist.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBTC::CBTC()
{
	n=0;
	t.resize(n);
	C.resize(n);
	for (int i=0; i<n; i++) {C[i]=0; t[i]=0;}
	structured = true;
}

CBTC::CBTC(int n1)
{
	n=n1;
	t.resize(n);
	C.resize(n);
	for (int i=0; i<n; i++) {C[i]=0; t[i]=0;}
	structured = true;
}

void CBTC::setnumpoints(int n1)
{
	
	n = n1;
	t.resize(n);
	C.resize(n);
	for (int i=0; i<n; i++) {C[i]=0; t[i]=0;}


}

CBTC::~CBTC()
{
	
}

CBTC::CBTC(const CBTC &CC)
{
	n=CC.n;
	t.resize(n);
	C.resize(n);
	for (int i=0; i<n; i++)
	{
		t[i] = CC.t[i];
		C[i] = CC.C[i];
	}
	structured = CC.structured;
}

CBTC::CBTC(string Filename)
{
	n = 0;
	t.clear();
	C.clear();
	ifstream file(Filename);
	vector<string> s;
	structured = true;
	if (file.good())
	while (file.eof()== false)
	{
		s = getline(file);
		if (s.size()>0)
		if (s[0].substr(0,2)!="//")
		{
			t.push_back(atof(s[0].c_str()));
			C.push_back(atof(s[1].c_str()));
			n++;
			if (t.size()>2)
				if ((t[t.size()-1]-t[t.size()-2])!=(t[t.size()-2]-t[t.size()-3]))
					structured = false;
				
		}
	}
	file.close();
	
}

/*CBTC CBTC::operator = (const CBTC &CC)
{
	n=CC.n;
	t = new double[n];
	C = new double[n];
	for (int i=0; i<n; i++)
	{
		t[i] = CC.t[i];
		C[i] = CC.C[i];
	}
	
	return *this;
}*/

CBTC& CBTC::operator = (const CBTC &CC)
{
	n=CC.n;
	t.resize(n);
	C.resize(n);
	structured = CC.structured;
	for (int i=0; i<n; i++)
	{
		t[i] = CC.t[i];
		C[i] = CC.C[i];
	}
	
	return *this;
}

CBTC CBTC::Log()
{
	CBTC BTC = CBTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = log(C[i]);
	}
	return BTC;
}

CBTC CBTC::Abs()
{
	CBTC BTC = CBTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = fabs(C[i]);
	}
	return BTC;
}

CBTC CBTC::Log(double m)
{
	CBTC BTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = log(max(C[i],m));
	}
	return BTC;
}

CBTC CBTC::Exp()
{
	CBTC BTC(n);
	for (int i=0; i<n; i++)
	{
		BTC.t[i] = t[i];
		BTC.C[i] = exp(C[i]);
	}
	return BTC;
}

double CBTC::interpol(double x)
{
	double r=0;
	if (n>1)
	{
	
		if (structured == false)
		{	for (int i=0; i<n-1; i++)
			{
				if (t[i] <= x && t[i+1] >= x)
					r=(C[i+1]-C[i])/(t[i+1]-t[i])*(x-t[i]) + C[i];
			}
			if (x>t[n-1]) r=C[n-1];
			if (x<t[0]) r=C[0];
		}
		else
		{
			double dt = t[1]-t[0];
			int i = int((x-t[0])/dt);
			if (x>=t[n-1]) r=C[n-1];
			else if (x<=t[0]) r=C[0];
			else r=(C[i+1]-C[i])/(t[i+1]-t[i])*(x-t[i]) + C[i];
		}
	}
	else
		if (n==1) r = C[0];
	return r;

}

CBTC CBTC::interpol(vector<double> x)
{
	CBTC BTCout;
	for (int i=0; i<x.size(); i++)
		BTCout.append(x[i],interpol(x[i]));
	return BTCout;

}

CBTC CBTC::interpol(CBTC &x)
{
	CBTC BTCout;
	for (int i=0; i<x.n; i++)
		BTCout.append(x.t[i],interpol(x.t[i]));
	return BTCout;

}

double ADD(CBTC &BTC_p, CBTC &BTC_d)
{
	double sum = 0;	
	for (int i=0; i<BTC_d.n; i++)
		if (abs(BTC_d.C[i]) < 1e-3)
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));
		else
			sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i])) /BTC_d.C[i];	
		
	return sum/BTC_d.n;
}

double diff_relative(CBTC &BTC_A, CBTC &BTC_B, double m)
{
	double sum = 0;	
	for (int i=0; i<min(BTC_A.n,BTC_B.n); i++)
		if (abs(BTC_A.C[i]) < m)
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i]));
		else
			sum += abs(BTC_B.C[i] - BTC_A.interpol(BTC_B.t[i])) /BTC_A.C[i];	
		
	return sum;
}


double diff(CBTC BTC_p, CBTC BTC_d, int scale)
{
	double sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		if (BTC_d.C[i] > BTC_p.interpol(BTC_d.t[i]))
			sum += scale*pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
		else
			sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)/sqrt(1.0+scale*scale);
	}
	return sum;
}

double diff(CBTC &BTC_p, CBTC &BTC_d)
{
	double sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{	
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(BTC_d.C[i] - a,2);	
	}
	
	return sum;
}

double diff_abs(CBTC &BTC_p, CBTC &BTC_d)
{
	double sum = 0;
	
	for (int i=0; i<BTC_d.n; i++)
	{	
		sum += abs(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]));	
	}
	
	return sum;
}

double diff_log(CBTC &BTC_p, CBTC &BTC_d, double lowlim)
{
	double sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{	
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(log(max(BTC_d.C[i],lowlim)) - log(max(a,lowlim)),2);
	
	}
	
	return sum;
}

double diff_mixed(CBTC &BTC_p, CBTC &BTC_d, double lowlim, double std_n, double std_ln)
{
	
	CNormalDist ND;
	double sum = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{	
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += ND.likelihood_mixed(a,BTC_d.C[i],std_n,std_ln);
	}
	
	return sum;
}


double diff2(CBTC BTC_p, CBTC BTC_d)
{
	double sum = 0;
	double sumvar1 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{	
		sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2);
		sumvar1 += BTC_d.C[i]*BTC_d.C[i];
	}

	return sum;
}


double R2(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return pow(sumcov-sum1*sum2,2)/(sumvar1-sum1*sum1)/(sumvar2-sum2*sum2);
}

double R2_c(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	double totcount = min(BTC_d.n,BTC_p.n);
	for (int i=0; i<totcount; i++)
	{
		sumcov += fabs(BTC_d.C[i])*fabs(BTC_p.C[i])/totcount;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/totcount;
		sumvar2 += BTC_p.C[i]*BTC_p.C[i]/totcount;	
		sum1 += fabs(BTC_d.C[i])/totcount;
		sum2 += fabs(BTC_p.C[i])/totcount;
	}

	return pow(sumcov-sum1*sum2,2)/(sumvar1-sum1*sum1)/(sumvar2-sum2*sum2);
}

double R(CBTC BTC_p, CBTC BTC_d, int nlimit)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	int N = BTC_d.n - nlimit;

	for (int i=nlimit; i<BTC_d.n; i++)
	{
		double x1 = BTC_d.C[i];
		double x2 = BTC_p.C[i];
		sumcov += x1*x2/N;
		sumvar1 += x1*x1/N;
		sumvar2 += x2*x2/N;	
		sum1 += x1/N;
		sum2 += x2/N;
	}

	double R_x1x2 = (sumcov-sum1*sum2)/pow(sumvar1-sum1*sum1,0.5)/pow(sumvar2-sum2*sum2,0.5);

	return R_x1x2;
}

double XYbar(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumcov;
}

double X2bar(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar1;
}

double Y2bar(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sumvar2;
}

double Ybar(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum2;
}

double Xbar(CBTC BTC_p, CBTC BTC_d)
{
	double sumcov = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		double x2 = BTC_p.interpol(BTC_d.t[i]);
		sumcov += BTC_d.C[i]*x2/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += x2*x2/BTC_d.n;	
		sum1 += BTC_d.C[i]/BTC_d.n;
		sum2 += x2/BTC_d.n;
	}

	return sum1;
}

double diff_norm(CBTC &BTC_p, CBTC &BTC_d)
{
	double sum = 0;
	double sumvar1 = 0;
	double sumvar2 = 0;
	double a;
	for (int i=0; i<BTC_d.n; i++)
	{	
		a = BTC_p.interpol(BTC_d.t[i]);
		sum += pow(BTC_d.C[i] - a,2)/BTC_d.n;
		sumvar1 += BTC_d.C[i]*BTC_d.C[i]/BTC_d.n;
		sumvar2 += pow(a,2)/BTC_d.n;
	}
	//cout<<sum<<endl;
	return sum/sqrt(sumvar1*sumvar2);

}


double diff(CBTC BTC_p, CBTC BTC_d, CBTC Q)
{
	double sum = 0;
	for (int i=0; i<BTC_d.n; i++)
	{
		sum += pow(BTC_d.C[i] - BTC_p.interpol(BTC_d.t[i]),2)*pow(Q.interpol(BTC_d.t[i]),2);
	}
	return sum;
}

void CBTC::readfile(string Filename)
{
	ifstream file(Filename);
	vector<string> s;
	if (file.good())
	while (file.eof()== false)
	{
		s = getline(file);
		if (s.size()>0)
		if (s[0].substr(0,2)!="//")
		{
			t.push_back(atof(s[0].c_str()));
			C.push_back(atof(s[1].c_str()));
			n++;
			if (t.size()>2)
				if (t[t.size()-1]-t[t.size()-2]!=t[t.size()-2]-t[t.size()-3])
					structured = false;
				
		}
	}
	file.close();

}

/*void CBTC::readfile(CString Filename)
{
	FILE *FILEBTC;
	FILEBTC = fopen(Filename, "r");
	if (FILEBTC == NULL) 
		double e=1;
	int numpoints = 0;
	double tt, CC;
	while (feof(FILEBTC)==false)
	{	
		fscanf(FILEBTC, "%lf, %lf\n", &tt, &CC);
		numpoints++;
	}
	//numpoints--;
	fclose(FILEBTC);
	
	n=numpoints;
	t = new double[numpoints];
	C = new double[numpoints];

	FILEBTC = fopen(Filename, "r");
	for (int i=0; i<numpoints; i++)
	{	
		fscanf(FILEBTC, "%lf, %lf", &t[i], &C[i]);
	}
	fclose(FILEBTC);



}*/

void CBTC::writefile(string Filename)
{
	FILE *FILEBTC;
	FILEBTC = fopen(Filename.c_str(), "w");
	for (int i=0; i<n; i++)
		fprintf(FILEBTC, "%lf, %le\n", t[i], C[i]);
	
	fclose(FILEBTC);

}

/*void CBTC::writefile(CString Filename)
{
	FILE *FILEBTC;
	FILEBTC = fopen(Filename, "w");
	for (int i=0; i<n; i++)
		fprintf(FILEBTC, "%lf, %le\n", t[i], C[i]);
	
	fclose(FILEBTC);

}*/

/*double CBTC::GetS0(CBTC &M)
{
	double sumprod = 0;
	double sumsqr = 0;
	for (int i = 0; i<M.n; i++)
	{
		sumprod += M.C[i]*interpol(M.t[i]);
		sumsqr += interpol(M.t[i])*interpol(M.t[i]);
	}
	double S0 = sumprod/sumsqr;
	return S0;
}

double CBTC::GetS0(CBTC &M, CBTC &Q)
{
	double sumprod = 0;
	double sumsqr = 0;
	for (int i = 0; i<M.n; i++)
	{
		sumprod += M.C[i]*interpol(M.t[i])*pow(Q.interpol(M.t[i]),2);
		sumsqr += interpol(M.t[i])*interpol(M.t[i])*pow(Q.interpol(M.t[i]),2);
	}
	double S0 = sumprod/sumsqr;
	return S0;
}*/

CBTC operator*(double alpha, CBTC CBTC_T)
{
	CBTC S(CBTC_T.n);
	for (int i=0; i<CBTC_T.n; i++)
	{
		S.t[i] = CBTC_T.t[i];
		S.C[i] = alpha*CBTC_T.C[i];
	}

	return S;
}

CBTC operator*(CBTC CBTC_T, double alpha)
{
	CBTC S = CBTC_T;
	for (int i=0; i<CBTC_T.n; i++)
	{
		//S.t[i] = CBTC_T.t[i];
		S.C[i] = alpha*CBTC_T.C[i];
	}


	return S;
}

CBTC operator/(CBTC BTC1, CBTC BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.interpol(BTC1.t[i]);

	return S;

}

CBTC operator-(CBTC BTC1, CBTC BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]-BTC2.interpol(BTC1.t[i]);

	return S;
}

CBTC operator-(CBTC BTC1, double a)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]-a;

	return S;
}

CBTC operator>(CBTC BTC1, CBTC BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<min(BTC1.n,BTC2.n); i++)
		S.C[i] = BTC1.C[i]-BTC2.C[i];

	return S;
}

double norm2(CBTC BTC1)
{
	double sum=0;
	for (int i=0; i<BTC1.n; i++)
		sum+=pow(BTC1.C[i],2);

	return sum;
}

CBTC operator*(CBTC &BTC1, CBTC &BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]*BTC2.interpol(BTC1.t[i]);

	return S;
}

CBTC operator%(CBTC BTC1, CBTC BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]/BTC2.C[i];

	return S;
}
CBTC operator&(CBTC BTC1, CBTC BTC2)
{
	CBTC S = BTC1;
	for (int i=0; i<BTC1.n; i++)
		S.C[i] = BTC1.C[i]+BTC2.C[i];

	return S;


}

/*double CBTC::EMC(CBTC &M)
{
	double sum = 0;
	double sumflow = 0;
	for (int i=0; i<n; i++)
	{	
		sum += C[i]*M.interpol(t[i]);
		sumflow += M.interpol(t[i]);
	}
	if (sumflow == 0.0)
		return 0;
	else
		return sum/sumflow;
}

double CBTC::Calculate_load(CBTC &M)
{
	double sum = 0;
	double sumflow = 0;
	for (int i=0; i<n; i++)
	{	
		sum += C[i]*M.interpol(t[i])*(t[2]-t[1]);
		
	}
	
	return sum;
}*/

double CBTC::maxC()
{
	double max = -1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]>max)
			max = C[i];
	}
	return max;
}

double CBTC::minC()
{
	double min = 1e32;
	for (int i=0; i<n; i++)
	{	if (C[i]<min)
			min = C[i];
	}
	return min;
}

double CBTC::std()
{
	double sum = 0;
	double m = mean();
	for (int i=0; i<n; i++)
	{	
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

double CBTC::std(int nlimit)
{
	double sum = 0;
	double m = mean(nlimit);
	for (int i=nlimit; i<n; i++)
	{	
		sum+= pow(C[i]-m,2);
	}
	return sqrt(sum/n);
}

double CBTC::mean()
{
	double sum = 0;
	for (int i=0; i<n; i++)
	{	
		sum+= C[i];
	}
	if (n>0)
		return sum/n;
	else
		return 0;
}

double CBTC::integrate()
{
	double sum = 0;
	
	for (int i=1; i<n; i++)
	{	
		sum+= (C[i]+C[i-1])/2.0*(t[i]-t[i-1]);
	}
	return sum;
}

double CBTC::average()
{
	if (n>0)
		return integrate()/(t[n-1]-t[0]);
	else
		return 0;
}


double CBTC::percentile(double x)
{
	vector<double> X = QSort(C);
	int i = int(x*X.size());
	return X[i];

}

double CBTC::percentile(double x, int limit)
{
	vector<double> C1(C.size()-limit);
	for (int i=0; i<C1.size(); i++)
		C1[i] = C[i+limit];
	vector<double> X = QSort(C1);
	int ii = int(x*double(X.size()));
	return X[ii];

}

double CBTC::mean(int limit)
{
	double sum = 0;
	for (int i=limit; i<n; i++)
		sum += C[i];
	return sum/double(n-limit);
}

double CBTC::mean_log(int limit)
{
	double sum = 0;
	for (int i=limit; i<n; i++)
		sum += log(C[i]);
	return sum/double(n-limit);
}

void CBTC::append(double x)
{
	n++;
	t.push_back(0);
	C.push_back(x);

}

void CBTC::append(double tt, double xx)
{
	n++;
	t.push_back(tt);
	C.push_back(xx);
	if (t.size()>2)
	{
		if (t[t.size()-1]-t[t.size()-2]!=t[t.size()-2]-t[t.size()-3])
			structured = false;

	}

}

void CBTC::append(CBTC &CC)
{
	for (int i=0; i<CC.n; i++) append(CC.t[i],CC.C[i]);

}

CBTC& CBTC::operator+=(CBTC &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.interpol(t[i]);
	return *this;
}

CBTC& CBTC::operator%=(CBTC &v)
{
	for (int i=0; i<n; ++i)
		C[i] += v.C[i];
	return *this;

}

CBTC operator+(CBTC v1, CBTC v2) 
{
	return v1 += v2;
}

CBTC CBTC::make_uniform(double increment)
{
	CBTC out;
	out.structured = true;
	
	out.append(t[0],C[0]);
	for (int i=0; i<n-1; i++)
	{
		int i1 = int((t[i]-t[0])/increment);
		int i2 = int((t[i+1]-t[0])/increment);
		for (int j=i1+1; j<=i2; j++)
		{
			double x = j*increment+t[0];
			double CC = (x-t[i])/(t[i+1]-t[i])*(C[i+1]-C[i])+C[i];
			out.append(x,CC);
		}
	}
	
	return out;

}

double prcntl(vector<double> C, double x)
{
	vector<double> X = QSort(C);
	int ii = int(x*double(X.size()));
	return X[ii];

}

vector<double> prcntl(vector<double> C, vector<double> x)
{
	vector<double> X = QSort(C);
	vector<double> Xout = x;
	for(int j =0; j< x.size(); j++)
	{
		int ii = int(x[j]*double(X.size()));
		Xout[j] = X[ii];
	}
	
	return Xout;
}

CBTC CBTC::extract(double t1, double t2)
{
	CBTC out;
	for (int i=0; i<n; i++)
		if ((t[i]>=t1) && (t[i]<=t2))
			out.append(t[i], C[i]);

	return out;
}


CBTC CBTC::distribution(int n_bins, int limit)
{
	CBTC out(n_bins+2);

	CVector C1(C.size()-limit);
	for (int i=0; i<C1.num; i++)
		C1[i] = C[i+limit];	
	
	double p_start = min(C1);
	double p_end = max(C1)*1.001;
	double dp = abs(p_end - p_start)/n_bins;

	out.t[0] = p_start - dp/2;	
	out.C[0] = 0;
	for (int i=0; i<n_bins+1; i++)
	{
		out.t[i+1] = out.t[i] + dp;
		out.C[i+1] = out.C[i];
	}
		
	for (int i=0; i<C1.num; i++)
		out.C[int((C1[i]-p_start)/dp)+1] += 1.0/C1.num/dp;

	return out;
}

vector<double> CBTC::trend()
{
	double x_bar = mean_t();
	double y_bar = mean();
	double sum_num = 0;
	double sum_denom = 0;
	for (int i=0; i<n; i++)
	{
		sum_num+=(t[i]-x_bar)*(C[i]-y_bar);
		sum_denom+=(t[i]-x_bar)*(t[i]-x_bar);
	}
	vector<double> out(2);
	out[1] = sum_num/sum_denom;
	out[0] = y_bar-out[1]*x_bar;
	return out;

}

double CBTC::mean_t()
{
	double sum = 0;
	for (int i=0; i<n; i++)
		sum += t[i];
	return sum/double(n);

}

int sgn(int val) {
    return (int(0) < val) - (val < int(0));
}

double sgn(double val) {
    return double(double(0) < val) - (val < int(0));
}

CBTC CBTC::add_noise(double std, bool logd)
{
	CBTC X(n);
	for (int i=0; i<n; i++)
	{
		X.t[i] = t[i];
		if (logd==false)
			X.C[i] = C[i]+getnormalrand(0,std);
		else
			X.C[i] = C[i]*exp(getnormalrand(0,std));
	}
	return X;

}

void CBTC::clear()
{
	C.clear();
	t.clear();
	n = 0;
}

CBTC CBTC::getcummulative()
{
	CBTC X(n);
	X.t = t;
	X.C[0] = 0;
	for (int i=1; i<n; i++)
		X.C[i]=X.C[i-1]+(X.t[i]-X.t[i-1])*0.5*(C[i]+C[i-1]);

	return X;
}

CBTC max(CBTC A,double b)
{
	CBTC S = A;
	for (int i=0; i<A.n; i++)
		S.C[i]=max(A.C[i],b);
	return S;
}