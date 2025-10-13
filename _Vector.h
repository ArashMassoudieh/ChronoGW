
#pragma once

#include <iostream>
#include <vector>
#include "QuickSort.h"
#include "armadillo"

using namespace arma;
using namespace std;

class CMatrix;
class SizeDist;
class CVector  
{
private:
	
	
public:
	vector<double> vec;
	CVector();
	CVector(int);
	CVector(const vector<double>, int);
	CVector(const vector<double> &v);
	CVector(const vector<int> &v);
	CVector(const double x, int n);
	CVector::CVector(const double x_min, const double x_max, int n);
	CVector(const CVector&);
	double& CVector::operator[](int);
	virtual ~CVector();
	int num;
	int CVector::range(int);
	CVector& CVector::operator=(const CVector&);
	CVector& CVector::operator=(const vector<double>&);
	CVector& CVector::operator=(const double &v);
	CVector CVector::operator=(mat);
	//mat CVector::operator=(const CVector&);
	CVector& CVector::operator+();
	void CVector::swap(int , int );
	int CVector::getsize();
	CVector& CVector::operator*=(double);
	CVector& CVector::operator/=(double);
	CVector& CVector::operator+=(const CVector&);
	CVector& CVector::operator-=(const CVector&);
	CVector& CVector::operator*=(const CVector&);
	friend double dotproduct(CVector, CVector);
	friend CVector mult(CMatrix&, CVector&);	
	friend double norm(CVector);
	friend double dotproduct(CVector v1, CVector v2);
	bool CVector::operator==(double v);
	double CVector::max();
	double CVector::min();
	double CVector::norm2();
	double CVector::sum();
	double CVector::abs_max();
	CMatrix CVector::T();
	CVector CVector::Log();
	CVector CVector::abs();
	CVector CVector::H();
	void CVector::writetofile(FILE *f);
	void CVector::writetofile(string filename);
	CVector CVector::Exp();
	vector<int> CVector::Int();
	CMatrix CVector::diagmat();
	CVector CVector::append(const CVector& V1);
	CVector CVector::append(double d);
	CVector CVector::sort();
	vector<int> CVector::lookup(double val);
	void CVector::print(string s);

};

CVector Log(CVector &);
CVector Exp(CVector &);
CVector abs(CVector &);
double abs_max(CVector &);
double min(CVector &);
double max(CVector &);
CVector H(CVector &);
double H(double x);
CVector operator+(CVector, CVector);
CVector operator+(double, CVector);
CVector operator+(CVector, double);
CVector operator-(CVector, CVector);
CVector operator-(double, CVector&);
CVector operator*(CVector, CVector);
CVector operator*(double, CVector);
CVector operator/(CVector, double); 
CVector operator/(CVector, CVector);
CVector operator/(double, CVector);
CVector zeros(int i);
CVector combinesort(const CVector& V1, const CVector &V2);
CVector combinesort_s(const CVector& V1, const CVector &V2);
int lookup(vector<int> v, int val);
int lookup(vector<double> v, double val);
int lookup(vector<string> v, string val);
double avg(CVector &);
