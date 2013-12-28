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

#include "Engine.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "ComponentPool.h"
#include "DirectedGraph.h"
#include "utils.h"

#include <stack>
#include <vector>
#include <map>
#include <time.h>
#include <iostream>
#include <algorithm>

using namespace std;

namespace YAAFE {

  Engine::ProcessingStep::ProcessingStep() :
    m_id(), m_params(), m_component(NULL), m_pool(NULL), m_input(), m_output() {
    }

  Engine::ProcessingStep::~ProcessingStep() {
    if (m_pool)
      m_pool->release(m_component);
    for (int i=0;i<m_output.size();++i)
      delete m_output[i].data;
    for (int i=0;i<m_input.size();++i)
      delete m_input[i].data;
  }

  inline bool Engine::ProcessingStep::hasInputAvailable() const
  {
    for (int i=0;i<m_input.size();i++)
      if (!m_input[i].data->empty())
        return true;
    return false;
  }

  Engine::Engine() :
    m_graph(NULL) {
      m_graph = new Graph<ProcessingStep>; // initialize with empty graph
    }

  Engine::~Engine() {
    if (m_graph)
      delete m_graph;
  }

  bool Engine::load(const DataFlow& df) {
    // release old dataflow
    if (m_graph)
      delete m_graph;
    m_graph = new Graph<ProcessingStep>;

    // load component libraries
    for (set<string>::const_iterator libIt=df.getComponentLibraries().begin();
        libIt!=df.getComponentLibraries().end(); libIt++)
    {
      if (ComponentFactory::instance()->loadLibrary(*libIt)!=0)
        return false;
    }

#ifdef WITH_TIMERS
    static Timer* gt = Timer::get_timer("loading dataflow");
    gt->start();
#endif

    const DataFlow::NodeList& nodes = df.getNodes();

    // create processing steps
    map<DataFlow::Node*,ProcessFlow::Node*> mapping;
    for (DataFlow::NodeList::const_iterator nodeIt = nodes.begin(); nodeIt
        != nodes.end(); nodeIt++) {
      DataFlow::Node* n = *nodeIt;
      ProcessFlow::Node* s = m_graph->createNode();
      if ((n->v.componentId!="Input") && (n->v.componentId!="Output"))
        s->v.m_pool = &m_pool;
      s->v.m_id = n->v.componentId;
      s->v.m_params = n->v.params;
      mapping[n] = s;
      if (verboseFlag)
        cout << "create step for component " << s->v.m_id << endl;
    }

    // set names
    for (DataFlow::NameMapCIt nameIt=df.getNames().begin();
        nameIt!=df.getNames().end();nameIt++)
    {
      m_graph->setNodeName(mapping[nameIt->second],nameIt->first);
    }

    // create links
    const DataFlow::LinkList& links = df.getLinks();
    for (DataFlow::LinkListCIt it=links.begin();it!=links.end();it++)
    {
      const DataFlow::Link* l = *it;
      m_graph->link(mapping[l->source],l->sourceOutputPort,mapping[l->target],l->targetInputPort);
    }
    // initialize components in order
    bool initOK =  m_graph->visitAll<Engine::initStep>();

#ifdef WITH_TIMERS
    gt->stop();
#endif

    // set start nodes
    m_startNodes = m_graph->rootNodes();

    return initOK;
  }

  Engine::ProcessingStep* Engine::getInputNode(const std::string& id)
  {
    ProcessFlow::Node* n = m_graph->getNode(id);
    if (n==NULL) {
      cerr << "WARNING: node '" << id << "' does not exist !" << endl;
      return NULL;
    }
    if (n->v.m_id!="Input")
    {
      cerr << "WARNING: node '" << id << "' is not an input node !" << endl;
      return NULL;
    }
    assert(n->v.m_input.size()==0);
    assert(n->v.m_output.size()==1);
    return &(n->v);
  }

  Engine::ProcessingStep* Engine::getOutputNode(const std::string& id)
  {
    ProcessFlow::Node* n = m_graph->getNode(id);
    if (n==NULL)
      return NULL;
    if (n->v.m_id!="Output")
    {
      cerr << "WARNING: node '" << id << "' is not an output node !" << endl;
      return NULL;
    }
    assert(n->v.m_input.size()==1);
    assert(n->v.m_output.size()==0);
    return &(n->v);
  }

