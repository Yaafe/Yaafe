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

#ifndef DATAFLOW_H_
#define DATAFLOW_H_

#include "DirectedGraph.h"
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <set>

namespace YAAFE
{

  struct NodeDesc {
    std::string componentId;
    std::map<std::string,std::string> params;

    NodeDesc(const std::string& id, const std::map<std::string,std::string>& p) :
      componentId(id), params(p) {};
    NodeDesc(const NodeDesc& a) :
      componentId(a.componentId), params(a.params) {};
    bool operator==(const NodeDesc& nd) const {
      return ((componentId==nd.componentId) && (params==nd.params));
    }
    //  NodeDesc& operator=(const NodeDesc& d) {
    //    componentId = d.componentId;
    //    params = d.params;
    //    return *this;
    //  }
  };

  class DataFlow : public Graph<NodeDesc>
  {
    friend class Engine;
   public:
    /**
     * convenient function to append a dataflow to an existing one.
     * The dataFlow object must have at most one final node and given
     * dataflow to append must have only one root node.
     */
    void append(const DataFlow& df);

    void merge(const DataFlow& df);

    bool load(const std::string& filename);
    bool loads(const std::string& df_str);
    void save(const std::string& filename);
    const std::string stringify();
    void dumpdot(const std::string& filename);
    void display();

    void useComponentLibrary(const std::string& lib);
    const std::set<std::string>& getComponentLibraries() const;

   private:
    static void mergeFlow(Graph<NodeDesc>* f,const Graph<NodeDesc>* g, std::map<Node*,Node*>& mapping);
    void print(std::ostream& out);
    bool yyparse(FILE*);

    std::set<std::string> m_libs;
  };

}

#endif /* DATAFLOW_H_ */
