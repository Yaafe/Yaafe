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

#ifndef DAFFY_PARSER_CONTEXT_H
#define DAFFY_PARSER_CONTEXT_H

#include "DataFlow.h"
#include <map>
#include <string>
#include "utils.h"

namespace YAAFE {

class DataFlowContext {
public:
  DataFlowContext();
  ~DataFlowContext();

  DataFlow* m_dataflow;
  std::map<std::string,Graph<NodeDesc>::Node*> m_identifiers;
  ParameterMap m_params;

};

}

#endif
