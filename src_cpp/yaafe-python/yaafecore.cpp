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

#include "yaafecore.h"

#include "stdlib.h"
#include "string.h"

#include "yaafe-core/ComponentFactory.h"
#include "yaafe-core/Component.h"
#include "yaafe-core/Buffer.h"
#include "yaafe-core/OutputFormat.h"

using namespace YAAFE;
using namespace std;

void destroyFactory() {
	ComponentFactory::destroy();
}

const char* getYaafeVersion() {
	return ComponentFactory::version();
}


int loadComponentLibrary(char* filename)
{
	return ComponentFactory::instance()->loadLibrary(filename);
}

int isComponentAvailable(char* id)
{
	const vector<const Component*>& pList = ComponentFactory::instance()->getPrototypeList();
	for (vector<const Component*>::const_iterator it=pList.begin();
			it!=pList.end(); it++)
		if ((*it)->getIdentifier()==id)
			return 1;
	return 0;
}

char** makeStrList(const vector<string>& vec) {
	char** out = new char*[vec.size()+1];
	for (int i=0;i<vec.size();i++)
		out[i] = strdup(vec[i].c_str());
	out[vec.size()] = NULL;
	return out;
}

char** getComponentList() {
	const vector<const Component*>& pList = ComponentFactory::instance()->getPrototypeList();
	vector<string> res;
	for (vector<const Component*>::const_iterator it=pList.begin();
			it!=pList.end(); it++)
		res.push_back((*it)->getIdentifier());
	return makeStrList(res);
}

char** getOutputFormatList() {
	vector<string> oList = OutputFormat::availableFormats();
	return makeStrList(oList);
}

void freeComponentList(char** strList) {
	if (strList==NULL) return;
	char** ptr = strList;
	while (*ptr!=NULL) {
		free(*ptr);
		ptr++;
	}
	delete [] strList;
}

char* getOutputFormatDescription(char* id) {
	const OutputFormat* proto = OutputFormat::get(id);
	if (!proto)
		return NULL;
	return strdup(proto->getDescription().c_str());
}

void freeOutputFormatDescription(char* desc) {
	if (!desc)
		free(desc);
}

struct ComponentParameter** makeComponentParameterList(const ParameterDescriptorList& pList)
{
	struct ComponentParameter** params =
			(struct ComponentParameter**) malloc((pList.size()+1)*sizeof(struct ComponentParameter*));
	for (int i=0;i<pList.size();i++)
	{
		params[i] = (struct ComponentParameter*) malloc(sizeof(struct ComponentParameter));
		params[i]->identifier = strdup(pList[i].m_identifier.c_str());
		params[i]->defaultValue = strdup(pList[i].m_defaultValue.c_str());
		params[i]->description = strdup(pList[i].m_description.c_str());
	}
	params[pList.size()] = NULL;
	return params;
}

struct ComponentParameter** getComponentParameters(const char* id)
{
	const Component* proto = ComponentFactory::instance()->getPrototype(id);
	if (proto)
		return makeComponentParameterList(proto->getParameterDescriptorList());
	return NULL;
}

struct ComponentParameter** getOutputFormatParameters(const char* id)
{
	const OutputFormat* format = OutputFormat::get(id);
	if (format)
		return makeComponentParameterList(format->getParameters());
	return NULL;
}


void freeComponentParameters(struct ComponentParameter** params)
{
	if (params==NULL) return;
	struct ComponentParameter** ptr = params;
	while (*ptr!=NULL) {
		free((*ptr)->identifier);
		free((*ptr)->defaultValue);
		free((*ptr)->description);
		free(*ptr);
		ptr++;
	}
	free(params);
}

void setPreferedDataBlockSize(int size) {
	DataBlock::setPreferedBlockSize(size);
}

void setVerbose(int value) {
	verboseFlag = (value!=0);
}


