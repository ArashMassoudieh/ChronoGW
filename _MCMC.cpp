// MCMC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MCMC.h"
#include <vector>
#include "normaldist.h"
#include <string>
#include <omp.h>


CMCMC::CMCMC(void)
{

}


CMCMC::~CMCMC(void)
{
	Params.clear();
	MCMCParam.clear();
	logp1.clear();
	logp.clear();

}

CBTCSet CMCMC::model(vector<double> par)
{
	double sum = 0;
	CBTCSet res;
	
	CGWA G1 = G;
	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)	
		{	if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2))/G.Well.size();
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2))/G.Well.size();
			G1.setparams(params[i],par[getparamno(i,0)]);
		}
		else
		{
			if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			G1.setparams(params[i],par[getparamno(i,0)]);
		}	
	}
		
	sum+=G1.getlogp();
		
	res = G1.modeled;
	
	return res;

}

CBTCSet CMCMC::model_lumped(vector<double> par)
{
	double sum = 0;
	CBTCSet res;
	
	CGWA G1 = G;
	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)	
		{	if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2))/G.Well.size();
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2))/G.Well.size();
			G1.setparams(params[i],par[getparamno(i,0)]);
		}
		else
		{
			if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			G1.setparams(params[i],par[getparamno(i,0)]);
		}	
	}
		
	sum+=G1.getlogp();
		
	res=G1.modeled;
	
	return res;

}

double CMCMC::posterior(vector<double> par)
{
	
	double sum = 0;
		
	CGWA G1 = G;
		
	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)	
		{	if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 3) if ((par[getparamno(i,0)]<MCMCParam[i].low) || (par[getparamno(i,0)]>MCMCParam[i].high)) sum-=200;
			G1.setparams(params[i],par[getparamno(i,0)]);
		}
		else
		{
			if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 3) if ((par[getparamno(i,0)]<MCMCParam[i].low) || (par[getparamno(i,0)]>MCMCParam[i].high)) sum-=200;
			
			G1.setparams(params[i],par[getparamno(i,0)]);
		}	
	}
	
	sum+=G1.getlogp();
	
	return sum; 
}

double CMCMC::posterior(vector<double> par, bool out)
{
	CGWA G1 = G;
	double sum = 0;
	
	
	for (int i=0; i<nActParams; i++)
		G1.setparams(params[i],par[getparamno(i,0)]);
		
	

	for (int i=0; i<nActParams; i++)
	{
		if (apply_to_all[i] == true)	
		{	if (MCMCParam[i].type == 1) sum -= pow(par[params[getparamno(i,0)]]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[params[getparamno(i,0)]])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 3) if ((par[params[getparamno(i,0)]]<MCMCParam[i].low) || (par[params[getparamno(i,0)]]>MCMCParam[i].high)) sum-=200;
			G.setparams(params[i],par[getparamno(i,0)]);
		}
		else
		{		
			if (MCMCParam[i].type == 1) sum -= pow(par[getparamno(i,0)]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 2) sum -= pow(log(par[getparamno(i,0)])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2));
			if (MCMCParam[i].type == 3) if ((par[getparamno(i,0)]<MCMCParam[i].low) || (par[getparamno(i,0)]>MCMCParam[i].high)) sum-=200;
			G.setparams(params[i],par[getparamno(i,0)]);
		}	
	}
	
	G1 = G;
	sum+=G1.getlogp();
			

	if (out) G_out = G1;
	return sum;

}

void CMCMC::initialize()
{
	double pp=0;
	for (int j=0; j<n_chains; j++) 
	{	for (int i=0; i<n; i++)
		{	if (MCMCParam[i].loged==1)
				Params[j][i] = exp(log(MCMCParam[i].low)+(log(MCMCParam[i].high-MCMCParam[i].low))*ND.unitrandom());
			else
				Params[j][i] = MCMCParam[i].low+(MCMCParam[i].high-MCMCParam[i].low)*ND.unitrandom();
			if (MCMCParam[get_act_paramno(i)].loged==1) 
				pp += log(Params[j][i]);
		}	
		logp[j] = posterior(Params[j]);
		logp1[j] = logp[j]+pp;
	}
}




