#pragma once

#include "..\Matrix.h"

class QbFormat
{
private:
	static Sint32 strToNum(std::string str);
public:
	static bool load(std::string p_fileName, std::vector<Matrix*>& p_matrixList);
};
