#include "stdafx.h"
#include "LIDconfig.h"


CLIDconfig::CLIDconfig(void)
{
}


CLIDconfig::~CLIDconfig(void)
{
}

CLIDconfig& CLIDconfig::operator = (const CLIDconfig &CC)
{
	keyword = CC.keyword;
	value = CC.value;
	param_names = CC.param_names;
	param_vals = CC.param_vals;
	return *this;
}


CLIDconfig::CLIDconfig(const CLIDconfig &CC)
{
	keyword = CC.keyword;
	value = CC.value;
	param_names = CC.param_names;
	param_vals = CC.param_vals;
	
}