void CMCMC::initialize(vector<double> par)
{
	
	
	for (int j=0; j<nActParams; j++)
	{
		if ((MCMCParam[j].type == 1) || (MCMCParam[j].type == 3))
		{
			MCMCParam[j].mean = 0.5*(MCMCParam[j].low+MCMCParam[j].high);
			MCMCParam[j].std = (MCMCParam[j].high - MCMCParam[j].mean)/2.0;
			
		}
		if (MCMCParam[j].type == 2)
		{
			MCMCParam[j].mean = sqrt(MCMCParam[j].low*MCMCParam[j].high);
			MCMCParam[j].std = (log(MCMCParam[j].high) - log(MCMCParam[j].mean))/2.0;
		}

	}

	if (sensbasedpurt==true)
	{	CVector X = sensitivity(1e-4,par);
	
		for (int j=0; j<nActParams; j++)
		{
			if ((MCMCParam[j].type == 1) || (MCMCParam[j].type == 3))
			{
				pertcoeff[j] = purt_fac/fabs(X[getparamno(j,0)]);
			}
			if (MCMCParam[j].type == 2)
			{
				pertcoeff[j] = purt_fac/fabs(sqrt(par[j])*X[getparamno(j,0)]);
			}

		}
	}
	else
	for (int j=0; j<nActParams; j++)
		{
			if ((MCMCParam[j].type == 1) || (MCMCParam[j].type == 3))
			{
				pertcoeff[j] = purt_fac*(-MCMCParam[j].low+MCMCParam[j].high);
			}
			if (MCMCParam[j].type == 2)
			{
				pertcoeff[j] = purt_fac*(-log(MCMCParam[j].low)+log(MCMCParam[j].high));
			}
		}
	double alpha;
	if (noinipurt==true) alpha = 0; else alpha = 1;
	

	for (int j=0; j<n_chains; j++)
	{	Params[j].resize(n);
		double pp=0;
		for (int i=0; i<n; i++)
		{
			if (MCMCParam[get_act_paramno(i)].loged==0) Params[j][i] = par[i]+alpha*ND.getnormalrand(0,pertcoeff[get_act_paramno(i)]); else Params[j][i] = par[i]*exp(alpha*ND.getnormalrand(0,pertcoeff[get_act_paramno(i)]));
			if (MCMCParam[get_act_paramno(i)].loged==2) while ((Params[j][i]<MCMCParam[get_act_paramno(i)].low) ||  (Params[j][i]>MCMCParam[get_act_paramno(i)].high))
			while ((Params[j][i]<MCMCParam[get_act_paramno(i)].low) ||  (Params[j][i]>MCMCParam[get_act_paramno(i)].high))
				if (MCMCParam[get_act_paramno(i)].loged==0) Params[j][i] = par[i]+alpha*ND.getnormalrand(0,pertcoeff[get_act_paramno(i)]); else Params[j][i] = par[i]*exp(alpha*ND.getnormalrand(0,pertcoeff[get_act_paramno(i)]));
			
			}
		logp[j] = posterior(Params[j]);
		logp1[j] = logp[j]+pp;
	}
}

