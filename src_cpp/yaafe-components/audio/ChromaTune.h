/*
 * ChromaBP.h
 *
 *  Created on: Dec 11, 2009
 *      Author: bmathieu
 */

#ifndef CHROMABP_H_
#define CHROMABP_H_

#include "yaafe-core/Component.h"

#define CHROMATUNE_ID "ChromaTune"

namespace YAAFE_EXT {

class ChromaTune: public YAAFE::ComponentBase<ChromaTune> {
public:
	ChromaTune();
	virtual ~ChromaTune();

    virtual const std::string getIdentifier() const { return CHROMATUNE_ID;};
	virtual bool stateLess() const { return false; }

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual bool init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp);
    virtual void reset();
	virtual bool process(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp);
	virtual void flush(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp);


private:
    double m_fmin;
    int m_binsPerOctave;

    int m_initLength;
    int m_binshift;

    void initshift(YAAFE::InputBuffer* in,int size, int nbFrames);

};

}

#endif /* CHROMABP_H_ */
