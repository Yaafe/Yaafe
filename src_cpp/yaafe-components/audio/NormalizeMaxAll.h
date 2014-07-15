/*
 * NormalizeMaxAll.h
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#ifndef NORMALIZEMAXALL_H_
#define NORMALIZEMAXALL_H_

#include "yaafe-core/Component.h"

#define NORMALIZEMAXALL_ID "NormalizeMaxAll"

namespace YAAFE_EXT {

class NormalizeMaxAll: public YAAFE::ComponentBase<NormalizeMaxAll> {
public:
	NormalizeMaxAll();
	virtual ~NormalizeMaxAll();

    virtual const std::string getIdentifier() const { return NORMALIZEMAXALL_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual bool init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp);
	virtual bool process(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp);
	virtual void flush(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp);

private:
    int m_nbFrames;

    void processBlock(YAAFE::InputBuffer* in, YAAFE::OutputBuffer* out, int nbFrames);

};

}

#endif /* NORMALIZEMAXALL_H_ */
