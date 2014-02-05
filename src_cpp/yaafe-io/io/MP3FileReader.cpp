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

#include "MP3FileReader.h"
#include "yaafe-core/Buffer.h"
#include "SmarcPFilterCache.h"

#include <iostream>
#include <mpg123.h>
#include <cmath>

#define MAX_INT_16 32768.0
#define SILENCE_THRESHOLD 1e-4

using namespace std;

namespace YAAFE {

  class MP3FileReader::MP3Decoder
  {
   public:
     MP3Decoder();
     ~MP3Decoder();

     bool openFile(const std::string& filename, bool resample, int outrate);
     void closeFile();
     int decode();
     double* outBuffer() { return (m_filter ? m_resampleBuffer : m_outbuffer); }

     double m_startSecond;
     double m_limitSecond;
     off_t m_frameLeft;

   private:
     long m_rate;
     int m_channels;
     mpg123_handle* m_mh;
     mpg123_pars* m_mp;
     unsigned char* m_buffer;
     double* m_outbuffer;
     size_t m_bufferSize;

     bool m_resample;
     int m_outrate;

     struct PFilter* m_filter;
     struct PState* m_state;
     int m_resampleBufferSize;
     double* m_resampleBuffer;

     static int s_mpg123refcount;
  };

  int MP3FileReader::MP3Decoder::s_mpg123refcount = 0;

  MP3FileReader::MP3Decoder::MP3Decoder() :
    m_rate(0), m_channels(0), m_mh(NULL), m_buffer(NULL), m_outbuffer(NULL), m_bufferSize(0),
    m_filter(NULL), m_state(NULL), m_resampleBufferSize(0), m_resampleBuffer(NULL),
    m_startSecond(0.0), m_limitSecond(0.0), m_frameLeft(0)
  {
    if (s_mpg123refcount == 0)
    {
      int err = MPG123_OK;
      err = mpg123_init();
      if (err != MPG123_OK)
      {
        cerr << "Trouble with mpg123: " <<  mpg123_plain_strerror(err) << endl;
      }
    }
    s_mpg123refcount++;

    int err = MPG123_OK;
    m_mp = mpg123_new_pars(&err);
    mpg123_par(m_mp, MPG123_ADD_FLAGS, MPG123_QUIET, 0);
    mpg123_par(m_mp, MPG123_RESYNC_LIMIT, -1, 0);
    m_mh = mpg123_parnew(m_mp, NULL, &err);
  }

  MP3FileReader::MP3Decoder::~MP3Decoder()
  {
    closeFile();
    if (m_filter) {
      SmarcPFilterCache::release();
    }
    mpg123_close(m_mh);
    mpg123_delete_pars(m_mp);
    mpg123_delete(m_mh);
    s_mpg123refcount--;
    if (s_mpg123refcount == 0)
    {
      mpg123_exit();
    }
  }

