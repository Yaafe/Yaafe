
#ifndef DECIMATE2_H
#define DECIMATE2_H

#include "yaafe-core/ComponentHelpers.h"

#define DECIMATE2_ID "Decimate2"

namespace YAAFE_EXT
{

class Decimate2: public YAAFE::ComponentBase<Decimate2>
{
public:
    Decimate2();
    virtual ~Decimate2();

    virtual const std::string getIdentifier() const { return DECIMATE2_ID;};
    virtual bool stateLess() const { return false; };

	virtual bool init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& in);
	virtual void reset();
	virtual bool process(YAAFE::Ports<YAAFE::InputBuffer*>& in, YAAFE::Ports<YAAFE::OutputBuffer*>& out);
	virtual void flush(YAAFE::Ports<YAAFE::InputBuffer*>& in, YAAFE::Ports<YAAFE::OutputBuffer*>& out);

private:
	double* m_state;
	int m_pos;

	static double* s_filter;
	static void initFilter();
};

} // YAAFE_EXT

#endif
