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

#include "CSVWriter.h"
#include "FileUtils.h"
#include <sstream>

#define BUFSIZE 64

using namespace std;

namespace YAAFE {

  CSVWriter::CSVWriter() {
  }

  CSVWriter::~CSVWriter() {
    if (m_fout.is_open())
      m_fout.close();
  }

  ParameterDescriptorList CSVWriter::getParameterDescriptorList() const
  {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "File";
    p.m_description = "CSV output filename";
    p.m_defaultValue = "";
    pList.push_back(p);

    p.m_identifier = "Attrs";
    p.m_description = "Metadata to be written to the output";
    p.m_defaultValue = "";
    pList.push_back(p);

    p.m_identifier = "Metadata";
    p.m_description = "If 'True' then write metadata as comments at the beginning of the csv file. If 'False', do not write metadata";
    p.m_defaultValue = "True";
    pList.push_back(p);

    p.m_identifier = "Precision";
    p.m_description = "precision of output floating point number.";
    p.m_defaultValue = "6";
    pList.push_back(p);

    return pList;
  }

  bool CSVWriter::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
  {
    assert(inp.size()==1);
    const StreamInfo& in = inp[0].data;

    string outputFile = getStringParam("File", params);
    m_precision = getIntParam("Precision",params);
    if (m_precision > (BUFSIZE-10)) {
      cerr << "WARNING: precision is too large ! use precision " << BUFSIZE - 10 << endl;
      m_precision = BUFSIZE - 10;
    }

    int res = preparedirs(outputFile.c_str());
    if (res!=0)
      return false;

    m_fout.open(outputFile.c_str(), ios_base::trunc);
    if (!m_fout.is_open() || m_fout.bad())
      return false;

    if (getStringParam("Metadata",params)=="True") {
      // write metadata at the beginnig of the file
      string paramStr = getStringParam("Attrs",params);
      map<string,string> params = decodeAttributeStr(paramStr);
      ostringstream oss;
      for (map<string,string>::const_iterator it=params.begin();it!=params.end();it++)
        oss << "% " << it->first << "=" << it->second << endl;
      m_fout.write(oss.str().c_str(),oss.str().size());
    }

    return true;
  }

  void CSVWriter::reset() {
    // nothing to do
  }

  bool CSVWriter::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    assert(outp.size()==0);
    char buf[BUFSIZE];
    while (!in->empty())
    {
      double* data = in->readToken();
      int strSize = sprintf(buf,"%0.*e",m_precision,data[0]);
      m_fout.write(buf,strSize);
      for (int i=1;i<in->info().size;i++)
      {
        strSize = sprintf(buf,",%0.*e",m_precision,data[i]);
        m_fout.write(buf,strSize);
      }
      m_fout << endl;
      in->consumeToken();
    }
    return true;
  }

  void CSVWriter::flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out)
  {
    process(in,out);
    m_fout.close();
  }

}