bool CMCMC::step(int k)
{
	
	vector<double> X = purturb(k-n_chains);
	
	double logp_0 = posterior(X);
	double logp_1 = logp_0;
	bool res;
	for (int i=0; i<n; i++)
	{
		
		/*if (MCMCParam[get_act_paramno(i)].loged==1) 
		{	
			if ((X[i]<MCMCParam[get_act_paramno(i)].low*exp(-4*MCMCParam[get_act_paramno(i)].std)) || (X[i]>MCMCParam[get_act_paramno(i)].high*exp(4*MCMCParam[get_act_paramno(i)].std)))
				logp_1 += -200;
		}
		else if (MCMCParam[get_act_paramno(i)].loged==0) 
		{	if ((X[i]<MCMCParam[get_act_paramno(i)].low-4*MCMCParam[get_act_paramno(i)].std) || (X[i]>MCMCParam[get_act_paramno(i)].high+4*MCMCParam[get_act_paramno(i)].std))
				logp_1 += -200;
		}
		else if (MCMCParam[get_act_paramno(i)].loged==2) 
		{	if ((X[i]<MCMCParam[get_act_paramno(i)].low) || (X[i]>MCMCParam[get_act_paramno(i)].high))
				logp_1 += -200;
		}*/
	}
	if (ND.unitrandom()<exp(logp_1-logp1[k-n_chains]))
	{
		res=true;
		Params[k] = X;
		logp[k] = logp_0;
		logp1[k] = logp_1;
	}
	else
	{
		res = false;
		Params[k] = Params[k-n_chains];
		logp[k] = logp[k-n_chains];
		logp1[k] = logp1[k-n_chains];

	}
	return res;
}

vector<double> CMCMC::purturb(int k)
{
	vector<double> X;
	X.resize(n);
	for (int i=0; i<n; i++)
	{
		if (MCMCParam[get_act_paramno(i)].loged == 1)
			X[i] = Params[k][i]*exp(pertcoeff[get_act_paramno(i)]*ND.getstdnormalrand());
		else
			X[i] = Params[k][i]+pertcoeff[get_act_paramno(i)]*ND.getstdnormalrand();

	}
	return X;
}

bool CMCMC::step(int k, int nsamps, string filename)
{
	FILE *file;
	if (continue_mcmc == false)
	{	file = fopen(filename.c_str(),"w");
		fclose(file);
	}
	

	char buffer[33];
	if (continue_mcmc == false)
	{
		file = fopen(filename.c_str(),"a");
		fprintf(file,"%s, ", "no.");
			for (int i=0; i<n; i++)
				fprintf(file, "%s, ", paramname[i].c_str());
		fprintf(file,"%s, %s, %s,", "logp", "logp_1", "stuck_counter");
		for (int j=0; j<pertcoeff.size(); j++) fprintf(file,"%s,", string("purt_coeff_" + string(_itoa(j,buffer,10))).c_str());
		fprintf(file, "\n");
		fclose(file);
	}

	CVector stuckcounter(n_chains);
	for (int kk=k; kk<k+nsamps; kk+=n_chains)
	{
#pragma omp parallel for
		for (int jj = kk; jj<kk+n_chains; jj++)
		{	bool stepstuck = !step(jj);
		
			if (stepstuck==true)
				stuckcounter[jj-kk]++;
			else 
				stuckcounter[jj-kk]=0;
		
#pragma omp critical
			if (jj%writeinterval==0)
			{   file = fopen(filename.c_str(),"a");
				fprintf(file,"%i, ", jj);
				for (int i=0; i<n; i++)
				fprintf(file, "%le, ", Params[jj][i]);
				fprintf(file,"%le, %le, %f,", logp[jj], logp1[jj], stuckcounter[jj-kk]);
				for (int j=0; j<pertcoeff.size(); j++) fprintf(file,"%le,", pertcoeff[j]);
				fprintf(file, "\n");
				fclose(file);
			}
		
			cout<<jj<<","<<pertcoeff[0]<<","<<stuckcounter.max()<<","<<stuckcounter.min()<<endl;
			if (stuckcounter.max()>stucklimit)
			{	for (int i=0; i<nActParams; i++) pertcoeff[i]*=purtscale;
				stuckcounter = 0;
			}
	
		}
	}
	
	return 0;
}

