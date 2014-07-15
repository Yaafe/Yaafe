/*
 * ChordDictDecoder.h
 *
 *  Created on: Dec 8, 2009
 *      Author: bmathieu
 */

#ifndef CHORDDICTDECODER_H_
#define CHORDDICTDECODER_H_

#include "yaafe-core/ComponentHelpers.h"
#include "ChordTemplate.h"

#define CHORDDICT_DECODER_ID "ChordDictDecoder"

namespace YAAFE {

class ChordDictDecoder: public YAAFE::StateLessOneInOneOutComponent<ChordDictDecoder> {
public:
	ChordDictDecoder();
	virtual ~ChordDictDecoder();

    virtual const std::string getIdentifier() const { return CHORDDICT_DECODER_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    std::string m_chordTypes;
    std::vector<ChordTemplate> m_chordTemplates;

};

}

#endif /* CHORDDICTDECODER_H_ */