  OutputBuffer* Engine::getInput(const std::string& id)
  {
    ProcessingStep* ps = getInputNode(id);
    if (!ps) return NULL;
    return ps->m_output[0].data;
  }

  ParameterMap Engine::getInputParams(const std::string& id)
  {
    ProcessingStep* ps = getInputNode(id);
    if (!ps) return ParameterMap();
    return ps->m_params;
  }

  bool Engine::bindInput(const std::string& id, Component* component) {
    ProcessingStep* ps = getInputNode(id);
    if (!ps)
      return false;
    const Ports<StreamInfo>& outInfo = component->getOutStreamInfo();
    if (outInfo.size()!=1)
    {
      cerr << "ERROR: cannot bind input " << id << " to component with " << outInfo.size() << " output ports" << endl;
      return false;
    }
    if (!(ps->m_output[0].data->info()==outInfo[0].data)) {
      cerr << "ERROR: StreamInfo mismatch while binding input " << id << endl;
      return false;
    }
    ps->m_component = component;
    return true;
  }

  void Engine::detachInput(const std::string& id)
  {
    ProcessingStep* ps = getInputNode(id);
    if (ps) {
      ps->m_component = NULL;
    }
  }

  std::vector<std::string> Engine::getInputs() {
    std::vector<std::string> res;
    const ProcessFlow::NameMap& names = m_graph->getNames();
    for (ProcessFlow::NameMapCIt it=names.begin();it!=names.end();it++)
      if (it->second->v.m_id=="Input")
        res.push_back(it->first);
    return res;
  }

  InputBuffer* Engine::getOutput(const std::string& id)
  {
    ProcessingStep* ps = getOutputNode(id);
    if (!ps) return NULL;
    return ps->m_input[0].data;
  }

  ParameterMap Engine::getOutputParams(const std::string& id)
  {
    ProcessingStep* ps = getOutputNode(id);
    if (!ps) return ParameterMap();
    return ps->m_params;
  }

  void Engine::bindOutput(const std::string& id, Component* component)
  {
    ProcessingStep* ps = getOutputNode(id);
    if (ps) {
      ps->m_component = component;
    }
  }

  void Engine::detachOutput(const std::string& id)
  {
    ProcessingStep* ps = getOutputNode(id);
    if (ps) {
      ps->m_component = NULL;
    }
  }

  std::vector<std::string> Engine::getOutputs() {
    std::vector<std::string> res;
    const ProcessFlow::NameMap& names = m_graph->getNames();
    for (ProcessFlow::NameMapCIt it=names.begin();it!=names.end();it++)
      if (it->second->v.m_id=="Output")
        res.push_back(it->first);
    return res;
  }

  inline bool Engine::initStep(ProcessFlow::Node& node) {
    if (node.v.m_id == "Input") {
      node.v.m_component = NULL;
      StreamInfo outStreamInfo;
      outStreamInfo.size = 1;
      outStreamInfo.sampleStep = 1;
      outStreamInfo.frameLength = 1;
      // get samplerate
      ParameterMap::const_iterator it = node.v.m_params.find("SampleRate");
      if (it==node.v.m_params.end()) {
        cerr << "WARNING: no SampleRate parameter specified for input !" << endl;
        outStreamInfo.sampleRate = 44100;
      } else {
        outStreamInfo.sampleRate = atof(it->second.c_str());
      }
      node.v.m_output.add(new OutputBuffer(outStreamInfo));
      return true;
    }
    Ports<StreamInfo> inStreamInfo;
    for (ProcessFlow::LinkListCIt it=node.sources().begin();it!=node.sources().end();it++)
    {
      const ProcessFlow::Link* l = *it;
      const Ports<OutputBuffer*>& outInfo = l->source->v.m_output;
      //		if (outInfo.size()!=1) {
      //			cerr << "ERROR: node " << node.v.m_id << " has multiple output stream info !" << endl;
      //			return false;
      //		}
      inStreamInfo.add(l->targetInputPort, outInfo[l->sourceOutputPort].data->info());
    }
    if (node.v.m_id == "Output") {
      node.v.m_component = NULL;
    } else {
      node.v.m_component = node.v.m_pool->get(node.v.m_id,node.v.m_params,inStreamInfo);
      if (!node.v.m_component) {
        cerr << "ERROR: cannot initialize component " << node.v.m_id << " !" << endl;
        return false;
      }
    }
    for (int i=0;i<node.sources().size();i++)
    {
      const ProcessFlow::Link* l = node.sources()[i];
      InputBuffer* source = new InputBuffer(l->source->v.m_output[l->sourceOutputPort].data->info());
      l->source->v.m_output[l->sourceOutputPort].data->bindInputBuffer(source);
      node.v.m_input.add(l->targetInputPort, source);
    }
    if (node.v.m_id != "Output") {
      node.v.m_output = node.v.m_component->getOutStreamInfo().map(buildOutputBufferFromInfo);
    }
    return true;
  }

