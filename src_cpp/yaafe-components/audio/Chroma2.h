/*
 * ChromaZhu.h
 *
 *  Created on: May 12, 2009
 *      Author: bmathieu
 */

#ifndef CHROMA2_H_
#define CHROMA2_H_

#include "yaafe-core/ComponentHelpers.h"
#include <Eigen/Dense>

#define CHROMA2_ID "Chroma2"

namespace YAAFE_EXT
{

class Chroma2: public YAAFE::StateLessOneInOneOutComponent<Chroma2>
{
public:
    Chroma2();
    virtual ~Chroma2();

    virtual const std::string getIdentifier() const { return CHROMA2_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    double m_cqtMinFreq;
    double m_cqtMaxFreq;
    int m_cqtNbBins;
    int m_cqtSize;
    int m_nbBinsSemitone;
    int m_nbBins;
    double m_tuning;

    double m_deviation;
    int m_tuningBin;
    double m_Q;
    double m_K;
    int m_nbNote;
    Eigen::VectorXd m_notePartial;
    int m_notePartialA;
    int m_notePartialFactor;
    int m_pcpSize;
    Eigen::VectorXd m_pcp;

    int m_pcpShift;
};

}

#endif /* CHROMA3_H_ */
