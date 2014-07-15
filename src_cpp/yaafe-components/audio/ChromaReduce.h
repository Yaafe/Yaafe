/*
 * ChromaReduce.h
 *
 *  Created on: Dec 14, 2009
 *      Author: bmathieu
 */

#ifndef CHROMAREDUCE_H_
#define CHROMAREDUCE_H_

#include "yaafe-core/ComponentHelpers.h"
#include <Eigen/Dense>

#define CHROMAREDUCE_ID "ChromaReduce"

namespace YAAFE {

class ChromaReduce: public YAAFE::StateLessOneInOneOutComponent<ChromaReduce> {
public:
	ChromaReduce();
	virtual ~ChromaReduce();

    virtual const std::string getIdentifier() const { return CHROMAREDUCE_ID;};

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    int m_shift;
    Eigen::RowVectorXd m_gausswin;
};

}

#endif /* CHROMAREDUCE_H_ */
