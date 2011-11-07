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

#ifndef OUTPUTFORMAT_H
#define OUTPUTFORMAT_H

#include "utils.h"
#include "Component.h"

#include <vector>

namespace YAAFE {

class OutputFormat {
public:

	virtual const std::string getId() const = 0;
	virtual const std::string getDescription() const = 0;
	virtual bool available() const = 0;
	virtual const ParameterDescriptorList getParameters() const = 0;
	virtual OutputFormat* clone() const = 0;
	virtual void setParameters(const std::string& outDir,
							   const ParameterMap& formatParams);
	virtual Component* createWriter(
			const std::string& inputfile,
			const std::string& feature,
			const ParameterMap& featureParams,
			const Ports<StreamInfo>& featureStream) = 0;

	static std::vector<std::string> availableFormats();
	static const OutputFormat* get(const std::string& id);

	static void registerFormat(const OutputFormat* format);
protected:
	std::string m_outDir;
	ParameterMap m_params;

	static std::string filenameConcat(const std::string& outDir,
			const std::string& filename, const std::string& suffix);
	static void eraseParameterDescriptor(ParameterDescriptorList& list, const std::string& id);
private:
	static std::vector<const OutputFormat*>& allFormats();
};

class CSVOutputFormat : public OutputFormat {
public:
	virtual const std::string getId() const { return "csv"; }
	virtual const std::string getDescription() const { return "Creates one CSV file per feature and input file."; }
	virtual bool available() const;
	virtual const ParameterDescriptorList getParameters() const;
	virtual OutputFormat* clone() const { return new CSVOutputFormat(); }
	virtual Component* createWriter(
			const std::string& inputfile,
			const std::string& feature,
			const ParameterMap& featureParams,
			const Ports<StreamInfo>& featureStream);
};

class H5OutputFormat : public OutputFormat {
public:
	virtual const std::string getId() const { return "h5"; }
	virtual const std::string getDescription() const { return "Creates one H5 file per input file, containing one dataset per features."; }
	virtual bool available() const;
	virtual const ParameterDescriptorList getParameters() const;
	virtual OutputFormat* clone() const { return new H5OutputFormat(); }
	virtual Component* createWriter(
			const std::string& inputfile,
			const std::string& feature,
			const ParameterMap& featureParams,
			const Ports<StreamInfo>& featureStream);
};


} // YAAFE

#endif
