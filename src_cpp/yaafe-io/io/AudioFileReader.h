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

#ifndef AUDIOFILEREADERCOMPONENT_H_
#define AUDIOFILEREADERCOMPONENT_H_

#include "yaafe-core/Component.h"
#include "sndfile.h"
#include "smarc.h"

#define AUDIO_FILE_READER_ID "AudioFileReader"

namespace YAAFE
{

class AudioFileReader: public ComponentBase<AudioFileReader>
{
public:
    AudioFileReader();
    virtual ~AudioFileReader();

    virtual const std::string getIdentifier() const  { return AUDIO_FILE_READER_ID; }
    virtual bool stateLess() const { return false; };

    virtual ParameterDescriptorList getParameterDescriptorList() const;

    virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
	virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);


private:
	int m_sampleRate;
    int m_bufferSize;
    SNDFILE* m_sndfile;
    SF_INFO m_sfinfo;
    double* m_readBuffer;

    struct PFilter* m_filter;
    struct PState* m_state;
    int m_resampleBufferSize;
    double* m_resampleBuffer;

    bool m_resample;
    bool m_removemean;
    double m_scaleMax;

    bool m_rescale;
    double m_mean;
    double m_factor;

    bool openFile(const std::string& filename);
    void closeFile();
    int readFramesIntoBuffer(); // read m_bufferSize frames

};

}

#endif /* AUDIOFILEREADERCOMPONENT_H_ */
