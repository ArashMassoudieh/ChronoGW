#include "Tracer.h"
#include "StringOP.h"
#include "Well.h"

CTracer::CTracer(void)
{

}


CTracer::~CTracer(void)
{
}

CTracer::CTracer(const CTracer &m)
{
	input = m.input;
	source = m.source;
	input_multiplier=m.input_multiplier;
	co = m.co;
	decay_rate=m.decay_rate;
	retard=m.retard;
	fm_max=m.fm_max;
	SourceTr = m.SourceTr;
	obs_std=m.obs_std;
	name = m.name;
	cm = m.cm;
	linear_prod=m.linear_prod;
	vz_delay = m.vz_delay;
	constant_input = m.constant_input;
	constant_input_val = m.constant_input_val;	
	conc = m.conc;
}

CTracer& CTracer::operator=(const CTracer &m)
{
	input = m.input;
	source = m.source;
	input_multiplier=m.input_multiplier;
	co = m.co;
	decay_rate=m.decay_rate;
	retard=m.retard;
	fm_max=m.fm_max;
	SourceTr = m.SourceTr;
	obs_std=m.obs_std;
	name = m.name;
	cm = m.cm;
	linear_prod=m.linear_prod;
	vz_delay = m.vz_delay;
	constant_input = m.constant_input;
	constant_input_val = m.constant_input_val;	
	conc = m.conc;
	return *this;
}

