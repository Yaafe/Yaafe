/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
 *
 * This file is part of Yaafe.
 *
 * Yaafe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Yaafe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTRALCRESTFACTORPERBAND_H_
#define SPECTRALCRESTFACTORPERBAND_H_

#include "yaafe-core/Component.h"
#include <vector>

#define SPECTRALCRESTFACTORPERBAND_ID "SpectralCrestFactorPerBand"

namespace YAAFE {

class SpectralCrestFactorPerBand: public YAAFE::ComponentBase<SpectralCrestFactorPerBand> {
public:
    SpectralCrestFactorPerBand();
    virtual ~SpectralCrestFactorPerBand();

    virtual const std::string getIdentifier() const { return SPECTRALCRESTFACTORPERBAND_ID;};
    virtual const std::string getDescription() const { return "Crest factor per log-spaced band of 1/4 octave."; }

    virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
    virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);

private:
    int m_inSize;
    struct bandinfo_ {
    	int start;
    	int end;
    	int group;
    	int length() { return end-start; }
    };
    typedef struct bandinfo_ bandinfo;
    std::vector<bandinfo> m_band; // start,end,group for each band
};

}

#endif /* SPECTRALCRESTFACTOR_H_ */