  bool MP3FileReader::MP3Decoder::openFile(const std::string& filename, bool resample, int outrate)
  {
    m_resample = resample;
    m_outrate = outrate;
    int err = MPG123_OK;
    int encoding;
    // const_cast is necessary for working with old version of mpg123
    if (mpg123_open(m_mh, const_cast<char*>(filename.c_str())) != MPG123_OK ||
        mpg123_getformat(m_mh, &m_rate, &m_channels, &encoding) ||
        mpg123_format_none(m_mh))
    {
      cerr << "Trouble with mpg123: " << mpg123_strerror(m_mh) << endl;
      mpg123_close(m_mh);
      return false;
    }
    DBLOG_IF(m_channels > 1, "Warning: MP3FileReader will convert stereo "
                             "audio file to mono doing mean of channels");
    err = mpg123_format(m_mh, m_rate, m_channels, MPG123_ENC_SIGNED_16);
    if (err != MPG123_OK)
    {
      cerr << "ERROR: cannot set MP3 decoder to correct output format: "
        << mpg123_strerror(m_mh) << endl;
      mpg123_close(m_mh);
      return false;
    }

    size_t bufferSize = mpg123_outblock(m_mh);
    if (bufferSize != m_bufferSize)
    {
      if (m_buffer)
        free(m_buffer);
      if (m_outbuffer)
        free(m_outbuffer);
      m_bufferSize = bufferSize;
      m_buffer = (unsigned char*) malloc(m_bufferSize);
      m_outbuffer = (double*) malloc(m_bufferSize*sizeof(double)/sizeof(int16_t));
    }

    if (m_rate!=m_outrate) {
      if (!m_resample) {
        cerr << "ERROR: MP3 file has incorrect sample rate " << m_rate << " (expected " << m_outrate << ")" << endl;
        return false;
      } else {
        m_filter = SmarcPFilterCache::getPFilter(m_rate,m_outrate);
        if (!m_filter) {
          cerr << "ERROR: cannot resample from " << m_rate << " to " << m_outrate << endl;
          return false;
        }
        if (m_state) smarc_destroy_pstate(m_state);
        m_state = smarc_init_pstate(m_filter);
        smarc_reset_pstate(m_state,m_filter);
        int rSize = smarc_get_output_buffer_size(m_filter,m_bufferSize/sizeof(int16_t));
        if (rSize>m_resampleBufferSize) {
          m_resampleBufferSize = rSize;
          if (m_resampleBuffer) free(m_resampleBuffer);
          m_resampleBuffer = (double*) malloc(m_resampleBufferSize*sizeof(double));
        }
      }
    }

    assert(mpg123_tell(m_mh) == 0);
    off_t startFrame = 0;
    if (m_startSecond != 0 || m_limitSecond > 0) {
      mpg123_scan(m_mh);

      if (m_startSecond != 0) {
        // NOTE: m_startSecond might be a negative value
        startFrame = mpg123_timeframe(m_mh, m_startSecond);
        if (startFrame < 0) {
          startFrame = mpg123_seek_frame(m_mh, -startFrame, SEEK_END);
        } else {
          startFrame = mpg123_seek_frame(m_mh, startFrame, SEEK_SET);
        }
      }

      if (m_limitSecond > 0) {
        off_t limit_frame = mpg123_timeframe(m_mh, m_limitSecond);
        off_t stop_frame = mpg123_seek_frame(m_mh, limit_frame, SEEK_CUR);
        m_frameLeft = stop_frame - startFrame + 1;
        mpg123_seek_frame(m_mh, startFrame, SEEK_SET);
      }

      DBLOG("MP3FileReader::init startFrame: %lld, m_startSecond: %lf "
            "m_frameLeft: %lld", startFrame, m_startSecond, m_frameLeft);

    }
    return true;
  }

  void MP3FileReader::MP3Decoder::closeFile()
  {
    if (m_buffer) {
      free(m_buffer);
      m_buffer = NULL;
    }
    if (m_outbuffer) {
      free(m_outbuffer);
      m_outbuffer = NULL;
    }
    if (m_resampleBuffer) {
      free(m_resampleBuffer);
      m_resampleBuffer = NULL;
    }
    if (m_state) {
      smarc_destroy_pstate(m_state);
      m_state = NULL;
    }

    mpg123_close(m_mh);
    m_filter = NULL;
  }

  int MP3FileReader::MP3Decoder::decode()
  {
    size_t done;
    int written;
    int err;
    do {
      if (m_limitSecond > 0) {
        --m_frameLeft;
        if (m_frameLeft < 0) {
          written = 0;
          break;
        }
      }

      err = mpg123_read(m_mh, m_buffer, m_bufferSize, &done);
      if (err != MPG123_OK && err != MPG123_DONE)
      {
        cerr << "ERROR: mp3 decoding ended prematurely because: " << (err
            == MPG123_ERR ? mpg123_strerror(m_mh) : mpg123_plain_strerror(
              err)) << endl;
        return 0;
      }
      done /= (sizeof(int16_t) * m_channels);
      if (done>0) {
        int16_t* bPtr = (int16_t*) m_buffer;
        double* outPtr = m_outbuffer;
        if (m_channels == 1)
        {
          for (size_t i = 0; i < done; ++i)
            *outPtr++ = bPtr[i] / MAX_INT_16;
        }
        else if (m_channels == 2)
        {
          for (size_t i = 0; i < done; ++i)
            *outPtr++ = (bPtr[2*i] + bPtr[2*i+1]) / (2 * MAX_INT_16);
        }
        else
        {
          cerr << "WARNING: invalid number of channel (" << m_channels << ") in mp3 file !" << endl;
          return -1;
        }
        if (m_filter) {
          written = smarc_resample(m_filter,m_state,m_outbuffer,(int)done,m_resampleBuffer,m_resampleBufferSize);
        } else {
          written = (int) done;
        }
      } else {
        if (m_filter) {
          written = smarc_resample_flush(m_filter,m_state,m_resampleBuffer,m_resampleBufferSize);
        } else {
          written = (int) done;
        }
      }
    } while ((written==0) && (err!=MPG123_DONE));
    return written;
  }

