/*
 * ChordDictDecoder.cpp
 *
 *  Created on: Dec 8, 2009
 *      Author: bmathieu
 */

#include "ChordDictDecoder.h"
#include <iostream>

using namespace std;
using namespace YAAFE;

namespace YAAFE_EXT {

ChordDictDecoder::ChordDictDecoder() :
	m_chordTypes(), m_chordTemplates() {

}

ChordDictDecoder::~ChordDictDecoder() {
}

ParameterDescriptorList ChordDictDecoder::getParameterDescriptorList() const
{
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier="ChordTypes";
	p.m_description="List of chord types to consider";
	p.m_defaultValue = "maj,min";
	pList.push_back(p);

	return pList;
}

StreamInfo ChordDictDecoder::init(const ParameterMap& params, const StreamInfo& in)
{
	string types = getStringParam("ChordTypes",params);
	m_chordTypes = types;
	while (types.size()>0)
	{
		size_t idx = types.find(',');
		if (idx==string::npos)
			idx = types.size();
		const ChordTemplate& cht = ChordTemplate::getChordTemplate(types.substr(0,idx));
		if (cht.notes().size()==0)
		{
			cerr << "ERROR: unknown chord " << types.substr(0,idx) << endl;
		} else {
			m_chordTemplates.push_back(cht);
		}
		if (idx==types.size())
			break;
		types = types.substr(idx+1);
	}

	assert(in.size==12*m_chordTemplates.size());

	return StreamInfo(in, 1);
}

void ChordDictDecoder::processToken(double* inData, const int inSize, double* out, const int outSize)
{
	int chordIndex = 0;
	for (int i=1;i<inSize;i++)
		if (inData[i]<inData[chordIndex])
			chordIndex = i;
	*out = m_chordTemplates[chordIndex/12].quality() * 12 + chordIndex%12;
}

}
