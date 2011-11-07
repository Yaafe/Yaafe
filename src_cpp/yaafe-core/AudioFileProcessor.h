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

#ifndef AUDIOFILEPROCESSOR_H_
#define AUDIOFILEPROCESSOR_H_

#include "DataFlow.h"
#include "Engine.h"
#include "OutputFormat.h"

namespace YAAFE {

class AudioFileProcessor {
public:
	AudioFileProcessor();
	virtual ~AudioFileProcessor();

	bool setOutputFormat(const std::string& format, const std::string& outdir, const ParameterMap& params);
	int processFile(Engine& engine, const std::string& filename);
private:
	OutputFormat* m_format;
};

}

#endif /* AUDIOFILEPROCESSOR_H_ */
