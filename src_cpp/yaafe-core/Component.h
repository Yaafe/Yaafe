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

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "Buffer.h"
#include "Ports.h"
#include "utils.h"
#include "DataFlow.h"

#include <vector>
#include <map>

namespace YAAFE {

/**
 * The Component class represent a computation step. The 4 main methods are
 * init, reset, process, flush. Theses methods are called according to this order:
 *
 * 1. init(...) always called first and only once
 * 2. reset() once
 * 3. process() called as many times as necessary
 * 4. flush() once
 * 5. If there is another data stream to process, go to 2.
 */
class Component {

/**
 * Specializable methods
 */
public:
	Component();
	virtual ~Component();

	/**
	 * Returns the component's identifier
	 */
	virtual const std::string getIdentifier() const = 0;

	/**
	 * Returns a description
	 */
	virtual const std::string getDescription() const { return "";}

	/**
	 * Return a new instance of the same component. (Prototype design pattern)
	 */
	virtual Component* clone() const = 0;

	/**
	 * A component is stateless if it doesn't hold any information between sucessive calls
	 * to init, process and flush methods. If so, it can be used to process several
	 * data streams at the same time.
	 */
	virtual bool stateLess() const { return false; }

	/**
	 * Returns the list of acceptable parameters
	 */
	virtual ParameterDescriptorList getParameterDescriptorList() const;

	/**
	 * Initialize the component with parameters and input data stream infos.
	 */
	virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in) = 0;

	/**
	 * Reset the component state so that a new data stream can be processed.
	 */
	virtual void reset() = 0;

	/**
	 * Process available data from input data streams, and write to output data streams.
	 */
	virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out) = 0;

	/**
	 * Process remaining data from input streams, write all data to output streams.
	 */
	virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out) = 0;

/**
 * Internal Methods
 */
public:
	const Ports<StreamInfo>& getOutStreamInfo() const;

protected:
	std::string getStringParam(const std::string& id, const ParameterMap& params);
	int getIntParam(const std::string& id, const ParameterMap& params);
	double getDoubleParam(const std::string& id, const ParameterMap& params);

	Ports<StreamInfo>& outStreamInfo();

private:
	Ports<StreamInfo> m_outInfo;
};


inline const Ports<StreamInfo>& Component::getOutStreamInfo() const
{
	return m_outInfo;
}

inline Ports<StreamInfo>& Component::outStreamInfo()
{
	return m_outInfo;
}

template<class T>
class ComponentBase : public Component {
public:
	virtual const std::string getIdentifier() const  = 0;
	virtual const std::string getDescription() const { return "";}

	virtual Component* clone() const { return new T();}
	virtual int getRevision() const { return 0; }

	virtual ParameterDescriptorList getParameterDescriptorList() const { return ParameterDescriptorList();};

	virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in) = 0;
	virtual void reset() {/* nothing to do */};
	virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out) = 0;
	virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out) { process(in,out);};
};

}

#endif /* COMPONENT_H_ */
