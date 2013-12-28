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

#ifndef MP3FILEREADER_H_
#define MP3FILEREADER_H_

#include "yaafe-core/Component.h"

#define MP3_FILE_READER_ID "MP3FileReader"

namespace YAAFE {

class MP3FileReader: public ComponentBase<MP3FileReader> {
public:
	MP3FileReader();
	virtual ~MP3FileReader();

    virtual const std::string getIdentifier() const { return MP3_FILE_READER_ID;};
    virtual bool stateLess() const { return false; };

    virtual ParameterDescriptorList getParameterDescriptorList() const;

    virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
    virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);
    virtual void flush(InputBuffer* in, OutputBuffer* out);

private:
    class MP3Decoder;
    MP3Decoder* m_decoder;

    bool m_rescale;
    double m_mean;
    double m_factor;
};

}

#endif /* MP3FILEREADER_H_ */