  inline bool Engine::resetStep(ProcessFlow::Node& step) {
    for (int i=0;i<step.v.m_input.size();++i)
      step.v.m_input[i].data->clear();
    for (int i=0;i<step.v.m_output.size();++i)
      step.v.m_output[i].data->clear();
    if ((step.v.m_id=="Input") || (step.v.m_id=="Output"))
      step.v.m_component = NULL;
    if (step.v.m_component != NULL)
      step.v.m_component->reset();
    return true;
  }

  inline bool Engine::processStep(ProcessFlow::Node& step) {
#ifdef DEBUG
    if (verboseFlag)
      cerr << "process step " << step.v.m_id << endl;
#endif
    if (!step.v.hasInputAvailable())
      return false;
#ifdef WITH_TIMERS
    Timer* t = Timer::get_timer(step.v.m_id);
    t->start();
    bool res = step.v.m_component->process(step.v.m_input, step.v.m_output);
    t->stop();
    return res;
#else
    return (step.v.m_component->process(step.v.m_input, step.v.m_output));
#endif
  }

  inline bool Engine::flushStep(ProcessFlow::Node& step) {
#ifdef DEBUG
    if (verboseFlag)
      cerr << "flush step " << step.v.m_component->getIdentifier() << endl;
#endif
#ifdef WITH_TIMERS
    Timer* t = Timer::get_timer(step.v.m_component->getIdentifier());
    t->start();
#endif
    if (step.v.m_component!=NULL)
      step.v.m_component->flush(step.v.m_input, step.v.m_output);
#ifdef WITH_TIMERS
    t->stop();
#endif
    for (int i=0;i<step.v.m_output.size();i++)
      step.v.m_output[i].data->flush();
    return true;
  }

  void Engine::reset() {
    m_graph->visitAll<Engine::resetStep>();
  }

  bool Engine::process() {
#ifdef WITH_TIMERS
    static Timer* gt = Timer::get_timer("processing");
    gt->start();
#endif

    // process data
    ProcessFlow::NodeList toProcess = m_startNodes;
    bool doneSomething = false;
    while (toProcess.size()>0)
    {
      ProcessFlow::Node* n = toProcess.back();
      toProcess.pop_back();
      ProcessingStep& step = n->v;
#ifdef DEBUG
      if (verboseFlag)
        cerr << "process step " << step.m_id << " ( " << toProcess.size() << " in queue)" << endl;
#endif
      bool b = true;
      if (step.m_component!=NULL)
      {
#ifdef WITH_TIMERS
        Timer* t = Timer::get_timer(step.m_id);
        t->start();
#endif
        b = step.m_component->process(step.m_input,step.m_output);
#ifdef WITH_TIMERS
        t->stop();
#endif
      }
      if (b)
      {
        if (step.m_component!=NULL)
          doneSomething = true;
        for (int i=0;i<n->v.m_output.size();i++) {
          //					n->v.m_output[i].data->debug();
          n->v.m_output[i].data->dispatch();
        }
        for (ProcessFlow::LinkListCIt it=n->targets().begin(); it!=n->targets().end(); it++)
          if ((*it)->target->v.hasInputAvailable())
            toProcess.push_back((*it)->target);
      }
    }
#ifdef WITH_TIMERS
    gt->stop();
#endif

    return doneSomething;
  }

  void Engine::flush() {
    m_graph->visitAll<Engine::flushStep> ();
  }


}
