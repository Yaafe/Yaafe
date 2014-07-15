/*
 * CQT.h
 *
 *  Created on: May 11, 2009
 *      Author: bmathieu
 */

#ifndef CQT_H_
#define CQT_H_

#include "yaafe-core/ComponentHelpers.h"
#include <Eigen/Dense>

#define CQT_ID "CQT"

namespace YAAFE_EXT
{

class CQT: public YAAFE::StateLessOneInOneOutComponent<CQT>
{
public:
    CQT();
    virtual ~CQT();

    virtual const std::string getIdentifier() const { return CQT_ID;};

    virtual YAAFE::ParameterDescriptorList getParameterDescriptorList() const;

    virtual YAAFE::StreamInfo init(const YAAFE::ParameterMap& params, const YAAFE::StreamInfo& in);
    virtual void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    int m_size;
    int m_fftLen;

    std::vector<Eigen::ArrayXcd> m_kernPosFilters;
    std::vector<Eigen::ArrayXcd> m_kernNegFilters;
    std::vector<int> m_kernPosStart;
    std::vector<int> m_kernNegStart;

};

}

#endif /* CQT_H_ */
