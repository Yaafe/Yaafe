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

#include "yaafecoreAudioFileProcessor.h"

#include "yaafe-core/AudioFileProcessor.h"
#include "yaafe-core/Engine.h"
#include "stdlib.h"
#include "string.h"

using namespace std;
using namespace YAAFE;

void* audiofileprocessor_create() {
	return new AudioFileProcessor();
}

void audiofileprocessor_destroy(void* afp)
{
	AudioFileProcessor* ptr = static_cast<AudioFileProcessor*>(afp);
	delete ptr;
}

int audiofileprocessor_setOutputFormat(void* afp, char* format, char* outDir, char** params)
{
	AudioFileProcessor* afp_ = static_cast<AudioFileProcessor*>(afp);
	ParameterMap paramMap;
	char** ptr = params;
	while (*ptr) {
		paramMap.insert(make_pair(ptr[0],ptr[1]));
		ptr += 2;
	}
	return (afp_->setOutputFormat(format, outDir, paramMap) ? 1 : 0);
}

int audiofileprocessor_processFile(void* afp,void* engine, char* filename)
{
	AudioFileProcessor* ptr = static_cast<AudioFileProcessor*>(afp);
	Engine* e = static_cast<Engine*>(engine);
	if (e==NULL)
		return -11;
	return ptr->processFile(*e,filename);
}

