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

#ifndef PORTS_H_
#define PORTS_H_

#include <string>
#include <vector>
#include <iostream>

namespace YAAFE {

template<class T>
struct Port {
	Port(const std::string& n, const T& d) : name(n), data(d) {};
	Port(const std::string& n) : name(n), data() {};
	bool operator==(const Port<T>& p) const { return ((name==p.name) && (data==p.data)); }
	std::string name;
	T data;
};

template<class T>
class Ports : public std::vector<Port<T> > {
public:
	Ports() {};
	Ports(const T& val) { add(val);};
	void add(const T& val);
	void add(const std::string& name, const T& val);
	const Port<T>& operator[](int i) const;
	const Port<T>& operator[](const std::string& name) const;
	Port<T>& operator[](int i);
	Port<T>& operator[](const std::string& name);

	template<class O>
	Ports<O> map(O fn(const T&)) const;
};

template<class T>
void Ports<T>::add(const T& val)
{
	this->push_back(Port<T>("",val));
}

template<class T>
void Ports<T>::add(const std::string& name, const T& val)
{
	this->push_back(Port<T>(name,val));
}

template<class T>
const Port<T>& Ports<T>::operator[](int i) const
{
	return std::vector<Port<T> >::at(i);
}

template<class T>
const Port<T>& Ports<T>::operator[](const std::string& n) const
{
	for (typename std::vector<Port<T> >::const_iterator it = std::vector<Port<T> >::begin();
			it!= std::vector<Port<T> >::end(); ++it)
	{
		if (it->name==n)
			return *it;
	}
	std::cerr << "ERROR: unknown port " << n << " !" << std::endl;
	return this->at(0);
}

template<class T>
Port<T>& Ports<T>::operator[](int i)
{
	return std::vector<Port<T> >::at(i);
}

template<class T>
Port<T>& Ports<T>::operator[](const std::string& n)
{
	for (typename std::vector<Port<T> >::iterator it = std::vector<Port<T> >::begin();
			it!= std::vector<Port<T> >::end(); ++it)
	{
		if (it->name==n)
			return *it;
	}
	this->push_back(Port<T>(n));
	return this->back();
}

template<class T>
template<class O>
Ports<O> Ports<T>::map(O fn(const T&)) const
{
	Ports<O> out;
	for (typename std::vector<Port<T> >::const_iterator it = std::vector<Port<T> >::begin();
			it!= std::vector<Port<T> >::end(); ++it)
	{
		out.add(it->name,fn(it->data));
	}
	return out;
}

} // namespace YAAFE

#endif /* PORTS_H_ */
