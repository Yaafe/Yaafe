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

#ifndef YAAFECOREENGINE_H_
#define YAAFECOREENGINE_H_

extern "C" {

  void* engine_create();
  void engine_destroy(void* engine);

  int engine_load(void* engine, void* dataflow);

  char** engine_getInputList(void* engine);
  char** engine_getOutputList(void* engine);
  void engine_freeIOList(char** strList);

  struct IOInfo {
    double sampleRate;
    int sampleStep;
    int frameLength;
    int size;
    char** parameters;
  };

  struct IOInfo* engine_getInputInfos(void* engine,char* input);
  struct IOInfo* engine_getOutputInfos(void* engine, char* output);
  void engine_freeIOInfos(struct IOInfo* i);

  void engine_input_write(void* engine, char* input, double* data, int size, int nbtokens);
  void engine_output_available(void* engine, char* output, int* size, int* tokens);
  int engine_output_read(void* engine, char* output, double* data, int size, int maxtokens);

  void engine_reset(void* engine);
  int engine_process(void* engine);
  void engine_flush(void* engine);

}


#endif /* YAAFECOREENGINE_H_ */