void CMCMC::writeoutput(string filename)
{
	FILE *file;
	file = fopen(filename.c_str(),"a");
	for (int i=0; i<n; i+writeinterval)
		fprintf(file, "%s, ", paramname[i].c_str());
	fprintf(file,"%s\n", "logp");
	for (int j=0; j<n; j++)
	{
		for (int i=0; i<n; i+writeinterval)
			fprintf(file, "%le, ", Params[j][i]);

		fprintf(file,"%le\n", logp[j]);
	}
	fclose(file);
}

void CMCMC::getfromGA(const CGA &GA)
{
	logtrans = GA.logtrans;
	G = GA.Sys;
	MCMCParam.resize(GA.params.size());
	params = GA.params;
	outputfilename = GA.mcmcoutputfile;
	n = GA.nParam;
	nsamples = GA.nMCMCsamples;
	Params.resize(nsamples);
	pertcoeff.resize(nActParams);
	stucklimit = GA.stucklimit;
	purtscale = GA.purtscale;
	apply_to_all = GA.apply_to_all;
	writeinterval = GA.writeinterval;
	purt_fac = GA.purt_fac;
	sensbasedpurt = GA.sensbasedpurt;
	continue_mcmc = GA.continue_mcmc;
	for (int i=0; i<n; i++)
	{	if (GA.loged[i] == 0)
		{
			MCMCParam[i].type = 1;
			MCMCParam[i].mean = 0.5*(GA.Ind[0].minrange[i]+GA.Ind[0].maxrange[i]);
			MCMCParam[i].std = 0.25*(GA.Ind[0].maxrange[i]-GA.Ind[0].minrange[i]);
			MCMCParam[i].loged = GA.loged[i];
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.Ind[0].minrange[i];
			MCMCParam[i].high = GA.Ind[0].maxrange[i];
		}
		else if (GA.loged[i] == 1)
		{
			MCMCParam[i].type = 2;
			MCMCParam[i].mean = pow(GA.Ind[0].minrange[i]*GA.Ind[0].maxrange[i],0.5);
			MCMCParam[i].std = 0.25*(log(GA.Ind[0].maxrange[i])-log(GA.Ind[0].minrange[i]));
			MCMCParam[i].loged = GA.loged[i];
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = pow(10,GA.Ind[0].minrange[i]);
			MCMCParam[i].high = pow(10,GA.Ind[0].maxrange[i]);
		}
		else if (GA.loged[i] == 2)
		{
			MCMCParam[i].type = 3;
			MCMCParam[i].mean = 0.5*(GA.Ind[0].minrange[i]+GA.Ind[0].maxrange[i]);
			MCMCParam[i].std = 0.25*(GA.Ind[0].maxrange[i]-GA.Ind[0].minrange[i]);
			MCMCParam[i].loged = GA.loged[i];
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.Ind[0].minrange[i];
			MCMCParam[i].high = GA.Ind[0].maxrange[i];
		}
	}


}

