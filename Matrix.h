// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__0D12870C_417A_469B_9809_1EA45078DD30__INCLUDED_)
#define AFX_MATRIX_H__0D12870C_417A_469B_9809_1EA45078DD30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Vector.h"
#include <iostream>
#include "math.h"
#include "armadillo"

using namespace arma;
class CVector;
class CMatrix  
{
friend class D5Matrix;
private:
	int numrows;
	int numcols;
	vector<CVector> matr;
	int range(int);
public:
	CMatrix(int, int);
	CMatrix(int);
	CMatrix();
	CMatrix(const CMatrix&);
	CMatrix(const CVector&);
	CVector& operator[](int);
	int CMatrix::getnumrows();
	int CMatrix::getnumcols();
	virtual ~CMatrix();
	CMatrix& CMatrix::operator=(const CMatrix&);
	CMatrix& CMatrix::operator+=(const CMatrix&);
	CMatrix& CMatrix::operator-=(const CMatrix &);	
	CMatrix& CMatrix::operator=(mat);
	//mat CMatrix::operator>=(const CMatrix &);
	friend CMatrix mult(CMatrix&, CMatrix&);
	friend CVector mult(CMatrix&, CVector&);
	friend CVector mult(CVector&, CMatrix&);
	friend void triangulate(CMatrix&, CVector&);
	friend void backsubst(CMatrix&, CVector&, CVector&);
	friend CVector gauss0(CMatrix, CVector);
	friend CVector diag(CMatrix);
	friend CMatrix Cholesky_factor(CMatrix &M);
	friend CMatrix LU_decomposition(CMatrix &M);
	CMatrix CMatrix::LU_decomposition();
	CMatrix CMatrix::Cholesky_factor();	
	double CMatrix::det();
	void CMatrix::Print(FILE *FIL);
	void CMatrix::print(string s);
	void CMatrix::setval(double a);
	void CMatrix::setvaldiag(double a);
	void CMatrix::writetofile(FILE *f);
	void CMatrix::writetofile(string filename);
	friend void write_to_file(vector<CMatrix> M, string filename);
	friend CMatrix Average(vector<CMatrix> M);
};
	
double det(CMatrix &);
CMatrix Log(CMatrix &M1);
CMatrix Exp(CMatrix &M1);
CMatrix Sqrt(CMatrix &M1);
CMatrix operator+(CMatrix, CMatrix);
CMatrix operator+(double, CMatrix);
CMatrix operator+(CMatrix, double);
CMatrix operator-(double d, CMatrix m1);
CMatrix operator+(CMatrix m1, double d);
CMatrix operator-(CMatrix m1,double d);
CMatrix operator/(CMatrix m1,double d);
CMatrix operator/(double d, CMatrix m1);
CMatrix operator-(CMatrix, CMatrix);
CMatrix operator*(CMatrix, CMatrix);
CVector operator*(CMatrix, CVector);
CMatrix operator*(CVector, CMatrix);
CMatrix operator*(double, CMatrix);
CVector operator/(CVector, CMatrix);
CMatrix Transpose(CMatrix &M1);
CMatrix Invert(CMatrix M1);
CVector SpareSolve(CMatrix, CVector);
CMatrix oneoneprod(CMatrix &m1, CMatrix &m2);
CVector solve_ar(CMatrix, CVector);
CMatrix inv(CMatrix);


#endif // !defined(AFX_MATRIX_H__0D12870C_417A_469B_9809_1EA45078DD30__INCLUDED_)