double CTracer::calc_conc(double t, CBTC &young_dist, double f, double _vz_delay, bool fixed_old_conc, double age_old, double fm)
{
	double sum=0;
	double vz1=0;
	double vz2=0; 
	if (vz_delay==true) vz1=_vz_delay;
	if (source!="") if (SourceTr->vz_delay==true) vz2=_vz_delay;
	if (source=="")
	{	if (linear_prod==false)
		{
			for (int i=1; i<young_dist.n; i++)
			{
				double a1,a2;
				a1=input_multiplier*young_dist.C[i-1]*input.interpol(t-retard*(young_dist.t[i-1]+vz1))*exp(-decay_rate*retard*(young_dist.t[i-1]+vz1));
				a2=input_multiplier*young_dist.C[i]*input.interpol(t-retard*(young_dist.t[i]+vz1))*exp(-decay_rate*retard*(young_dist.t[i]+vz1));
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
		}
		else
		{
			for (int i=1; i<young_dist.n; i++)
			{
				double a1=input_multiplier*young_dist.C[i-1]*(input.interpol(t-retard*(young_dist.t[i-1]+vz1))+decay_rate*retard*(young_dist.t[i-1]+vz1));
				double a2=input_multiplier*young_dist.C[i]*(input.interpol(t-retard*(young_dist.t[i]+vz1))+decay_rate*retard*(young_dist.t[i]+vz1));
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
		}
	}
	else
	{
		for (int i=1; i<young_dist.n; i++)
			{
				double a1,a2;
				a1=SourceTr->input_multiplier*young_dist.C[i-1]*SourceTr->input.interpol(t-SourceTr->retard*(young_dist.t[i-1]+vz2))*(1-exp(-SourceTr->decay_rate*SourceTr->retard*young_dist.t[i-1]))*exp(-SourceTr->decay_rate*SourceTr->retard*vz2);
				a2=SourceTr->input_multiplier*young_dist.C[i]*SourceTr->input.interpol(t-SourceTr->retard*(young_dist.t[i]+vz2))*(1-exp(-SourceTr->decay_rate*SourceTr->retard*young_dist.t[i]))*exp(-SourceTr->decay_rate*SourceTr->retard*vz2);
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
	}
	if (fixed_old_conc==true)
		sum = sum*(1-f) + (1-fm*fm_max)*f*co + fm*fm_max*cm;
	else
		if (linear_prod==false)
			sum = sum*(1-f) + input_multiplier*(1-fm*fm_max)*f*input.interpol(t-retard*(age_old+vz1))*exp(-decay_rate*retard*(age_old+vz1))+ fm*fm_max*cm;
		else
			sum = sum*(1-f) + input_multiplier*(1-fm*fm_max)*f*(input.interpol(t-retard*(age_old+vz1))+decay_rate*retard*(age_old+vz1))+ fm*fm_max*cm;
	
	return sum;
}

double CTracer::calc_concFromMatrix(double t, vector<double> params, double f, double _vz_delay, bool fixed_old_conc, double age_old, double fm)
{
	
	// 0: Dispersion coefficient 
	// 1: x_gw
	// 2: x_vz
	// 3: v

		
	double sum = 0;
	double vz1 = 0;
	double vz2 = 0;
	vector<double> mu_lambda(2);
	double x;
	if (vz_delay)
	{
		x = params[1] + params[2];
	}
	else
	{
		x = params[1];
	}
	double D = params[0];
	double v = params[3];
	mu_lambda[0] = x / v;
	mu_lambda[1] = x*x / 2 / D;

	if (linear_prod)
	{
		return constant_input_val + decay_rate*mu_lambda[0];
	}

	//if (vz_delay == true) vz1 = _vz_delay;
	if (source != "") if (SourceTr->vz_delay == true) vz2 = _vz_delay;
	sum += (1 - fm*fm_max)*interpolationMatrix(t, mu_lambda);


	
	if (fixed_old_conc == true)
		sum = sum*(1 - f) + (1 - fm*fm_max)*f*co + fm*fm_max*cm;
	else
		if (linear_prod == false)
			sum = sum*(1 - f) + input_multiplier*(1 - fm*fm_max)*f*input.interpol(t - retard*(age_old + vz1))*exp(-decay_rate*retard*(age_old + vz1)) + fm*fm_max*cm;
		else
			sum = sum*(1 - f) + input_multiplier*(1 - fm*fm_max)*f*(input.interpol(t - retard*(age_old + vz1)) + decay_rate*retard*(age_old + vz1)) + fm*fm_max*cm;

	return sum;
}
/*double CTracer::calc_conc(double t, CBTC &young_dist, double f, double _vz_delay, bool fixed_old_conc, double age_old, double fm)
{
	//ts = QSort(
	double sum=0;
	double vz1=0;
	double vz2=0; 
	if (vz_delay==true) vz1=_vz_delay;
	if (source!=-1) if (SourceTr->vz_delay==true) vz2=_vz_delay;
	if (source==-1)
	{	if (linear_prod==false)
		{
			for (int i=1; i<young_dist.n; i++)
			{
				double a1,a2;
				a1=input_multiplier*young_dist.C[i-1]*input.interpol(t-retard*(young_dist.t[i-1]+vz1))*exp(-decay_rate*retard*(young_dist.t[i-1]+vz1));
				a2=input_multiplier*young_dist.C[i]*input.interpol(t-retard*(young_dist.t[i]+vz1))*exp(-decay_rate*retard*(young_dist.t[i]+vz1));
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
		}
		else
		{
			for (int i=1; i<young_dist.n; i++)
			{
				double a1=input_multiplier*young_dist.C[i-1]*(input.interpol(t-retard*(young_dist.t[i-1]+vz1))+decay_rate*retard*(young_dist.t[i-1]+vz1));
				double a2=input_multiplier*young_dist.C[i]*(input.interpol(t-retard*(young_dist.t[i]+vz1))+decay_rate*retard*(young_dist.t[i]+vz1));
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
		}
	}
	else
	{
		for (int i=1; i<young_dist.n; i++)
			{
				double a1,a2;
				a1=SourceTr->input_multiplier*young_dist.C[i-1]*SourceTr->input.interpol(t-SourceTr->retard*(young_dist.t[i-1]+vz2))*(1-exp(-SourceTr->decay_rate*SourceTr->retard*young_dist.t[i-1]))*exp(-SourceTr->decay_rate*SourceTr->retard*vz2);
				a2=SourceTr->input_multiplier*young_dist.C[i]*SourceTr->input.interpol(t-SourceTr->retard*(young_dist.t[i]+vz2))*(1-exp(-SourceTr->decay_rate*SourceTr->retard*young_dist.t[i]))*exp(-SourceTr->decay_rate*SourceTr->retard*vz2);
				sum += (1-fm*fm_max)*0.5*(a1+a2)*(young_dist.t[i]-young_dist.t[i-1]);
			}
	}
	if (fixed_old_conc==true)
		sum = sum*(1-f) + (1-fm*fm_max)*f*co + fm*fm_max*cm;
	else
		if (linear_prod==false)
			sum = sum*(1-f) + input_multiplier*(1-fm*fm_max)*f*input.interpol(t-retard*(age_old+vz1))*exp(-decay_rate*retard*(age_old+vz1))+ fm*fm_max*cm;
		else
			sum = sum*(1-f) + input_multiplier*(1-fm*fm_max)*f*(input.interpol(t-retard*(age_old+vz1))+decay_rate*retard*(age_old+vz1))+ fm*fm_max*cm;
	
	return sum;
}*/


void CTracer::set_val(string S, double val)
{
	if (tolower(S)=="co") co = val;
	if (tolower(S)=="cm") cm = val;
	if (tolower(S)=="decay") decay_rate = val;  
	if (tolower(S)=="fm") fm_max = val;  
	if (tolower(S)=="retard") retard = val;  
	if (tolower(S)=="input_multiplier") input_multiplier = val; 
	if (tolower(S)=="vz_delay") vz_delay = val;
	if (tolower(S)=="constant_input") 
		{
			constant_input_val = val;
			constant_input = true;
		}				
}		
double CTracer::interpolationMatrix(double t, vector<double> params)
{
	/*	0: mu
		1: lambda
		2: year 
		3: decay rate
		*/
	int k = 0;
	if (ndecay_rate != 1)
		k = decayrateInv(decay_rate);
	///////////
	float y = yearInv(t);
	float m = muInv(params[0]);
	float l = lambdaInv(params[1]);


	int y0 = floor(y);
	int m0 = floor(m);
	int l0 = floor(l);

	CMatrix conc2(2, 2);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			conc2[i][j] = conc[k][y0][m0+i][l0+j] + (conc[k][y0 + 1][m0+i][l0+j] - conc[k][y0][m0+i][l0+j]) / (year(y0 + 1) - year(y0))*(t - year(y0));
	vector<double> conc3;
	conc3.resize(2);
	for (int j = 0; j < 2; j++)
		conc3[j] = conc2[0][0] + (conc2[0][j] - conc2[0][0]) / (lambda(l0 + 1) - lambda(l0))*(params[1] - lambda(l0));
	
	return conc3[0] + (conc3[1] - conc3[0]) / (mu(m0 + 1) - mu(m0))*(params[0] - mu(m0));

}

void CTracer::calcMatrix()
{
	if (linear_prod)
	{
		return;
	}
	conc.reserve(ndecay_rate);
	cout << "\n" + name;

	for (int k = 0; k < ndecay_rate; k++)
	{
		decay_rate = decayrate(k);
		conc[k].resize(numberofSteps); //Number of years ,

		cout << "\n" + name;
		for (int y = 0; y < numberofSteps; y++)
		{
			int _year = year(y);
			cout << "\n" + std::to_string(_year) + "\n";
			conc[k][y] = CMatrix(nmu, nlambda);
			for (int i = 0; i < nmu; i++)
			{
				cout << std::to_string((100 * i) / nmu) + "% ";
				float _mu = mu(i);
				for (int j = 0; j < nlambda; j++)
				{
					float _lambda = lambda(j);
					vector<double> params;
					params.push_back(_mu);
					params.push_back(_lambda);
					CBTC dist = CWell::creat_age_dist_InvG_mu_lambda(params, 500, 500, 0.05);
					//			dist.writefile("out/distributions/age_dist mu(" + std::to_string(mu) + ") lambda(" + std::to_string(lambda) + ").txt");
					conc[k][y][i][j] = calc_conc(_year, dist, 0);
				}
			}
		}
	}
	//		conc[y].writetofile("out/tracers/" + name + "-" + std::to_string(year) + ".txt");
}

int CTracer::decayrate(int index)
{
	if (ndecay_rate == 1) return decay_rate;
	return exp(log(min_decayrate) + log(max_decayrate / min_decayrate) / ndecay_rate*(index + 1));
}
float CTracer::decayrateInv(int decayrate)
{
	if (ndecay_rate == 1) return 0;
	return ndecay_rate*(log(decayrate) - log(min_decayrate)) / (log(max_decayrate / min_decayrate)) - 1;
}


float CTracer::year(int index)
{
	return startYear + index / numberofSteps * (endYear - startYear);
}

float CTracer::yearInv(float year)
{
	return 1.0 * (year - startYear) * numberofSteps / (endYear - startYear);
}

double CTracer::mu(int index)
{
	return exp(log(min_mu) + log(max_mu / min_mu) / nmu*(index + 1));
}

int CTracer::muInv(double mu)
{
	return nmu*(log(mu) - log(min_mu)) / (log(max_mu / min_mu)) - 1;
}

double CTracer::lambda(int index)
{
	return exp(log(min_lambda) + log(max_lambda / min_lambda) / nlambda*(index + 1));
}

int CTracer::lambdaInv(double lambda)
{
	return nlambda*(log(lambda) - log(min_lambda)) / (log(max_lambda / min_lambda)) - 1;
}