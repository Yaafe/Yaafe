
#include "Concatenate.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include "math.h"
#include "string.h"

using namespace std;
using namespace YAAFE;

namespace YAAFE_EXT {

Concatenate::Concatenate() : m_portOrder(NULL) {};

Concatenate::~Concatenate() {
	if (m_portOrder) delete [] m_portOrder;
};

bool Concatenate::init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp)
{
	// determine concatenation order (based on port name)
	vector<string> portNames;
	for (int i=0;i<inp.size();i++)
	{
		portNames.push_back(inp[i].name);
	}
	sort(portNames.begin(),portNames.end());
	m_portSize = inp.size();
	m_portOrder = new int[m_portSize];
	for (int i=0;i<m_portSize;i++)
		for (int j=0;j<m_portSize;j++)
			if (portNames[i]==inp[j].name)
				m_portOrder[i] = j;

	// check all inputs have same rates
	StreamInfo si = inp[m_portOrder[0]].data;
	for (int i=1;i<inp.size();i++) {
		if (fabs(si.sampleRate/si.sampleStep - inp[m_portOrder[i]].data.sampleRate/inp[m_portOrder[i]].data.sampleStep)>0.000001) {
			cerr << "ERROR: try to concatenate stream with different sample rates !" << endl;
			return false;
		}
		si.size += inp[m_portOrder[i]].data.size;
	}

	outStreamInfo().add(si);
	return true;
}

bool Concatenate::process(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp)
{
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;
#ifdef DEBUG
	// check input and output sizes
	int s = 0;
	for (int i=0;i<inp.size();i++)
		s += inp[i].data->info().size;
	assert(s == out->info().size);
#endif


	// check if some data is available
	for (int i=0;i<inp.size();i++)
	{
		if (inp[i].data->empty())
			return false;
	}

	InputBuffer* inputs[m_portSize];
	for (int i=0;i<m_portSize;i++)
		inputs[i] = inp[m_portOrder[i]].data;

	// concatenate streams
	bool hasNextToken = true;
	while (hasNextToken) {
		double* outTok = out->writeToken();
		for (int i=0;i<m_portSize;i++) {
			InputBuffer* in = inputs[i];
			memcpy(outTok,in->readToken(),in->info().size*sizeof(double));
			in->consumeToken();
			outTok += in->info().size;
			if (in->empty()) hasNextToken = false;
		}
	}

	return true;
}

} // YAAFE_EXT
