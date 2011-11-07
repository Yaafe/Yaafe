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

#include "OutputFormat.h"
#include "ComponentFactory.h"
#include <iostream>
#include <algorithm>

using namespace std;

namespace YAAFE {

std::vector<std::string> OutputFormat::availableFormats()
{
	vector<string> av;
	for (vector<const OutputFormat*>::iterator it=allFormats().begin();
			it!=allFormats().end(); it++) {
		if ((*it)->available())
			av.push_back((*it)->getId());
	}
	return av;
}

const OutputFormat* OutputFormat::get(const std::string& id)
{
	for (vector<const OutputFormat*>::iterator it=allFormats().begin();
			it!=allFormats().end(); it++) {
		if ((*it)->getId()==id)
			return *it;
	}
	cerr << "ERROR: unknown OutputFormat " << id << " ! (returning NULL)" << endl;
	return NULL;
}

void OutputFormat::registerFormat(const OutputFormat* format)
{
	allFormats().push_back(format);
}

std::vector<const OutputFormat*>& OutputFormat::allFormats()
{
	static vector<const OutputFormat*> allFormats;
	if (allFormats.size()==0) {
		allFormats.push_back(new CSVOutputFormat());
		allFormats.push_back(new H5OutputFormat());
	}
	return allFormats;
}

void OutputFormat::setParameters(const std::string& outDir,
		const ParameterMap& formatParams)
{
	m_outDir = outDir;
	m_params.clear();
	ParameterDescriptorList pList = getParameters();
	for (ParameterDescriptorList::iterator it=pList.begin();
			it!=pList.end(); it++) {
		ParameterMap::const_iterator pIt = formatParams.find(it->m_identifier);
		if (pIt!=formatParams.end()) {
			m_params[it->m_identifier] = pIt->second;
		} else {
			m_params[it->m_identifier] = it->m_defaultValue;
		}
	}
}

std::string OutputFormat::filenameConcat(const std::string& outDir,
		const std::string& filename, const std::string& suffix)
{
	string outfile = "";
	if (filename[0]=='/') {
		if (outDir.size()==0)
			outfile = filename;
		else if (outDir[outDir.size()-1]=='/')
			outfile = outDir + filename.substr(1);
		else
			outfile = outDir + filename;
	} else {
		if (outDir.size()==0)
			outfile = filename;
		else if (outDir[outDir.size()-1]!='/')
			outfile = outDir + "/" + filename;
		else
			outfile = outDir + filename;
	}
	return outfile + suffix;
}

void OutputFormat::eraseParameterDescriptor(ParameterDescriptorList& list,
		const std::string& id)
{
	for (ParameterDescriptorList::iterator it=list.begin();
			it!=list.end(); it++)
	{
		if (it->m_identifier==id) {
			list.erase(it);
			return;
		}
	}
	cerr << "WARNING: cannot erase " << id << " from ParameterDescriptorList ! (not found)" << endl;
}

/*
 *  CSVOutputFormat functions
 */

bool CSVOutputFormat::available() const {
	return ComponentFactory::instance()->exists("CSVWriter");
}

const ParameterDescriptorList CSVOutputFormat::getParameters() const {
	const Component* csvw = ComponentFactory::instance()->getPrototype("CSVWriter");
	if (csvw==NULL) {
		cerr << "WARNING: CSVWriter not available ! cannot retrieve parameter list" << endl;
		return ParameterDescriptorList();
	}
	ParameterDescriptorList pList = csvw->getParameterDescriptorList();
	eraseParameterDescriptor(pList,"File");
	eraseParameterDescriptor(pList,"Attrs");
	return pList;
}

Component* CSVOutputFormat::createWriter(
		const std::string& inputfile,
		const std::string& feature,
		const ParameterMap& featureParams,
		const Ports<StreamInfo>& featureStream)
{
	Component* writer = ComponentFactory::instance()->createComponent("CSVWriter");
	ParameterMap writerParams = m_params;
	writerParams["File"] = filenameConcat(m_outDir,inputfile,"." + feature + ".csv");
	writerParams["Attrs"] = encodeParameterMap(featureParams);
	if (!writer->init(writerParams,featureStream)) {
		delete writer;
		cerr << "ERROR: cannot initialize CSVWriter !" << endl;
		return NULL;
	}
	return writer;
}

/*
 * H5OutputFormat functions
 */


bool H5OutputFormat::available() const {
	return ComponentFactory::instance()->exists("H5DatasetWriter");
}

const ParameterDescriptorList H5OutputFormat::getParameters() const
{
	const Component* h5proto = ComponentFactory::instance()->getPrototype("H5DatasetWriter");
	if (h5proto==NULL) {
		cerr << "WARNING: H5DatasetWriter not available ! cannot retrieve parameter list" << endl;
		return ParameterDescriptorList();
	}
	ParameterDescriptorList pList = h5proto->getParameterDescriptorList();
	eraseParameterDescriptor(pList,"H5File");
	eraseParameterDescriptor(pList,"H5Attrs");
	eraseParameterDescriptor(pList,"Dataset");
	return pList;
}

Component* H5OutputFormat::createWriter(
		const std::string& inputfile,
		const std::string& feature,
		const ParameterMap& featureParams,
		const Ports<StreamInfo>& featureStream)
{
	Component* writer = ComponentFactory::instance()->createComponent("H5DatasetWriter");
	ParameterMap writerParams = m_params;
	writerParams["H5File"] = filenameConcat(m_outDir,inputfile,".h5");
	writerParams["H5Attrs"] = encodeParameterMap(featureParams);
	writerParams["Dataset"] = feature;
	if (!writer->init(writerParams,featureStream)) {
		delete writer;
		cerr << "ERROR: cannot initialize H5DatasetWriter !" << endl;
		return NULL;
	}
	return writer;
}

}
