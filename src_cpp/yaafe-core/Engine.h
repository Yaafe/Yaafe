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

#ifndef ENGINE_H_
#define ENGINE_H_

#include "DataFlow.h"
#include "Component.h"
#include "ComponentPool.h"
#include "DirectedGraph.h"

namespace YAAFE
{

class Engine
{
public:
    Engine();
    virtual ~Engine();

    bool load(const DataFlow& df);

    OutputBuffer* getInput(const std::string& id);
    ParameterMap getInputParams(const std::string& id);
    bool bindInput(const std::string& id, Component* component);
    void detachInput(const std::string& id);
    std::vector<std::string> getInputs();

    InputBuffer* getOutput(const std::string& id);
    ParameterMap getOutputParams(const std::string& id);
    void bindOutput(const std::string& id, Component* component);
    void detachOutput(const std::string& id);
    std::vector<std::string> getOutputs();

    void reset();
    bool process();
    void flush();

private:
	ComponentPool m_pool;

	class ProcessingStep {
	public:
		ProcessingStep();
		~ProcessingStep();
		std::string m_id;
		ParameterMap m_params;
		Component* m_component;
		ComponentPool* m_pool;
		Ports<InputBuffer*> m_input;
		Ports<OutputBuffer*> m_output;

		bool hasInputAvailable() const;
	};

	typedef Graph<ProcessingStep> ProcessFlow;
	ProcessFlow* m_graph;
	ProcessFlow::NodeList m_startNodes;

	static inline bool initStep(ProcessFlow::Node& step);
	static inline bool resetStep(ProcessFlow::Node& step);
	static inline bool processStep(ProcessFlow::Node& step);
	static inline bool flushStep(ProcessFlow::Node& step);

	ProcessingStep* getInputNode(const std::string& id);
	ProcessingStep* getOutputNode(const std::string& id);

};

}

#endif /* ENGINE_H_ */
