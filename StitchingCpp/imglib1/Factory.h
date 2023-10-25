#pragma once
#include "header.h"

class Factory {
public:
	virtual void printProperties() = 0;
	virtual string getErrorMessage() = 0;
	virtual void setParameters(string configuration) = 0;
};