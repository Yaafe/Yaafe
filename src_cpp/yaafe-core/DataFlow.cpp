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

#include "DataFlow.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include "DataFlowParserContext.h"

#ifdef __APPLE__
#include "fmemopen.h"
#endif

using namespace std;

extern "C" {
  void df_parser_restart(FILE* yyin);
}

extern "C++" {
  int df_parser_parse(class YAAFE::DataFlowContext& context);
}

namespace YAAFE
{

  void DataFlow::append(const DataFlow& df)
  {
    // merge library usage
    m_libs.insert(df.m_libs.begin(),df.m_libs.end());
    // append dataflow
    NodeList fNodes = Graph<NodeDesc>::finalNodes();
    const Graph<NodeDesc>& dfg = (const Graph<NodeDesc>&) df;
    NodeList rNodes = dfg.rootNodes();
    if (rNodes.size()==0)
    {
      cerr << "WARNING: appending empty dataflow ..." << endl;
      return;
    }
    if (fNodes.size()>1)
    {
      cerr << "ERROR: try to append a dataflow to a dataflow having " << fNodes.size() << " final nodes !" << endl;
      return;
    }
    map<Node*,Node*> mapping;
    for (NodeListCIt it=rNodes.begin();it!=rNodes.end();it++)
    {
      Node* n = Graph<NodeDesc>::createNode((*it)->v);
      if (fNodes.size()>0)
        Graph<NodeDesc>::link(fNodes[0],"",n,"");
      mapping[*it] = n;
    }
    mergeFlow(this,&dfg,mapping);
  }


  void DataFlow::merge(const DataFlow& df)
  {
    // merge library usage
    m_libs.insert(df.m_libs.begin(),df.m_libs.end());
    // merge flow
    map<Node*,Node*> mapping;
    mergeFlow(this,&df,mapping);
  }

  bool sources_match(const Graph<NodeDesc>::LinkList& a,
      const Graph<NodeDesc>::LinkList& b,
      const map<Graph<NodeDesc>::Node*,Graph<NodeDesc>::Node*>& mapping)
  {
    if (a.size()!=b.size())
      return false;
    for (Graph<NodeDesc>::LinkListCIt aIt=a.begin();aIt!=a.end();aIt++)
    {
      const Graph<NodeDesc>::Link* la = *aIt;
      map<Graph<NodeDesc>::Node*,Graph<NodeDesc>::Node*>::const_iterator mappedSourceIt = mapping.find(la->source);
      if (mappedSourceIt==mapping.end())
        return false;
      const Graph<NodeDesc>::Node* mappedSource = mappedSourceIt->second;
      bool found = false;
      for (Graph<NodeDesc>::LinkListCIt bIt=b.begin();bIt!=b.end();bIt++)
      {
        Graph<NodeDesc>::Link* lb = *bIt;
        if ((mappedSource==lb->source) && (la->sourceOutputPort==lb->sourceOutputPort)
            && (la->targetInputPort==lb->targetInputPort))
        {
          found = true;
          break;
        }
      }
      if (!found)
        return false;
    }
    return true;
  }


  void DataFlow::mergeFlow(Graph<NodeDesc>* f,const Graph<NodeDesc>* g, std::map<Node*,Node*>& mapping)
  {
    // mapping is g -> f
    NodeList queue = g->rootNodes();
    while (!queue.empty())
    {
      Node* n = queue.back();
      queue.pop_back();
      // check predecessors
      bool allSourcesMapped = true;
      for (LinkListCIt sIt=n->sources().begin();sIt!=n->sources().end();sIt++)
      {
        Link* sLink = *sIt;
        map<Node*,Node*>::iterator findIt=mapping.find(sLink->source);
        if (findIt==mapping.end())
        {
          allSourcesMapped = false;
          break;
        }
      }
      if (!allSourcesMapped)
      {
        // some sources miss. Ignore current node for now
        continue;
      }
      if (mapping.find(n)==mapping.end())
      {
        // merge node
        // all sources are merged, merge current node
        Node* mergedNode(NULL);
        // first check if identical node already exists in f
        NodeList candidates;
        if (n->sources().size()>0)
        {
          candidates = mapping.find(n->sources()[0]->source)->second->targetNodes();
        } else {
          candidates = f->rootNodes();
        }
        for (NodeListIt cIt=candidates.begin();cIt!=candidates.end();cIt++)
        {
          if ((**cIt==*n) && sources_match(n->sources(),(*cIt)->sources(),mapping))
          {
            // found identical node
            mergedNode = *cIt;
            break;
          }
        }
        if (mergedNode==NULL)
        {
          // no identical node found. Create node
          mergedNode = f->createNode(n->v);
          for (LinkListCIt sIt=n->sources().begin();sIt!=n->sources().end();sIt++)
          {
            f->link(mapping.find((*sIt)->source)->second,(*sIt)->sourceOutputPort, mergedNode, (*sIt)->targetInputPort);
          }
        }
        // register mapping
        mapping[n] = mergedNode;
      }
      // add target nodes to queue
      NodeList targets = n->targetNodes();
      queue.insert(queue.end(),targets.begin(),targets.end());
    }
    // merge names
    for (NameMapCIt gnIt=g->getNames().begin();gnIt!=g->getNames().end();gnIt++)
    {
      NameMapCIt fnIt = f->getNames().find(gnIt->first);
      if (fnIt!=f->getNames().end()) {
        // check nodes are merged
        if (mapping[gnIt->second]!=fnIt->second) {
          cerr << "ERROR: '" << gnIt->first << "' node exists in the two graphs and cannot be merged !" << endl;
        }
      } else {
        // add named node to f
        f->setNodeName(mapping[gnIt->second], gnIt->first);
      }
    }
  }


