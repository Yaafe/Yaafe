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

#ifndef COMPONENT_HELPERS_H
#define COMPONENT_HELPERS_H

#include "Component.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

namespace YAAFE
{

template<class T>
class StateLessOneInOneOutComponent : public Component
{
	virtual const std::string getIdentifier() const  = 0;
	virtual const std::string getDescription() const { return "";}
    virtual bool stateLess() const { return true; };

    virtual Component* clone() const { return new T();}
	virtual int getRevision() const { return 0; }

	virtual ParameterDescriptorList getParameterDescriptorList() const { return ParameterDescriptorList();};

	virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
	virtual StreamInfo init(const ParameterMap& params, const StreamInfo& in) = 0;
	// void processToken(double* inData, const int inSize, double* out, const int outSize);
	virtual void reset() { /* nothing to do */ };
	virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);
	virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);

};

template<class T>
bool StateLessOneInOneOutComponent<T>::init(const ParameterMap& params, const Ports<StreamInfo>& inp) {
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;
	outStreamInfo().add(this->init(params,in));
	return true;
}

template<class T>
bool StateLessOneInOneOutComponent<T>::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp) {
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	const int N = in->info().size;
	const int P = out->info().size;
	while (!in->empty()) {
		static_cast<T*>(this)->processToken(in->readToken(),N,out->writeToken(),P);
		in->consumeToken();
	}
	return true;
}

template<class T>
void StateLessOneInOneOutComponent<T>::flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out) {
	process(in,out);
}

template<class T>
class TemporalFilter : public Component
{
public:
	TemporalFilter();
	virtual ~TemporalFilter();

	virtual const std::string getIdentifier() const  = 0;
	virtual const std::string getDescription() const { return "";}
    virtual bool stateLess() const { return false; };

	virtual Component* clone() const { return new T();}
	virtual int getRevision() const { return 0; }

	virtual ParameterDescriptorList getParameterDescriptorList() const { return ParameterDescriptorList();};

	virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& inp);
	virtual void reset();
	virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);
	virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);

protected:
	virtual bool initFilter(const ParameterMap& params, const StreamInfo& in) = 0;

	double* m_filter;
	int m_length;
	int m_delay;

private:
	int m_size;
	int m_pos;
	double* m_data;
};

template<class T>
TemporalFilter<T>::TemporalFilter() :
  m_filter(NULL), m_data(NULL)
{}

template<class T>
TemporalFilter<T>::~TemporalFilter() {
	if (m_filter)
		delete [] m_filter;
	if (m_data)
		delete [] m_data;
}

template<class T>
bool TemporalFilter<T>::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;
	m_size = in.size;

	m_length = -1;
	m_delay = -1;
	m_filter = NULL;
	if (!initFilter(params,in))
		return false;
	if (m_length<0) {
		std::cerr << "ERROR: initFilter must set m_length attribute !" << std::endl;
		return false;
	}
	if (m_delay<0) {
		std::cerr << "ERROR: initFilter must set m_delay attribute !" << std::endl;
		return false;
	}
	if (m_filter==NULL) {
		std::cerr << "ERROR: initFilter must allocate m_filter array :" << std::endl;
		return false;
	}
	m_data = new double[m_length*m_size];
	outStreamInfo().add(in);
	return true;
}

template<class T>
void TemporalFilter<T>::reset() {
	m_pos = 0;
	for (int i=0;i<m_length*m_size;i++)
		m_data[i] = 0.0;
}

template<class T>
bool TemporalFilter<T>::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp) {
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	while (!in->empty() && (in->tokenno()-out->tokenno()<m_delay))
	{
		const double* inPtr = in->readToken();
		for (int i=0;i<m_size;i++)
			m_data[i*m_length+m_pos] = inPtr[i];
		in->consumeToken();
		m_pos = ++m_pos % m_length;
	}

	while (!in->empty())
	{
		const double* inPtr = in->readToken();
		double* outPtr = out->writeToken();
		for (int i=0;i<m_size;i++)
		{
			double* data = m_data + i*m_length;
			data[m_pos] = inPtr[i];
			long double v = 0;
			for (int j=0, fj=m_length-m_pos-1;j<m_length;j++,fj++)
			{
				if (fj==m_length) fj=0;
				v += m_filter[fj]*data[j];
			}
			outPtr[i] = v;
		}
		in->consumeToken();
		m_pos = ++m_pos % m_length;
	}
	return true;
}

template<class T>
void TemporalFilter<T>::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	InputBuffer* in = inp[0].data;
	in->appendZeros(m_delay);
	process(inp,outp);
}


} // YAAFE

#endif
