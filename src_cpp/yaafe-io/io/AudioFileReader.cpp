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

#include "AudioFileReader.h"
#include "SmarcPFilterCache.h"
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <cmath>

using namespace std;

namespace YAAFE {

  AudioFileReader::AudioFileReader() :
    m_sampleRate(0), m_bufferSize(0), m_sndfile(NULL), m_sfinfo(), m_readBuffer(NULL),
    m_filter(NULL), m_state(NULL), m_resampleBufferSize(0), m_resampleBuffer(NULL),
    m_resample(false), m_rescale(false), m_mean(0), m_factor(1), m_startSecond(0),
    m_limitSecond(0), m_frameLeft(0) {
    }

  AudioFileReader::~AudioFileReader() {
    closeFile();
    if (m_filter) {
      SmarcPFilterCache::release();
    }
    if (m_readBuffer)
      delete[] m_readBuffer;
    if (m_resampleBuffer)
      delete[] m_resampleBuffer;
  }

  int AudioFileReader::readFramesIntoBuffer() {
    assert(m_readBuffer!=NULL);
    assert((m_sfinfo.channels==1) || (m_sfinfo.channels==2));
    int nbRead = sf_readf_double(m_sndfile,m_readBuffer,m_bufferSize);
    if (m_sfinfo.channels == 2) {
      for (int i=0;i<nbRead;++i)
        m_readBuffer[i] = 0.5 * (m_readBuffer[2*i] + m_readBuffer[2*i+1]);
    }
    if (m_filter) {
      // resample
      if (nbRead) {
        nbRead = smarc_resample(m_filter,m_state,m_readBuffer,nbRead,m_resampleBuffer,m_resampleBufferSize);
        memcpy(m_readBuffer,m_resampleBuffer,nbRead*sizeof(double));
      } else {
        nbRead = smarc_resample_flush(m_filter,m_state,m_readBuffer,m_resampleBufferSize);
      }
    }

    if (m_frameLeft > 0 ) {
      m_frameLeft -= nbRead;
      if (m_frameLeft <= 0) {
        return 0;
      }
    }
    return nbRead;
  }

  bool AudioFileReader::openFile(const std::string& filename) {
    // close file if one is opened
    if (m_sndfile) {
      if (verboseFlag)
        cerr << "WARNING: close old audio file when opening " << filename
          << endl;
      closeFile();
    }

    memset(&m_sfinfo, 0, sizeof(SF_INFO));
    m_sndfile = sf_open(filename.c_str(), SFM_READ, &m_sfinfo);
    if (!m_sndfile) {
      cerr << "ERROR: cannot open audio file " << filename << ": " << sf_strerror(m_sndfile) << endl;
      return false;
    }
    if (m_sfinfo.channels > 1) {
      if (m_sfinfo.channels > 2) {
        cerr
          << "ERROR: AudioFileReader cannot handle audio files with more than 2 channels !"
          << endl;
        closeFile();
        return false;
      }
      DBLOG_IF(m_sfinfo.channels > 1, "Warning: AudioFileReader will convert stereo "
                               "audio file to mono doing mean of channels");
    }

    if (m_resample && (m_sampleRate!=m_sfinfo.samplerate)) {
      int fsin = m_sfinfo.samplerate;
      m_filter = SmarcPFilterCache::getPFilter(fsin,m_sampleRate);
      if (!m_filter) return false;
      m_state = smarc_init_pstate(m_filter);
      m_resampleBufferSize = smarc_get_output_buffer_size(m_filter,m_bufferSize);
      m_resampleBuffer = new double[m_resampleBufferSize];
      if (m_resampleBufferSize>(2*m_bufferSize)) {
        delete [] m_readBuffer;
        m_readBuffer = new double[m_resampleBufferSize];
      }

    }

    sf_count_t startFrame = static_cast<sf_count_t>(m_startSecond * m_sampleRate);
    m_frameLeft = static_cast<sf_count_t>(m_limitSecond * m_sampleRate);
    if (startFrame <= -m_sfinfo.frames) {
      startFrame = 0; 
    }

    int res = 0;
    if (startFrame !=0) {
      if (!m_sfinfo.seekable) {
        cerr << "ERROR: audio file " << filename << " is not seekable!" << endl;
        return false;
      }
      if (startFrame > 0) {
        res = sf_seek(m_sndfile, startFrame, SEEK_SET);
      }
      else {
        res = sf_seek(m_sndfile, startFrame, SEEK_END);
      }
    }

    if (res == -1)
    {
      cerr << "ERROR: cannot seek start in audio file !" << endl;
      return false;
    }

    DBLOG("AudioFileReader::openFile startFrame: %lld, m_frameLeft: %lld",
          startFrame, m_frameLeft);

    return true;
  }

  void AudioFileReader::closeFile() {
    if (m_state) {
      smarc_destroy_pstate(m_state);
      m_state = NULL;
    }
    if (m_sndfile) {
      sf_close(m_sndfile);
      m_sndfile = NULL;
    }
  }

