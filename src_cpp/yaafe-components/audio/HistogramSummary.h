/*
 * HistogramSummaryIntegrator.h
 *
 *  Created on: Sep 1, 2009
 *      Author: bmathieu
 */

#ifndef HISTOGRAMSUMMARYINTEGRATOR_H_
#define HISTOGRAMSUMMARYINTEGRATOR_H_

#include "yaafe-core/ComponentHelpers.h"

#define HISTOGRAMSUMMARY_ID "HistogramSummary"

namespace YAAFE
{

class HistogramSummary: public YAAFE::StateLessOneInOneOutComponent<HistogramSummary>
{
public:
    HistogramSummary();
    virtual ~HistogramSummary();

    virtual const std::string getIdentifier() const { return HISTOGRAMSUMMARY_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

	YAAFE::StreamInfo init(const YAAFE::ParameterMap& params,const YAAFE::StreamInfo& in);
	void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    double m_hinf;
    double m_hstep;
};

}

#endif /* HISTOGRAMSUMMARYINTEGRATOR_H_ */
