#pragma once
#include <vector>
#include <iostream>
#include "string.h"
#include "StringOP.h"

using namespace std;

class CLIDconfig
{
public:
	CLIDconfig(void);
	~CLIDconfig(void);
	CLIDconfig& CLIDconfig::operator = (const CLIDconfig &CC);
	CLIDconfig(const CLIDconfig &CC);
	vector<string> keyword;
	vector<string> value;
	vector<vector<string>> param_names;
	vector<vector<string>> param_vals;
	vector<vector<string>> est_param; // -1:fixed >=0: parameter number
	vector<string> params_string;
	
};