  ParameterDescriptorList AudioFileReader::getParameterDescriptorList() const {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "File";
    p.m_description = "audio file to read";
    p.m_defaultValue = "";
    pList.push_back(p);

    p.m_identifier = "RemoveMean";
    p.m_description = "If 'yes' then the whole file read when initialize, and mean is removed in signal outputed. If 'no', signal is outputed as is.";
    p.m_defaultValue = "no";
    pList.push_back(p);

    p.m_identifier = "ScaleMax";
    p.m_description = "Scale signal so that maximum of absolute value reached the given value. If given value is negative, nothing is done.";
    p.m_defaultValue = "-1";
    pList.push_back(p);

    p.m_identifier = "SampleRate";
    p.m_description = "Check audio sample rate.";
    p.m_defaultValue = "16000";
    pList.push_back(p);

    p.m_identifier = "Resample";
    p.m_description = "yes|no, allows to resample original audio to desired sampleRate";
    p.m_defaultValue = "no";
    pList.push_back(p);

    p.m_identifier = "TimeStart";
    p.m_description = "time position where to start process";
    p.m_defaultValue = "0s";
    pList.push_back(p);

    p.m_identifier = "TimeLimit";
    p.m_description = "longest time duration to keep, 0s means no limit";
    p.m_defaultValue = "0s";
    pList.push_back(p);

    return pList;
  }

  bool AudioFileReader::init(const ParameterMap& params, const Ports<StreamInfo>& in)
  {
    m_resample = (getStringParam("Resample", params)=="yes");
    m_removemean = (getStringParam("RemoveMean",params)=="yes");
    m_scaleMax = getDoubleParam("ScaleMax",params);
    m_sampleRate = 	getIntParam("SampleRate",params);
    string filename = getStringParam("File", params);
    string timeStart = getStringParam("TimeStart",params);

    if (timeStart[timeStart.size()-1]=='s')
    {
      m_startSecond = atof(timeStart.substr(0,timeStart.size()-1).c_str());
    } else {
      cerr << "ERROR: invalid TimeStart parameter !" << endl;
      return false;
    }
    string timeLimit = getStringParam("TimeLimit",params);
    if (timeLimit[timeLimit.size()-1]=='s')
    {
      m_limitSecond = atof(timeLimit.substr(0,timeLimit.size()-1).c_str());
    } else {
      cerr << "ERROR: invalid TimeLimit parameter !" << endl;
      return false;
    }

    m_bufferSize = DataBlock::preferedBlockSize();
    m_readBuffer = new double[2*m_bufferSize]; // enough to read stereo if needed

    if (!openFile(filename))
      return false;
    
    // find mean and max if needed
    m_mean = 0;
    m_factor = 1;
    if (m_removemean || m_scaleMax>0)
    {
      m_rescale = true;

      int count = 0;
      double smean = 0;
      double smin = 0;
      double smax = 0;
      while (true) {
        int nbRead = readFramesIntoBuffer();
        if (nbRead==0)
          break;
        for (int i=0;i<nbRead;i++)
        {
          smean += m_readBuffer[i];
          if (m_readBuffer[i]<smin)
            smin = m_readBuffer[i];
          if (m_readBuffer[i]>smax)
            smax = m_readBuffer[i];
        }
        count += nbRead;
      }

      if (m_removemean)
        m_mean = smean / count;
      if (m_scaleMax>0)
        m_factor = m_scaleMax / max(abs( (smax-m_mean) ),abs( (smin-m_mean) ));

      int res = sf_seek(m_sndfile,0,SEEK_SET);
      if (res!=0)
      {
        cerr << "ERROR: cannot seek start in audio file !" << endl;
        return false;
      }

      if (verboseFlag)
        cerr << "INFO: remove mean of input signal (" << m_mean << ") and scale to " << m_scaleMax << endl;
      closeFile();
      if (!openFile(filename)) {
        cerr << "ERROR: cannot re-open file " << filename << " !" << endl; 
        return false;
      }
    }

    if ((!m_resample) && (m_sfinfo.samplerate != m_sampleRate)) {
      cerr << "ERROR: resampling is disabled and file has sample rate " << m_sfinfo.samplerate << " Hz. Expecting " << m_sampleRate << " Hz !" << endl;
      return false;
    }

    outStreamInfo().add(StreamInfo());
    StreamInfo& outInfo = outStreamInfo()[0].data;
    outInfo.size = 1;
    outInfo.sampleRate = m_sampleRate;
    outInfo.sampleStep = 1;
    outInfo.frameLength = 1;
    return true;
  }

  bool AudioFileReader::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==0);
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    int nbRead = readFramesIntoBuffer();
    if (nbRead==0)
      return false;
    if (m_rescale)
      for (int i=0;i<nbRead;i++)
        m_readBuffer[i] = (m_readBuffer[i] - m_mean) * m_factor;
    out->write(m_readBuffer,nbRead);
    return true;
  }

}
