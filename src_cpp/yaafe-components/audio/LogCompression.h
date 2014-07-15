/*
 * LogCompression.h
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#ifndef LOGCOMPRESSION_H_
#define LOGCOMPRESSION_H_

#include "yaafe-core/ComponentHelpers.h"

#define LOGCOMPRESSION_ID "LogCompression"

namespace YAAFE_EXT {

class LogCompression: public YAAFE::StateLessOneInOneOutComponent<LogCompression> {
public:
	LogCompression();
	virtual ~LogCompression();

    virtual const std::string getIdentifier() const { return LOGCOMPRESSION_ID;};

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

};

}

#endif /* LOGCOMPRESSION_H_ */
