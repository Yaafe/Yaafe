/*
 * Chroma2Chords.h
 *
 *  Created on: Dec 2, 2009
 *      Author: bmathieu
 */

#ifndef CHROMA2CHORDS_H_
#define CHROMA2CHORDS_H_

#include "yaafe-core/ComponentHelpers.h"

#define CHROMA2CHORDS_ID "Chroma2ChordDict"

namespace YAAFE {

class Chroma2ChordDict: public YAAFE::StateLessOneInOneOutComponent<Chroma2ChordDict> {
public:
	Chroma2ChordDict();
	virtual ~Chroma2ChordDict();

    virtual const std::string getIdentifier() const { return CHROMA2CHORDS_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    std::string m_chordTypes;
    int m_nbHarmnonics;

    class Dict;
    Dict* m_dict;
};

}

#endif /* CHROMA2CHORDS_H_ */
