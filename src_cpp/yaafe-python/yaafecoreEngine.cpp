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

#include "yaafecoreEngine.h"

#include "yaafe-core/Engine.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>

using namespace YAAFE;
using namespace std;

void* engine_create() {
  return new Engine();
}

void engine_destroy(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  delete e;
}

int engine_load(void* engine, void* dataflow) {
  Engine* e = static_cast<Engine*>(engine);
  DataFlow* df = static_cast<DataFlow*>(dataflow);
  return e->load(*df);
}

char** engine_buildStrList(const vector<string>& vec) {
  char** strList = (char**) malloc((vec.size()+1)*sizeof(char*));
  for (int i=0;i<vec.size();i++)
    strList[i] = strdup(vec[i].c_str());
  strList[vec.size()] = NULL;
  return strList;
}

char** engine_getInputList(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  vector<string> inputs = e->getInputs();
  return engine_buildStrList(inputs);
}

char** engine_getOutputList(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  vector<string> outputs = e->getOutputs();
  return engine_buildStrList(outputs);
}

void engine_freeIOList(char** strList) {
  char** ptr = strList;
  while (*ptr!=NULL) {
    free(*ptr);
    ptr++;
  }
  free(strList);
}

char** engine_buildParameterMap(const ParameterMap& params) {
  char** out = (char**) malloc((params.size()*2+1)*sizeof(char*));
  char** ptr = out;
  for (ParameterMap::const_iterator it=params.begin();
      it!=params.end(); it++)
  {
    ptr[0] = strdup(it->first.c_str());
    ptr[1] = strdup(it->second.c_str());
    ptr += 2;
  }
  *ptr = NULL;
  return out;
}

struct IOInfo* engine_getInputInfos(void* engine,char* input) {
  Engine* e = static_cast<Engine*>(engine);
  OutputBuffer* buf = e->getInput(input);
  if (buf==NULL)
    return NULL;
  ParameterMap params = e->getInputParams(input);
  struct IOInfo* info = (struct IOInfo*) malloc(sizeof(struct IOInfo));
  info->sampleRate = buf->info().sampleRate;
  info->sampleStep = buf->info().sampleStep;
  info->frameLength = buf->info().frameLength;
  info->size = buf->info().size;
  info->parameters = engine_buildParameterMap(params);
  return info;
}

struct IOInfo* engine_getOutputInfos(void* engine, char* output) {
  Engine* e = static_cast<Engine*>(engine);
  InputBuffer* buf = e->getOutput(output);
  if (buf==NULL)
    return NULL;
  ParameterMap params = e->getOutputParams(output);
  struct IOInfo* info = (struct IOInfo*) malloc(sizeof(struct IOInfo));
  info->sampleRate = buf->info().sampleRate;
  info->sampleStep = buf->info().sampleStep;
  info->frameLength = buf->info().frameLength;
  info->size = buf->info().size;
  info->parameters = engine_buildParameterMap(params);
  return info;
}

void engine_freeIOInfos(struct IOInfo* i) {
  char** ptr = i->parameters;
  while (*ptr!=NULL) {
    free(*ptr);
    ptr++;
  }
  free(i);
}

void engine_input_write(void* engine, char* input, double* data, int size, int nbtokens) {
  Engine* e = static_cast<Engine*>(engine);
  OutputBuffer* buf = e->getInput(input);
  if (buf==NULL) {
    cerr << "ERROR: unknown input " << input << endl;
    return;
  }
  if (size!=buf->info().size) {
    cerr << "ERROR: invalid input size ! input '" << input << "' has size " << buf->info().size << endl;
    return;
  }
  buf->write(data,nbtokens);
}

void engine_output_available(void* engine, char* output, int* size, int* tokens) {
  Engine* e = static_cast<Engine*>(engine);
  InputBuffer* buf = e->getOutput(output);
  if (buf==NULL) {
    cerr << "ERROR: unknown output " << output << endl;
    *tokens = -1;
    return;
  }
  *size = buf->info().size;
  *tokens = buf->availableTokens();
}

int engine_output_read(void* engine, char* output, double* data, int size, int maxtokens) {
  Engine* e = static_cast<Engine*>(engine);
  InputBuffer* buf = e->getOutput(output);
  if (buf==NULL) {
    cerr << "ERROR: unknown output " << output << endl;
    return -1;
  }
  if (size!=buf->info().size) {
    cerr << "ERROR: invalid output size ! output '" << output << "' has size " << buf->info().size << endl;
    return -1;
  }
  int read = buf->read(data,maxtokens);
  buf->consumeTokens(read);
  return read;
}

void engine_reset(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  e->reset();
}

int engine_process(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  return (e->process() ? 1 : 0);
}

void engine_flush(void* engine) {
  Engine* e = static_cast<Engine*>(engine);
  e->flush();
}

