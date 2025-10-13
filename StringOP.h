#pragma once

#include <vector>
#include <string>

using namespace std;

class CStringOP
{
public:
	CStringOP(void);
	CStringOP(string S);
	~CStringOP(void);
    CStringOP& operator = (const CStringOP &P);
	CStringOP(const CStringOP &P);
	vector<CStringOP> terms;
	int nterms;
	int nopts;
	vector<int> operators;
    vector<string> breakparents(string S);
    vector<int> breakops(string S);
	bool physical_ch;
	bool constant;
	bool parameter;
	bool s_concentration;
	bool concentration;
	bool function;
	bool s_block, t_block, a_block;
	double value;
	int number;
	int phase;
};

int opertr(char a);
bool isnumber(char S);
vector<string> getline(ifstream&); 
vector<vector<string>> getline_op(ifstream&,char del1);
vector<vector<string>> getline_op(ifstream&,vector<char> del1);
vector<string> split(string s, char del); 
vector<string> split(string s, vector<char> del);
vector<string> split_curly_semicolon(string s);
vector<vector<string>> getline_op_eqplus(ifstream&);
string trim(string s);
vector<int> ATOI(vector<string> ii);
vector<double> ATOF(vector<string> ii);
int getoperator(string S);
string tolower(string S);
void writeline(ofstream&, vector<string>, string del);
void writeline(ofstream&, vector<vector<string>>, string del1, string del2);
void writestring(ofstream&, string s);
void writenumber(ofstream&, double s);
void writeendl(ofstream&);
double Heavyside(double x);
double pipe_poly(double x);
vector<int> look_up(string s, char del); 
string numbertostring(double x);
string numbertostring(int x);
//int min(int x, int y) { return (x < y) ? x : y; };
//int max(int x, int y) { return (x < y) ? y : x; };

