#pragma once
#include <vector>
#include <string>

using namespace std;

class CLIDconfig
{
public:
	CLIDconfig(void);
	~CLIDconfig(void);
    CLIDconfig& operator = (const CLIDconfig &CC);
	CLIDconfig(const CLIDconfig &CC);
    std::vector<std::string> keyword;
    std::vector<std::string> value;
    std::vector<std::vector<std::string>> param_names;
    std::vector<std::vector<std::string>> param_vals;
    std::vector<std::vector<std::string>> est_param; // -1:fixed >=0: parameter number
    std::vector<std::string> params_string;
	
};

