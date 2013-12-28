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

#include "mex.h"
#include "yaafe-core/Engine.h"
#include "yaafe-core/DataFlow.h"
#include "yaafe-core/ComponentFactory.h"
#include "yaafe-core/AudioFileProcessor.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace YAAFE;

class YHandle {
 public:
   YHandle() : flowfile(), engine(NULL) {}
   ~YHandle() { if (engine) delete engine; }

   string flowfile;
   Engine* engine;
};

static int initialized = 0;
static vector<YHandle> s_handles;

void cleanup(void) {
  mexPrintf("MEX-file is terminating, destroying remaining handles\n");
  // TODO
  s_handles.clear();
}

bool parseHandle(const mxArray* arg, int* out) {
  if (!mxIsInt32(arg)) {
    mexPrintf("Invalid yaafe handle format ! (should be Int32)\n");
    return false;
  }
  int* p = (int*) mxGetData(arg);
  if ((*p)<0 || (*p)>=s_handles.size()) {
    mexPrintf("Invalid yaafe handle %i !\n",*p, 0, s_handles.size());
    return false;
  }
  if (s_handles[*p].engine==NULL) {
    mexPrintf("Invalid yaafe handle (already destroyed)");
    return false;
  }
  *out = *p;
  return true;
}

mxArray* makeMxArrayFromInt(int value) {
  mxArray* res = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
  int* resVal = (int*) mxGetData(res);
  *resVal = value;
  return res;
}

mxArray* makeMxArrayFromDouble(double value) {
  mxArray* res = mxCreateNumericMatrix(1,1,mxDOUBLE_CLASS,mxREAL);
  double* resVal = (double*) mxGetData(res);
  *resVal = value;
  return res;
}

void readFeature(mxArray* feats, Engine* engine, const std::string& feature) {
  InputBuffer* buf = engine->getOutput(feature);
  if (!buf) {
    mexPrintf("ERROR: no '%s' output !\n",feature.c_str());
    return;
  }

  /* create output struct */
  const char* field_names[] = {"name","size","sampleStep","frameLength","sampleRate","data"};
  mxArray* feat = mxCreateStructMatrix(1,1,6,field_names);

  /* populate meta data */
  mxSetField(feat,0,"name",mxCreateString(feature.c_str()));
  mxSetField(feat,0,"size",makeMxArrayFromInt(buf->info().size));
  mxSetField(feat,0,"sampleStep",makeMxArrayFromInt(buf->info().sampleStep));
  mxSetField(feat,0,"frameLength",makeMxArrayFromInt(buf->info().frameLength));
  mxSetField(feat,0,"sampleRate",makeMxArrayFromDouble(buf->info().sampleRate));

  /* write data */
  int dims[2] = {buf->info().size , buf->availableTokens()};
  mxArray* featdata = mxCreateNumericArray(2,dims,mxDOUBLE_CLASS,mxREAL);
  double* featdataPtr = (double*) mxGetData(featdata);
  buf->read(featdataPtr,buf->availableTokens());
  buf->consumeTokens(buf->availableTokens());
  mxSetField(feat,0,"data",featdata);

  /* add feat to output */
  int featNumber = mxAddField(feats,feature.c_str());
  mxSetFieldByNumber(feats,0,featNumber,feat);
}

void processBuffer(mxArray* feats,Engine* engine, double* signal, int signalLen)
{
  /* reset engine */
  engine->reset();

  /* get 'audio' input */
  OutputBuffer* audio = engine->getInput("audio");
  if (!audio) {
    mexPrintf("ERROR: no 'audio' input !\n");
    return;
  }

  /* write signal and process by block */
  int pos = 0;
  while (pos<signalLen)
  {
    int toWrite = min(DataBlock::preferedBlockSize(),signalLen-pos);
    audio->write(&signal[pos],toWrite);
    pos += toWrite;
    engine->process();
  }

  /* flush engine to get remaining features */
  engine->flush();

  /* retrieve outputs */
  vector<string> outputs = engine->getOutputs();
  for (int i=0;i<outputs.size();i++) {
    readFeature(feats, engine, outputs[i]);
  }
}

void processFile(mxArray* feats, Engine* engine, char* filename)
{
  AudioFileProcessor afp;
  int res = afp.processFile(*engine,filename);

  if (res==0) {
    /* retrieve outputs */
    vector<string> outputs = engine->getOutputs();
    for (int i=0;i<outputs.size();i++) {
      readFeature(feats, engine, outputs[i]);
    }
  } else {
    mexPrintf("an error occured while processing file %s\n",filename);
  }
}

