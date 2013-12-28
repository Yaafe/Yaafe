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

#include "yaafecoreDataFlow.h"
#include "yaafe-core/DataFlow.h"
#include "yaafe-core/utils.h"
#include "stdlib.h"
#include "string.h"

using namespace YAAFE;
using namespace std;

void* dataflow_create() {
	return static_cast<void*>(new DataFlow());
}

void dataflow_destroy(void* df) {
	DataFlow* ptr = static_cast<DataFlow*>(df);
	delete ptr;
}

int dataflow_load(void* dataflow, char* filename)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	return (df->load(filename) ? 1 : 0);
}

void dataflow_save(void* dataflow, char* filename)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	df->save(filename);
}

void dataflow_dumpdot(void* dataflow, char* filename)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	df->dumpdot(filename);
}

void dataflow_display(void* dataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	df->display();
}

void* dataflow_createNode(void* dataflow, char* componentId, char** parameters)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	ParameterMap map;
	char** ptr = parameters;
	while (*ptr!=NULL) {
		map[ptr[0]] = ptr[1];
		ptr += 2;
	}
	return df->createNode(NodeDesc(componentId,map));
}

void dataflow_setNodeName(void* dataflow, void* node, char* name)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	DataFlow::Node* n = static_cast<DataFlow::Node*>(node);
	df->setNodeName(n,name);
}

void* dataflow_getNode(void* dataflow, char* name)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	return df->getNode(name);
}

const char* dataflow_node_getId(void* node)
{
	DataFlow::Node* n = static_cast<DataFlow::Node*>(node);
	return n->v.componentId.c_str();
}

const char** dataflow_node_getParams(void* node)
{
	DataFlow::Node* n = static_cast<DataFlow::Node*>(node);
	ParameterMap& params = n->v.params;
	const char** res = (const char**) malloc(((params.size()*2)+1)*sizeof(char*));
	const char** ptr = res;
	for (ParameterMap::const_iterator it=params.begin();it!=params.end();it++)
	{
		ptr[0] = it->first.c_str();
		ptr[1] = it->second.c_str();
		ptr+=2;
	}
	*ptr = NULL;
	return res;
}

void dataflow_node_freeParams(const char** params)
{
	free(params);
}

void** buildNodeList(const DataFlow::NodeList& nodes) {
	void** res = (void**) malloc((nodes.size()+1)*sizeof(void*));
	for (int i=0;i<nodes.size();i++)
		res[i] = nodes[i];
	res[nodes.size()] = NULL;
	return res;
}

void** dataflow_node_sources(void* node)
{
	DataFlow::Node* n = static_cast<DataFlow::Node*>(node);
	return buildNodeList(n->sourceNodes());
}

void** dataflow_node_targets(void* node)
{
	DataFlow::Node* n = static_cast<DataFlow::Node*>(node);
	return buildNodeList(n->targetNodes());
}

void** dataflow_getNodes(void* dataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	return buildNodeList(df->getNodes());
}

void** dataflow_rootNodes(void* dataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	return buildNodeList(df->rootNodes());
}

void** dataflow_finalNodes(void* dataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	return buildNodeList(df->finalNodes());
}


void dataflow_freeNodeList(void** nodelist)
{
	free(nodelist);
}

void dataflow_link(void* dataflow, void* source, char* sourceport, void* target, char* targetport)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	DataFlow::Node* sourceNode = static_cast<DataFlow::Node*>(source);
	DataFlow::Node* targetNode = static_cast<DataFlow::Node*>(target);
	df->link(sourceNode,sourceport,targetNode,targetport);
}

void dataflow_append(void* dataflow,void* otherdataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	DataFlow* df2 = static_cast<DataFlow*>(otherdataflow);
	df->append(*df2);
}

void dataflow_merge(void* dataflow,void* otherdataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	DataFlow* df2 = static_cast<DataFlow*>(otherdataflow);
	df->merge(*df2);
}

void dataflow_useComponentLibrary(void* dataflow, char* lib)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	df->useComponentLibrary(lib);
}

const char** dataflow_getComponentLibraries(void* dataflow)
{
	DataFlow* df = static_cast<DataFlow*>(dataflow);
	const set<string>& libs = df->getComponentLibraries();
	const char** res = (const char**) malloc((libs.size()+1)*sizeof(const char*));
	const char** ptr = res;
	for (set<string>::const_iterator it=libs.begin();it!=libs.end();it++)
		*ptr++ = it->c_str();
	*ptr = NULL;
	return res;
}

void dataflow_freeComponentLibraries(const char** libs)
{
	free(libs);
}


