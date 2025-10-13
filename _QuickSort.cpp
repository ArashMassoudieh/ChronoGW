#include "QuickSort.h"
#include <iostream>
#include <algorithm>	// std::swap()
#include <vector>

using namespace std;

CQuickSort::CQuickSort(void)
{
}


CQuickSort::~CQuickSort(void)
{

}

vector<double> QSort(vector<double> V)
{
	if (V.size()<=1) return V;
	int end = V.size();
	vector<double> less, greater;
	greater.push_back(V[end-1]);
	for (int i=0; i<end-1; i++)
		if (V[i]<V[end-1]) less.push_back(V[i]);
		else greater.push_back(V[i]);
		
	
	vector<double> res = QSort(less);
	if ((V==greater) && (less.size()==0)) return greater;
	vector<double> x2 = QSort(greater);
	
	res.insert(res.end(), x2.begin(), x2.end());
	less.clear();
	greater.clear();
	x2.clear();
	return res;

}

vector<int> QSort(vector<int> V)
{
	if (V.size()<=1) return V;
	int end = V.size();
	vector<int> less, greater;
	greater.push_back(V[end-1]);
	for (int i=0; i<end-1; i++)
		if (V[i]<V[end-1]) less.push_back(V[i]);
		else greater.push_back(V[i]);
		
	
	vector<int> res = QSort(less);
	if ((V==greater) && (less.size()==0)) return greater;
	vector<int> x2 = QSort(greater);
	
	res.insert(res.end(), x2.begin(), x2.end());
	less.clear();
	greater.clear();
	x2.clear();
	return res;

}