CMCMC::CMCMC(const CGA &GA)
{
	logtrans = GA.logtrans;
	fixedstd = GA.fixedstd;
	mixederror = GA.mixederror;
	G = GA.Sys;
	MCMCParam.resize(GA.params.size());
	params = GA.params;
	n_chains = GA.nchains;
	n_burnout = GA.burnout;
	nActParams = GA.nParam;
	outputfilename = GA.mcmcoutputfile;
	n = GA.totnParam;
	nsamples = GA.nMCMCsamples;
	Params.resize(nsamples);
	logp.resize(nsamples);
	logp1.resize(nsamples);
	pertcoeff.resize(nActParams);
	stucklimit = GA.stucklimit;
	purtscale = GA.purtscale;
	apply_to_all = GA.apply_to_all;
	writeinterval = GA.writeinterval;
	purt_fac = GA.purt_fac;
	noinipurt = GA.noinipurt;
	sensbasedpurt = GA.sensbasedpurt;
	global_sensitivity = GA.global_sensitivity;
	continue_mcmc = GA.continue_mcmc;
	for (int i=0; i<nActParams; i++)
	{	if (GA.loged[i] == 0)
		{
			MCMCParam[i].type = 1;
			MCMCParam[i].mean = 0.5*(GA.minval[i]+GA.maxval[i]);
			MCMCParam[i].std = 0.25*(-GA.minval[i]+GA.maxval[i]);
			MCMCParam[i].loged = false;
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.minval[i];
			MCMCParam[i].high = GA.maxval[i];
		}
		else if (GA.loged[i] == 1)
		{
			MCMCParam[i].type = 2;
			MCMCParam[i].mean = pow(GA.minval[i]*GA.maxval[i],0.5);
			MCMCParam[i].std = 0.25*(-log(GA.minval[i])+log(GA.maxval[i]));
			MCMCParam[i].loged = true;
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = pow(10,GA.minval[i]);
			MCMCParam[i].high = pow(10,GA.maxval[i]);
		}
		else  if (GA.loged[i] == 2)
		{	MCMCParam[i].type = 3;
			MCMCParam[i].mean = 0.5*(GA.minval[i]+GA.maxval[i]);
			MCMCParam[i].std = 0.25*(-GA.minval[i]+GA.maxval[i]);
			MCMCParam[i].loged = false;
			MCMCParam[i].param_ID = GA.params[i];
			MCMCParam[i].low = GA.minval[i];
			MCMCParam[i].high = GA.maxval[i];
		}
	}
	paramname = GA.paramname;

}



int CMCMC::getparamno(int i,int ts)
{
	int l=0;	
	for (int j=0; j<i; j++)
		if (apply_to_all[j]) l++ ; else l+=G.Well.size();
	
	if (apply_to_all[i])
		return l;
	else
		return l+ts;

}

int CMCMC::get_act_paramno(int i)
{
	int l=-1;	
	for (int j=0; j<nActParams; j++)
	{
		if (apply_to_all[j]) l++; else l+= G.Well.size();
		if (l>=i) 
		{
			if (apply_to_all[j]) l-= G.Well.size(); else l--;
			return j;
		}
	}
}


int CMCMC::get_time_series(int i)
{
	int l=0;	
	for (int j=0; j<nActParams; j++)
	{
		if (apply_to_all[j]) l+=1; else l+=G.Well.size();
		if (l>=i) 
		{
			if (apply_to_all[j]) l-= G.Well.size(); else l--;
			return i-l;
		}
	}
}

CVector CMCMC::sensitivity(double d, vector<double> par)
{
	
	double base = posterior(par);
	CVector X(n);
	for (int i=0; i<n; i++)
	{	
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
		double base_1 = posterior(par1);

		X[i] = (sqrt(fabs(base))-sqrt(fabs(base_1)))/(d*par[i]);
	}
 	return X;
}

CMatrix CMCMC::sensitivity_mat(double d, vector<double> par)
{
	
	CBTCSet base = model(par);
	CMatrix X(n,base.nvars);
	for (int i=0; i<n; i++)
	{	
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
		CBTCSet base_1 = model(par1);
		for (int j=0; j<base_1.nvars; j++)
			X[i][j] = norm2(base.BTC[j].Abs().Log()>base_1.BTC[j].Abs().Log())/d;
	}
 	return X;
}

CMatrix CMCMC::sensitivity_mat_lumped(double d, vector<double> par)
{
	
	CBTCSet base = model_lumped(par);
	int ii = G.measured_quan.size();
	
	CMatrix X(n,ii);
	for (int i=0; i<n; i++)
	{	
		vector<double> par1 = par;
		par1[i]=par[i]*(1+d);
		CBTCSet base_1 = model_lumped(par1);

		X[i] += norm2dif(base,base_1)/d;
	}
 	return X;
}

