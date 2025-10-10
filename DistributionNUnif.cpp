// Distribution.cpp: implementation of the CDistributionNUnif class.
//
//////////////////////////////////////////////////////////////////////

#include "DistributionNUnif.h"
#include "math.h"
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDistributionNUnif::CDistributionNUnif()
{
	
	set = false;
	
}

CDistributionNUnif::CDistributionNUnif(int n_n)
{
	n = n_n;
	x.resize(n);
	y.resize(n);
	
	set = false;
}

CDistributionNUnif::CDistributionNUnif(const CDistributionNUnif &D)
{
	n = D.n;
	x = D.x;
	y = D.y;
	
	set = D.set;
}

CDistributionNUnif CDistributionNUnif::operator = (const CDistributionNUnif &D)
{
	n = D.n;
	x = D.x;
	y = D.y;
	set = D.set;

	return *this;
}

CDistributionNUnif::~CDistributionNUnif()
{
	
}

void CDistributionNUnif::initializeGamma(double dx0, double dxmult, int nint, double r, double lambda)
{
	double mmax = dx0*pow(dxmult,n);
	x[0] = 0;
	y[0] = 0;
	double x0 = 0;
	double x1 = dx0;
	for (int i=1; i<=n; i++)
	{	y[i] = y[i-1];
		x[i] = x1;
		double dx = (x1-x0)/static_cast<int>(nint);
		for (double xi = x0+dx/2; xi<=x1; xi+=dx)
			y[i] += Gammapdf(xi,r,lambda)*dx;
		if (i==1)
		{	
			x0 = dx0;
			x1 = dx0*dxmult;
		}
		else
		{	
			x0 = x0*dxmult;
			x1 = x0*dxmult;
		}

	}
	set = true;	
	symetrical = false;

}


void CDistributionNUnif::initializeNormal(double dx0, double dxmult,int nint)
{
	double mmax = dx0*pow(dxmult,n);
	x[0] = 0;
	y[0] = 0;
	double x0 = 0;
	double x1 = dx0;
	for (int i=1; i<=n; i++)
	{	y[i] = y[i-1];
		x[i] = x1;
		double dx = (x1-x0)/static_cast<int>(nint);
		for (double xi = x0+dx/2; xi<=x1; xi+=dx)
			y[i] += NormalStdpdf(xi)*dx;
		if (i==1)
		{	
			x0 = dx0;
			x1 = dx0*dxmult;
		}
		else
		{	
			x0 = x0*dxmult;
			x1 = x0*dxmult;
		}

	}
	set = true;		
	symetrical = true;

}


double NormalStdpdf(double x)
{
	double pi = atan(1.0)*4;
	return 1/sqrt(2*pi)*exp(-0.5*x*x);
}



double CDistributionNUnif::GetRndNorm(double mean, double std)
{
	if (set == true)
	{
		double xi = GetRndUniF(0, 1)-0.5;
		if (xi>0)
		{	for (int i=0; i<n; i++)
				if ((xi>y[i]) && (xi<y[i+1]))
					return (x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(xi-y[i]))*std + mean;
		}
		else
		{
			for (int i=0; i<n; i++)
				if ((-xi>y[i]) && (-xi<y[i+1]))
					return (-(x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(-xi-y[i])))*std + mean;
		}
	}
	else
		return -1;
}

double CDistributionNUnif::GetRndGamma()
{
	if (set == true)
	{
		double xi = GetRndUniF(0, 1);
		{	for (int i=0; i<n; i++)
				if ((xi>y[i]) && (xi<y[i+1]))
					return x[i] + (x[i+1]-x[i])/(y[i+1]-y[i])*(xi-y[i]);
		}
	}
	else
		return -1;

}

double GetRndUniF(double xmin, double xmax)
{
	double a = double(rand());
	double k = double(RAND_MAX);
	return a/k*(xmax-xmin) + xmin;
}
