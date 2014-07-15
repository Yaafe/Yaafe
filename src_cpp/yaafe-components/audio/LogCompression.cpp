/*
 * LogCompression.cpp
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#include "LogCompression.h"
#include "MathUtils.h"

using namespace YAAFE;
using namespace std;

namespace YAAFE_EXT {

LogCompression::LogCompression() {
}

LogCompression::~LogCompression() {
}

StreamInfo LogCompression::init(const ParameterMap& params, const StreamInfo& in)
{
	return in;
}



void LogCompression::processToken(double* inData, const int inSize, double* outData, const int outSize)
{
	assert(inSize==outSize);
	for (int i=0;i<inSize;i++)
		outData[i] = 20 * log10(abs(inData[i]));
}

}
