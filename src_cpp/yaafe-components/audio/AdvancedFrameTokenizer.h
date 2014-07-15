
#ifndef ADVANCEDFRAMETOKENIZER_H
#define ADVANCEDFRAMETOKENIZER_H

#include "yaafe-core/Component.h"

#define ADVANCEDFRAMETOKENIZER_ID "AdvancedFrameTokenizer"

namespace YAAFE
{

class AdvancedFrameTokenizer : public YAAFE::ComponentBase<AdvancedFrameTokenizer>
{
public:
	AdvancedFrameTokenizer();
	virtual ~AdvancedFrameTokenizer();

    virtual const std::string getIdentifier() const { return ADVANCEDFRAMETOKENIZER_ID;};
    virtual bool stateLess() const { return true; };

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

	virtual bool init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& in);
	virtual bool process(YAAFE::Ports<YAAFE::InputBuffer*>& in, YAAFE::Ports<YAAFE::OutputBuffer*>& out);
	virtual void flush(YAAFE::Ports<YAAFE::InputBuffer*>& in, YAAFE::Ports<YAAFE::OutputBuffer*>& out);

private:
	int m_blockSize;
	int m_outStepSize;
	double m_outSampleRate;
};

} // namespace YAAFE

#endif // ADVANCEDFRAMETOKENIZER_H
