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

#ifndef YAAFECOREDATAFLOW_H_
#define YAAFECOREDATAFLOW_H_

extern "C" {

void* dataflow_create();
void dataflow_destroy(void* dataflow);

int dataflow_load(void* dataflow, char* filename);
void dataflow_save(void* dataflow, char* filename);
void dataflow_dumpdot(void* dataflow, char* filename);
void dataflow_display(void* dataflow);

void* dataflow_createNode(void* dataflow, char* componentId, char** params);
void dataflow_setNodeName(void* dataflow, void* node, char* name);
void* dataflow_getNode(void* dataflow, char* name);

const char* dataflow_node_getId(void* node);
const char** dataflow_node_getParams(void* node);
void dataflow_node_freeParams(const char** params);
void** dataflow_node_sources(void* node);
void** dataflow_node_targets(void* node);

void** dataflow_getNodes(void* dataflow);
void** dataflow_rootNodes(void* dataflow);
void** dataflow_finalNodes(void* dataflow);

void dataflow_freeNodeList(void** nodelist);

void dataflow_link(void* dataflow, void* source, char* sourceport, void* target, char* targetport);

void dataflow_append(void* dataflow,void* otherdataflow);
void dataflow_merge(void* dataflow,void* otherdataflow);

void dataflow_useComponentLibrary(void* dataflow, char* lib);
const char** dataflow_getComponentLibraries(void* dataflow);
void dataflow_freeComponentLibraries(const char** libs);

}


#endif /* YAAFECOREDATAFLOW_H_ */
