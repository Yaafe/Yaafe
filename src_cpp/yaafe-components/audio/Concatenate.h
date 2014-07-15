
#ifndef CONCATENATE_H
#define CONCATENATE_H

#include "yaafe-core/Component.h"

#define CONCATENATE_ID "Concatenate"

namespace YAAFE
{

class Concatenate: public YAAFE::ComponentBase<Concatenate>
{
public:
	Concatenate();
	virtual ~Concatenate();

    virtual const std::string getIdentifier() const { return CONCATENATE_ID;};
    virtual bool stateLess() const { return false; };

	virtual bool init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& in);
	virtual bool process(YAAFE::Ports<YAAFE::InputBuffer*>& in, YAAFE::Ports<YAAFE::OutputBuffer*>& out);
private:
	int m_portSize;
	int* m_portOrder;
};

} // YAAFE

#endif