void mexFunction(int nlhs,
    mxArray *plhs[],
    int nrhs,
    const mxArray *prhs[])
{
  if (!initialized) {
    mexPrintf("MEX-file initializing\n");

    /* Create persistent array and register its cleanup. */
    mexAtExit(cleanup);
    initialized = 1;

    /* register IO components */
    ComponentFactory::instance()->loadLibrary("yaafe-io");
  }

  if (nrhs==0 || !mxIsChar(prhs[0])) {
    mexPrintf("Please specify a command in first parameter !\n");
    return;
  }

  /* parse command string */
  int BUF_SIZE = 4096;
  char buf[BUF_SIZE];
  if (mxGetString(prhs[0],buf,BUF_SIZE)!=0)
  {
    mexPrintf("Invalid action string !\n");
    return;
  }

  if (strcmp(buf,"load")==0)
  {
    /* create engine and load flow */
    if (nlhs!=1) {
      mexPrintf("Please provide one output argument !\n");
      return;
    }
    plhs[0] = mxCreateNumericMatrix(1,1,mxINT32_CLASS ,mxREAL);
    int* out = (int*) mxGetData(plhs[0]);
    *out = -1; /* error code */

    if (nrhs<2 || !mxIsChar(prhs[1])) {
      mexPrintf("Please provide file name !\n");
      return;
    }
    if (mxGetString(prhs[1],buf,BUF_SIZE)!=0) {
      mexPrintf("File name too long !\n");
      return;
    }

    DataFlow df;
    if (!df.load(buf)) {
      mexPrintf("Error while loading flow from file %s",buf);
      return;
    }

    s_handles.push_back(YHandle());
    YHandle& yh = s_handles.back();
    if (yh.engine)
      delete yh.engine;
    yh.engine = new Engine();
    if (yh.engine->load(df)) {
      yh.flowfile = buf;
      *out = s_handles.size()-1;
    } else {
      delete yh.engine;
      yh.engine = NULL;
      yh.flowfile = "";
      *out = -1;
    }
  } else if (strcmp(buf,"destroy")==0) {
    /* release engine */
    if (nrhs<2) {
      mexPrintf("Please provide yaafe handle !\n");
      return;
    }
    int hid;
    if (!parseHandle(prhs[1],&hid))
      return;
    YHandle& yh = s_handles[hid];

    if (yh.engine) {
      delete yh.engine;
      yh.engine = NULL;
    }
  } else if (strcmp(buf,"print")==0) {
    /* display info */
    if (nrhs<2) {
      mexPrintf("Please provide yaafe handle !\n");
      return;
    }
    int hid;
    if (!parseHandle(prhs[1],&hid))
      return;
    YHandle& yh = s_handles[hid];

    mexPrintf("Flow file: %s\n", yh.flowfile.c_str());

  } else if (strcmp(buf,"process")==0) { /* process data */

    if (nlhs!=1) {
      mexPrintf("Please provide one output argument:\n");
      mexPrintf("Usage: features = yaafemex('process',handle,signal)\n");
      return;
    }
    /* create output struct */
    mwSize dims[2] = {1, 1};
    mxArray* feats = mxCreateStructArray(2,dims,0,NULL);
    plhs[0] = feats;

    if (nrhs>3) {
      mexPrintf("Please provide 3 parameters:\n");
      mexPrintf("Usage: features = yaafemex('process',handle,signal)\n");
      return;
    }


    int hid;
    if (!parseHandle(prhs[1],&hid))
      return;
    YHandle& yh = s_handles[hid];

    if (!mxIsDouble(prhs[2])) {
      mexPrintf("Invalid signal format ! must be double !\n");
      return;
    }
    if (mxGetN(prhs[2])!=1 && mxGetM(prhs[2])!=1) {
      mexPrintf("signal must have only one dimension !\n");
      return;
    }
    double* signal = (double*) mxGetData(prhs[2]);
    int signalLen = max(mxGetN(prhs[2]),mxGetM(prhs[2]));

    processBuffer(feats, yh.engine, signal, signalLen);

  } else if (strcmp(buf,"processfile")==0) { /* process file */

    if (nlhs!=1) {
      mexPrintf("Please provide one output argument:\n");
      mexPrintf("Usage: features = yaafemex('processfile',handle,filename)\n");
      return;
    }
    /* create output struct */
    mwSize dims[2] = {1, 1};
    mxArray* feats = mxCreateStructArray(2,dims,0,NULL);
    plhs[0] = feats;

    if (nrhs>3) {
      mexPrintf("Please provide 3 parameters:\n");
      mexPrintf("Usage: features = yaafemex('processfile',handle,filename)\n");
      return;
    }

    int hid;
    if (!parseHandle(prhs[1],&hid))
      return;
    YHandle& yh = s_handles[hid];

    if (mxGetString(prhs[2],buf,BUF_SIZE)!=0) {
      mexPrintf("File name too long !\n");
      return;
    }

    processFile(feats, yh.engine, buf);

  } else {
    mexPrintf("Unknown command %s !\n",buf);
  }

  return;
}
