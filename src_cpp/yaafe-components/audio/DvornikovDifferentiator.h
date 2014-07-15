/*
 * DvornikovDifferentiator.h
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#ifndef DVORNIKOVDIFFERENTIATOR_H_
#define DVORNIKOVDIFFERENTIATOR_H_

#include "yaafe-core/ComponentHelpers.h"

#define DVORNIKOVDIFFERENTIATOR_ID "DvornikovDifferentiator"

namespace YAAFE_EXT {

class DvornikovDifferentiator: public YAAFE::TemporalFilter<DvornikovDifferentiator> {
public:
	DvornikovDifferentiator();
	virtual ~DvornikovDifferentiator();

	virtual const std::string getIdentifier() const { return DVORNIKOVDIFFERENTIATOR_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual bool initFilter(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
};

}

#endif /* DVORNIKOVDIFFERENTIATOR_H_ */
