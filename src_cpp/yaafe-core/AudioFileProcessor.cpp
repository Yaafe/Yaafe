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

#include "AudioFileProcessor.h"
#include "ComponentFactory.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <time.h>

using namespace std;

namespace YAAFE {

bool hasEnding (const std::string& fullString, const std::string& ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

AudioFileProcessor::AudioFileProcessor() : m_format(NULL) {
}

AudioFileProcessor::~AudioFileProcessor() {
	if (m_format)
		delete m_format;
}

bool AudioFileProcessor::setOutputFormat(const std::string& format,
		const std::string& outDir,
		const ParameterMap& params)
{
	if (m_format)
		delete m_format;
	const OutputFormat* prototype = OutputFormat::get(format);
	if (prototype==NULL || !prototype->available())
		return false;
	m_format = prototype->clone();
	m_format->setParameters(outDir,params);
	return true;
}

int AudioFileProcessor::processFile(Engine& engine, const std::string& filename)
{
	{
		// check engine inputs
		vector<string> inputs = engine.getInputs();
		if (inputs.size()!=1) {
			cerr << "ERROR: dataflow must have exactly 1 root node" << endl;
			return -1;
		}
		if (inputs[0]!="audio") {
			cerr << "ERROR: root node is not 'audio' node !" << endl;
			return -2;
		}
	}

	int exitCode = 0;
	ComponentFactory* factory = ComponentFactory::instance();
	Component* reader = NULL;
	std::vector<pair<string,Component*> > writers;

	cerr << "process file " << filename << endl;
	clock_t end;
	clock_t start = clock();

	// reset engine state
	engine.reset();

	// determine audio file format and select appropriate reader
	std::string readerComponent = "";
	{
		std::string lowerFilename = filename;
		transform(lowerFilename.begin(),lowerFilename.end(),lowerFilename.begin(),::tolower);
		if (hasEnding(lowerFilename,"mp3")) {
			if (!factory->exists("MP3FileReader")) {
				cerr << "ERROR: cannot read mp3 file ! please compile yaafe with mpg123 support" << endl;
				return -4;
			}
			readerComponent = "MP3FileReader";
		} else {
			if (!factory->exists("AudioFileReader")) {
				cerr << "ERROR: please compile yaafe with libsndfile support" << endl;
				return -3;
			}
			readerComponent = "AudioFileReader";
		}
	}

	// initialize reader
	ParameterMap readerParams = engine.getInputParams("audio");
	readerParams["File"] = filename;
	reader = factory->createComponent(readerComponent);
	Ports<StreamInfo> inports;
	if (!reader->init(readerParams,inports)) {
		cerr << "ERROR: cannot initialize reader " << readerComponent << " for file " << filename << endl;
		exitCode = -6; goto exit;
	}

	// bind audio input
	if (!engine.bindInput("audio",reader)) {
		cerr << "ERROR: cannot bind audio reader" << endl;
		exitCode = -7; goto exit;
	}

	// configure outputs
	if (m_format)
	{
		vector<string> outputs = engine.getOutputs();
		for (int i=0;i<outputs.size();i++) {
			const string& outName = outputs[i];
			Ports<StreamInfo> inp;
			inp.add(engine.getOutput(outName)->info());
			Component* writer = m_format->createWriter(filename,
					outName,engine.getOutputParams(outName),inp);
			if (!writer) {
				cerr << "ERROR: cannot initialize writer for output " << outName << endl;
				exitCode = -8; goto exit;
			}
			engine.bindOutput(outName,writer);
			writers.push_back(make_pair(outName,writer));
		}
	}

	// process audio
	while (engine.process());

	// flush last samples
	engine.flush();

	end = clock();
	cerr << "done in " << (float) (end - start)
			/ (float) CLOCKS_PER_SEC << "s" << endl;

	exit:
	// detach and release reader
	engine.detachInput("audio");
	if (reader) delete reader;
	// detach and release writers
	vector<string> outputs  = engine.getOutputs();
	for (vector<pair<string,Component*> >::iterator it=writers.begin();it!=writers.end();it++) {
		engine.detachOutput(it->first);
		delete it->second;
	}
	writers.clear();
	return exitCode;
}


}