int CMCMC::readfromfile(string filename)
{
	for (int j=0; j<nActParams; j++)
	{
		if (MCMCParam[j].type == 1)
		{
			MCMCParam[j].mean = 0.5*(MCMCParam[j].low+MCMCParam[j].high);
			MCMCParam[j].std = (MCMCParam[j].high - MCMCParam[j].mean)/2.0;
			
		}
		if (MCMCParam[j].type == 2)
		{
			MCMCParam[j].mean = sqrt(MCMCParam[j].low*MCMCParam[j].high);
			MCMCParam[j].std = (log(MCMCParam[j].high) - log(MCMCParam[j].mean))/2.0;
		}

	}
	
	ifstream file(filename);
	vector<string> s;
	s = getline(file);
	int jj=0;
	while (file.eof() == false)
	{
		s = getline(file);
		if (s.size() == 2*n+4)
		{	Params[jj].resize(n);
			for (int i=0; i<n; i++)
			{	
				Params[jj][i] = atof(s[i+1].c_str());
				pertcoeff[i] = 	atof(s[n+i+4].c_str());
			}
			logp[jj] = atof(s[n+1].c_str());
			logp1[jj] = atof(s[n+2].c_str());
			jj++;
		}
	}
	file.close();
	return jj;
}

CBTCSet CMCMC::prior_distribution(int n_bins)
{
	CBTCSet A(nActParams);
	CBTC B(n_bins);	
	
	double min_range , max_range;  
		
	for (int i=0; i<nActParams; i++)
	{		
		if (MCMCParam[i].type == 1)
		{
			MCMCParam[i].mean = 0.5*(MCMCParam[i].low+MCMCParam[i].high);
			MCMCParam[i].std = (MCMCParam[i].high - MCMCParam[i].mean)/2.0;	
			min_range = MCMCParam[i].mean - 4*MCMCParam[i].std;  
			max_range = MCMCParam[i].mean + 4*MCMCParam[i].std;  
		}
		if (MCMCParam[i].type == 2)
		{
			MCMCParam[i].mean = sqrt(MCMCParam[i].low*MCMCParam[i].high);
			MCMCParam[i].std = (log(MCMCParam[i].high) - log(MCMCParam[i].mean))/2.0;	
			min_range = MCMCParam[i].mean * exp(-4*MCMCParam[i].std);  
			max_range = MCMCParam[i].mean * exp(4*MCMCParam[i].std);   
		}	
		if (MCMCParam[i].type == 3)
		{
			MCMCParam[i].mean = 0.5*(MCMCParam[i].low+MCMCParam[i].high);
			MCMCParam[i].std = (MCMCParam[i].high - MCMCParam[i].mean)/2.0;	
			min_range = MCMCParam[i].mean - 4*MCMCParam[i].std;  
			max_range = MCMCParam[i].mean + 4*MCMCParam[i].std;   
		}	

		
		double dp = abs(max_range - min_range) / n_bins;

		B.t[0] = min_range + dp/2;		
		for (int j=0; j<n_bins-1; j++)
			B.t[j+1] = B.t[j] + dp;
		
		if (MCMCParam[i].type == 1)
			for (int j=0; j<n_bins; j++)
				B.C[j] = exp(-pow(B.t[j]-MCMCParam[i].mean,2)/(2.0*pow(MCMCParam[i].std,2)))/(MCMCParam[i].std*pow(6.28,0.5));

		if (MCMCParam[i].type == 2)
			for (int j=0; j<n_bins; j++)
				B.C[j] = exp(-pow(log(B.t[j])-log(MCMCParam[i].mean),2)/(2.0*pow(MCMCParam[i].std,2)))/(B.t[j]*MCMCParam[i].std*pow(6.28,0.5));

		if (MCMCParam[i].type == 3)
			for (int j=0; j<n_bins; j++)
				if ((B.t[j]<MCMCParam[i].high) && (B.t[j]>MCMCParam[i].low)) B.C[j] = 1/(MCMCParam[i].high-MCMCParam[i].low); else B.C[j]=0;

		A.BTC[i] = B;
	}

	return A;
}