  MP3FileReader::MP3FileReader() : m_rescale(false), m_mean(0.0), m_factor(1.0) {
    m_decoder = new MP3Decoder();
  }

  MP3FileReader::~MP3FileReader() {
    delete m_decoder;
  }

  ParameterDescriptorList MP3FileReader::getParameterDescriptorList() const
  {
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
    p.m_description = "time position where to start process, if given a negative value(e.g: \"-10s\" ), it will start at the last 10s.";
    p.m_defaultValue = "0s";
    pList.push_back(p);

    p.m_identifier = "TimeLimit";
    p.m_description = "longest time duration to keep, 0s means no limit. If the given value is longer than the available duration, the excess should be ignored.";
    p.m_defaultValue = "0s";
    pList.push_back(p);

    return pList;
  }

  bool MP3FileReader::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
  {
    assert(inp.size()==0);

    bool resample = (getStringParam("Resample",params)=="yes");
    bool removemean = (getStringParam("RemoveMean",params)=="yes");
    double scaleMax = getDoubleParam("ScaleMax",params);
    int sr = getIntParam("SampleRate",params);
    string filename = getStringParam("File",params);
    string timeStart = getStringParam("TimeStart",params);
    string timeLimit = getStringParam("TimeLimit",params);
    double startSecond, limitSecond;

    if (timeStart[timeStart.size()-1]=='s')
    {
      startSecond = atof(timeStart.substr(0,timeStart.size()-1).c_str());
    } else {
      cerr << "ERROR: invalid TimeStart parameter !" << endl;
      return false;
    }

    if (timeLimit[timeLimit.size()-1]=='s')
    {
      limitSecond = atof(timeLimit.substr(0,timeLimit.size()-1).c_str());
    } else {
      cerr << "ERROR: invalid TimeLimit parameter !" << endl;
      return false;
    }

    m_decoder->m_startSecond = startSecond;
    m_decoder->m_limitSecond = limitSecond;
    if (!m_decoder->openFile(filename,resample,sr))
      return false;

    if (removemean || scaleMax>0) {
      // read all file to extract mean and scale factor
      double sum = 0.0;
      double min = 0.0;
      double max = 0.0;
      long count = 0;
      int r = 0;
      double* buf = m_decoder->outBuffer();
      while ( (r = m_decoder->decode()) ) {
        count += r;
        for (int i=0;i<r;i++) {
          register double v = buf[i];
          sum += v;
          if (v<min) min=v;
          if (v>max) max=v;
        }
      }
      m_rescale = true;
      m_mean = sum / count;
      min = abs(min-m_mean);
      max = abs(max-m_mean);
      m_factor = scaleMax / (max>min ? max : min);
      m_decoder->closeFile();
      if (!m_decoder->openFile(filename,resample,sr)) {
        cerr << "ERROR: cannot re-open file " << filename << " !" << endl;
        return false;
      }
    }

    outStreamInfo().add(StreamInfo());
    StreamInfo& out = outStreamInfo()[0].data;
    out.size = 1;
    out.sampleRate = sr;
    out.frameLength = 1;
    out.sampleStep = 1;

    return true;
  }

  bool MP3FileReader::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==0);
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    int toRead = min(out->remainingSpace(), DataBlock::preferedBlockSize());
    if (toRead == 0)
      toRead = DataBlock::preferedBlockSize();

    int nbRead = 0;
    double* buf = m_decoder->outBuffer();
    while (nbRead<toRead)
    {
      int read = m_decoder->decode();
      if (read==0) break;
      if (m_rescale) {
        for (int i=0;i<read;i++) {
          buf[i] = (buf[i]-m_mean)*m_factor;
        }
      }

      out->write(buf,read);
      nbRead += read;
    }
    return (nbRead > 0);
  }

  void MP3FileReader::flush(InputBuffer* in, OutputBuffer* out)
  {
    // do not read any more data
  }

}
