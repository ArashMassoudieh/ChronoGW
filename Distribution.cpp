#include "Distribution.h"


CDistribution::CDistribution(void)
{
	pi=4*atan(1.0);
}


CDistribution::~CDistribution(void)
{
}

CDistribution::CDistribution(string _name)
{
	pi=4*atan(1.0);
	name = _name;
	if (name=="normal") params.resize(2);
	if (name=="lognormal") params.resize(2);
	if (name=="levy") params.resize(1);
	if (name=="shifted_levy") params.resize(2);
	if (name=="exp") params.resize(1);
	if (name=="shifted_exp") params.resize(2);
	if (name=="invgaussian") params.resize(2);
	if (name=="gamma") params.resize(2);
	if (name=="gamma") params.resize(2);
	if (name=="generalized_igaussian") params.resize(3);
}

double CDistribution::evaluate(double x)
{
	if (name=="normal")
		return 1/(2*sqrt(pi)*params[1])*exp(-pow(x-params[0],2)/(2*params[1]*params[1]));
	if (name=="lognormal")
		return 1/(2*sqrt(pi)*params[1]*x)*exp(-pow(log(x)-log(params[0]),2)/(2*params[1]*params[1]));
	if (name=="levy")
		return sqrt(params[0]/(2*pi))*exp(-params[0]/(2*x))/pow(x,1.5);
	if (name=="shifted_levy")
	{	if (x>params[1])
			return sqrt(params[0]/(2*pi))*exp(-params[0]/(2*(x-params[1])))/pow(x-params[1],1.5);
		else
			return 0;
	}
	if (name=="exp")
		return 1/params[0]*exp(x/params[0]);
	if (name=="shifted_exp")
	{
		if (x>params[1])
			return 1/params[0]*exp((x-params[1])/params[0]);
		else
			return 0;
	}
	if (name=="invgaussian")
		return sqrt(params[1]/(2*pi*pow(x,3)))*exp(-params[1]*pow(x-params[0],2)/(2*params[0]*params[0]*x));
	if (name=="gamma") 
		return 0;

}

double Gammapdf(double x, double k, double theta)
{
	return 1/pow(theta, k)*pow(x, k-1)*exp(-x/theta)/gamma(k);
}

double gamma(double x)
{
    int i,k,m;
    double ga,gr,r,z;
	double M_PI = atan(1.0)*4.0;
    static double g[] = {
        1.0,
        0.5772156649015329,
       -0.6558780715202538,
       -0.420026350340952e-1,
        0.1665386113822915,
       -0.421977345555443e-1,
       -0.9621971527877e-2,
        0.7218943246663e-2,
       -0.11651675918591e-2,
       -0.2152416741149e-3,
        0.1280502823882e-3,
       -0.201348547807e-4,
       -0.12504934821e-5,
        0.1133027232e-5,
       -0.2056338417e-6,
        0.6116095e-8,
        0.50020075e-8,
       -0.11812746e-8,
        0.1043427e-9,
        0.77823e-11,
       -0.36968e-11,
        0.51e-12,
       -0.206e-13,
       -0.54e-14,
        0.14e-14};

    if (x > 171.0) return 1e308;    // This value is an overflow flag.
    if (x == (int)x) {
        if (x > 0.0) {
            ga = 1.0;               // use factorial
            for (i=2;i<x;i++) {
               ga *= i;
            }
         }
         else
            ga = 1e308;
     }
     else {
        if (fabs(x) > 1.0) {
            z = fabs(x);
            m = (int)z;
            r = 1.0;
            for (k=1;k<=m;k++) {
                r *= (z-k);
            }
            z -= m;
        }
        else
            z = x;
        gr = g[24];
        for (k=23;k>=0;k--) {
            gr = gr*z+g[k];
        }
        ga = 1.0/(gr*z);
        if (fabs(x) > 1.0) {
            ga *= r;
            if (x < 0.0) {
                ga = -M_PI/(x*ga*sin(M_PI*x));
            }
        }
    }
    return ga;
}


CDistribution::CDistribution(int nn)
{
	pi=4*atan(1.0);
	n = nn;
	e.resize(n);
	s.resize(n);
}

CDistribution::CDistribution(const CDistribution &C)
{
	pi=4*atan(1.0);
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i=0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}


}

CDistribution CDistribution::operator = (const CDistribution &C)
{
	pi=4*atan(1.0);
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i=0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}

	return *this;

}

int CDistribution::GetRand()
{
	double x = GetRndUniF(0,1);
	int ii = 0;
	for (int i=0; i<n-1; i++)
	{	
		if (x<e[i] && x>s[i])
			ii = i;
	}
	return ii;

}