  bool DataFlow::load(const std::string& filename)
  {
    FILE* yyin = fopen(filename.c_str(),"r");
    if (yyin==NULL)
    {
      cerr << "cannot open file " << filename << endl;
      return false;
    }
    return yyparse(yyin);
  }

  bool DataFlow::loads(const std::string& df_str)
  {
    // fmemopen  http://www.delorie.com/gnu/docs/glibc/libc_228.html
    const char* buf = df_str.c_str();
    size_t buf_size = sizeof(char) * (df_str.size() + 1);
    FILE* yyin = fmemopen((void *)buf, buf_size, "r");
    if (yyin == NULL) {
      cerr << "cannot parse dataflow:" << endl << df_str << endl;
      return false;
    }
    return yyparse(yyin);
  }

  bool DataFlow::yyparse(FILE* yyin)
  {
    df_parser_restart(yyin);
    DataFlowContext context;
    context.m_dataflow = this;
    int iRc = df_parser_parse(context);
    fclose(yyin);
    return (iRc==0);
  }

  void DataFlow::save(const std::string& filename)
  {
    ofstream out(filename.c_str());
    if (!out.good())
    {
      cerr << "ERROR: cannot open file " << filename << "!" << endl;
      return;
    }
    print(out);
    out.close();
  }

  void printParams(ostream& out, const ParameterMap& params)
  {
    for (ParameterMap::const_iterator pit=params.begin();
        pit!=params.end(); pit++)
      out << " " << pit->first << "=\"" << pit->second << "\"";
  }

  void DataFlow::print(ostream& out)
  {
    for (set<string>::const_iterator libIt=m_libs.begin();libIt!=m_libs.end();libIt++)
      out << "useComponentLibrary " << *libIt << endl;
    map<Node*,std::string> mapping;
    for (NameMapCIt it=m_names.begin();it!=m_names.end();it++)
    {
      mapping[it->second] = it->first;
    }
    for (size_t i=0;i<m_nodes.size(); i++)
    {
      Node* node=m_nodes[i];
      map<Node*,std::string>::iterator nameIt = mapping.find(node);
      if (nameIt==mapping.end()) {
        ostringstream oss;
        oss << "n" << i;
        mapping[node] = oss.str();
      }
      out << mapping[node] << " := " << node->v.componentId;
      printParams(out,node->v.params);
      out << endl;
    }
    for (size_t i=0;i<m_links.size();i++)
    {
      Link* l=m_links[i];
      out << mapping[l->source];
      if (l->sourceOutputPort.size()>0)
        out << ":" << l->sourceOutputPort;
      out	<< " > " << mapping[l->target];
      if (l->targetInputPort.size()>0)
        out << ":" << l->targetInputPort;
      out << endl;
    }
  }

  void DataFlow::display()
  {
    cout << stringify() << endl;
  }

  const std::string DataFlow::stringify()
  {
    ostringstream oss;
    print(oss);
    return oss.str();
  }

  void DataFlow::dumpdot(const std::string& filename)
  {
    ofstream out(filename.c_str());
    if (!out.good())
    {
      cerr << "ERROR: cannot open file " << filename << "!" << endl;
      return;
    }

    out << "digraph \"yaafe dataflow\" {" << endl;
    out << "  node [shape=box];" << endl;
    map<Node*,int> mapping;
    for (size_t i=0;i<m_nodes.size(); i++)
    {
      Node* node=m_nodes[i];
      mapping[node] = i;
      out << "n" << i << " [label=\"" << node->v.componentId;
      for (ParameterMap::const_iterator it=node->v.params.begin();
          it!=node->v.params.end(); it++)
        out << " " << it->first << "=\\\"" << it->second << "\\\"";
      out << "\"];" << endl;
    }
    for (size_t i=0;i<m_links.size();i++)
    {
      Link* l=m_links[i];
      out << "n" << mapping[l->source] << " -> n" << mapping[l->target];
      if ((l->sourceOutputPort.size()>0) || (l->targetInputPort.size()>0))
        out << " [label=\"'" << l->sourceOutputPort << "'>'" << l->targetInputPort << "'\"]";
      out << ";" << endl;
    }
    out << "}" << endl;
  }

  void DataFlow::useComponentLibrary(const std::string& lib)
  {
    m_libs.insert(lib);
  }

  const std::set<std::string>& DataFlow::getComponentLibraries() const
  {
    return m_libs;
  }

} // namespace YAAFE

