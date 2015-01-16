/*
 * Chroma2Chords.cpp
 *
 *  Created on: Dec 2, 2009
 *      Author: bmathieu
 */

#include "Chroma2ChordDict.h"
#include "ChordTemplate.h"
#include <Eigen/Dense>

#define EPS 2.220446049250313e-16

using namespace std;
using namespace Eigen;

namespace YAAFE {

class Chroma2ChordDict::Dict {
public:
	Dict();
	~Dict();


	bool populate(const string& chordTypes, int nbHarmo);
	void fillPattern(const vector<int>& notes,const int nbHarmo, double* pattern);

	inline int size() { return m_nbTypes*12; };
	inline double* pattern(int i) { return m_patterns.data() + ((i/12)*24 + 12 - i%12); }

private:
	ArrayXd m_patterns;
	int m_nbTypes;
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

Chroma2ChordDict::Dict::Dict() : m_nbTypes(0), m_patterns()
		{}

Chroma2ChordDict::Dict::~Dict() {
//	if (m_patterns)
//		delete [] m_patterns;
}

bool Chroma2ChordDict::Dict::populate(const string& chordTypes, int nbHarmo)
{
	// gather templates
	vector<string> templates;
	string str=chordTypes;
	while (str.size()>0)
	{
		size_t idx = str.find(',');
		if (idx==string::npos)
			idx = str.size();
		templates.push_back(str.substr(0,idx));
		if (idx==str.size())
			break;
		str = str.substr(idx+1);
	}

	// build patterns
	m_nbTypes = templates.size();
//	m_patterns = new double[24*m_nbTypes];
	m_patterns.resize(24*m_nbTypes);
	for (int i=0;i<m_nbTypes;i++)
	{
		const ChordTemplate& cht = ChordTemplate::getChordTemplate(templates[i]);
		if (cht.notes().size()==0)
		{
			printf("ERROR: unknown chord %s\n",templates[i].c_str());
			return false;
		}
		fillPattern(cht.notes(),nbHarmo,&m_patterns[24*i]);
	}

	return true;
}

void Chroma2ChordDict::Dict::fillPattern(const vector<int>& notes,const int nbHarmo, double* pattern)
{
	int nH = nbHarmo;
	if (nH>6)
	{
		printf("WARNING: cannot use more than 6 harmonics (%i asked). Use only 6 harmonics.\n",nH);
		nH = 6;
	}
	static int harmonic_chroma[] = {0, 0, 7, 0, 4, 7};
	static double harmonic_weight[] = {1.0, 0.6, 0.36, 0.216, 0.1206, 0.07776};

	Map<ArrayXd> pVec(pattern,12);

	// init pattern to zeros
	pVec.setZero();
//	for (int i=0;i<12;i++)
//		pVec(i) = 0.0;

	// compute pattern
	for (unsigned int n=0;n<notes.size();n++)
	{
		for (int h = 0;h<nH;h++)
		{
			int chroma = (notes[n]+harmonic_chroma[h]) % 12;
			pVec(chroma) += harmonic_weight[h];
		}
	}

	// normalize
	pVec = (pVec + EPS) / pVec.sum();

	// duplicate pattern
	Map<ArrayXd> dupVec(pattern+12,12);
	dupVec = pVec;
}

Chroma2ChordDict::Chroma2ChordDict() :
		m_chordTypes(), m_nbHarmnonics(0), m_dict(NULL)
{
}

Chroma2ChordDict::~Chroma2ChordDict() {
	if (m_dict)
		delete m_dict;
}

ParameterDescriptorList Chroma2ChordDict::getParameterDescriptorList() const
{
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier="ChordTypes";
	p.m_description="List of chord types to consider";
	p.m_defaultValue = "maj,min";
	pList.push_back(p);

	p.m_identifier="ChordNbHarmonics";
	p.m_description="Number of harmonics to consider for each chord's note.";
	p.m_defaultValue="1";
	pList.push_back(p);

	return pList;
}

StreamInfo Chroma2ChordDict::init(const ParameterMap& params, const StreamInfo& in) {
	assert(in.size==12);

	string types = getStringParam("ChordTypes",params);
	int nbHarmo = getIntParam("ChordNbHarmonics",params);

	m_chordTypes = types;
	m_nbHarmnonics = nbHarmo;

	m_dict = new Dict();
	if (!m_dict->populate(m_chordTypes,m_nbHarmnonics))
	{
		delete m_dict;
		m_dict = NULL;
		// return false
	}
	return StreamInfo(in, m_dict->size());
}

void Chroma2ChordDict::processToken(double* inPtr, const int inSize, double* outData, const int outSize)
{
	assert(inSize==12);
	assert(outSize==m_dict->size());
	Map<ArrayXd> inData(inPtr,12);
	ArrayXd chroma(12);
	chroma = inData / inData.sum();
	for (int i=0;i<m_dict->size();i++)
	{
		Map<ArrayXd> pattern(m_dict->pattern(i),12);
		outData[i] = (pattern*(pattern/chroma).log() - pattern + chroma).sum();
	}
}

}
