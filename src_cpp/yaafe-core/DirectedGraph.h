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

#ifndef DIRECTEDGRAPH_H_
#define DIRECTEDGRAPH_H_

#include <vector>
#include <map>
#include <iostream>
#include "Ports.h"

namespace YAAFE {

template<class T>
class Graph {
public:

	class Node;

	typedef typename std::vector<Node*> NodeList;
	typedef typename std::vector<Node*>::iterator NodeListIt;
	typedef typename std::vector<Node*>::const_iterator NodeListCIt;

	class Link;

	typedef typename std::vector<Link*> LinkList;
	typedef typename std::vector<Link*>::iterator LinkListIt;
	typedef typename std::vector<Link*>::const_iterator LinkListCIt;

	typedef typename std::map<std::string,Node*> NameMap;
	typedef typename std::map<std::string,Node*>::iterator NameMapIt;
	typedef typename std::map<std::string,Node*>::const_iterator NameMapCIt;

	class Node {
		friend class Graph<T>;
	public:
		~Node() {};
		T v;
		bool operator==(const Graph<T>::Node& b) const { return (v==b.v); };
		const LinkList& targets() const { return m_targets; };
		const LinkList& sources() const { return m_sources; };
		NodeList targetNodes() const {
			NodeList res;
			for (LinkListCIt it=m_targets.begin();it!=m_targets.end();it++)
				res.push_back((*it)->target);
			return res;
		}
		NodeList sourceNodes() const {
			NodeList res;
			for (LinkListCIt it=m_sources.begin();it!=m_sources.end();it++)
				res.push_back((*it)->source);
			return res;
		}
	private:
		Node() : v() {};
		Node(const T& value) : v(value) {};
		LinkList m_sources;
		LinkList m_targets;
	};

	class Link {
		friend class Graph<T>;
	public:
		~Link() {};
		Node* source;
		std::string sourceOutputPort;
		Node* target;
		std::string targetInputPort;
		bool operator==(const Graph<T>::Link& b) const;
	private:
		Link(Node* s, const std::string& srcOutPort,
				Node* t, const std::string& tgtInPort)
		: source(s), sourceOutputPort(srcOutPort), target(t), targetInputPort(tgtInPort) {};
	};

	Graph();
	~Graph();

	Node* createNode();
	Node* createNode(const T& value);
	void setNodeName(Node* node, const std::string& name);
	Node* getNode(const std::string& name);
	const Node* getNode(const std::string& name) const;
	NodeList rootNodes() const;
	NodeList finalNodes() const;
	void link(Node* source, const std::string& sourcePort,
			Node* target, const std::string& targetPort);
	const NodeList& getNodes() const { return m_nodes; }
	const LinkList& getLinks() const { return m_links; }
	const NameMap& getNames() const { return m_names; }

	template<bool fn(Node& node)>
	bool visitAll();

protected:
	NodeList m_nodes;
	LinkList m_links;
	NameMap m_names;
};

template<class T>
Graph<T>::Graph()
{
}

template<class T>
Graph<T>::~Graph()
{
	for (NodeListIt it=m_nodes.begin();it!=m_nodes.end();it++)
		delete *it;
	m_nodes.clear();
	for (LinkListIt it=m_links.begin();it!=m_links.end();it++)
		delete *it;
	m_links.clear();
	// TODO delete inputs and outputs
}

template<class T>
inline bool Graph<T>::Link::operator==(const Graph<T>::Link& b) const
{
	return ((source==b.source) && (sourceOutputPort==b.sourceOutputPort) &&
			(target==b.target) && (targetInputPort==b.targetInputPort));
}

template<class T>
typename Graph<T>::Node* Graph<T>::createNode()
{
	Node* n = new Node();
	m_nodes.push_back(n);
	return n;
}

template<class T>
typename Graph<T>::Node* Graph<T>::createNode(const T& value)
{
	Node* n = new Node(value);
	m_nodes.push_back(n);
	return n;
}

template<class T>
void Graph<T>::setNodeName(Node* node, const std::string& name)
{
	NameMapCIt it = m_names.find(name);
	if (it!=m_names.end()) {
		std::cerr << "WARNING: overrides already existing node '" << name << "' !" << std::endl;
	}
	m_names[name] = node;
}

template<class T>
typename Graph<T>::Node* Graph<T>::getNode(const std::string& name) {
	NameMapIt it = m_names.find(name);
	if (it==m_names.end())
		return NULL;
	return it->second;
}

template<class T>
const typename Graph<T>::Node* Graph<T>::getNode(const std::string& name) const {
	NameMapCIt it = m_names.find(name);
	if (it==m_names.end())
		return NULL;
	return it->second;
}

template<class T>
typename Graph<T>::NodeList Graph<T>::rootNodes() const {
	NodeList res;
	for (NodeListCIt it=m_nodes.begin();
			it!=m_nodes.end(); it++)
	{
		if ((*it)->m_sources.size()==0)
			res.push_back(*it);
	}
	return res;
}

template<class T>
typename Graph<T>::NodeList Graph<T>::finalNodes() const {
	NodeList res;
	for (NodeListCIt it=m_nodes.begin();
			it!=m_nodes.end(); it++)
	{
		if ((*it)->m_targets.size()==0)
			res.push_back(*it);
	}
	return res;
}

template<class T>
void Graph<T>::link(Node* source, const std::string& sourcePort,
		Node* target, const std::string& targetPort) {
	m_links.push_back(new Link(source,sourcePort,target,targetPort));
	source->m_targets.push_back(m_links.back());
	target->m_sources.push_back(m_links.back());
}

template<class T>
template<bool fn(typename Graph<T>::Node& node)>
bool Graph<T>::visitAll() {
	std::map<Node*,int> count;
	for (NodeListIt it=m_nodes.begin();
			it!=m_nodes.end(); it++)
		count[*it] = 0;
	typename std::map<Node*,int>::iterator findIt;
	NodeList toVisit = rootNodes();
	while (toVisit.size()>0) {
		Node* n = toVisit.back();
		toVisit.pop_back();
		findIt = count.find(n);
		findIt->second++;
		if (findIt->second>=n->sources().size())
		{
			if (!fn(*n))
				return false;
			for (LinkListCIt lIt=n->targets().begin();lIt!=n->targets().end();lIt++)
				toVisit.push_back((*lIt)->target);
		}
	}
	return true;
}


}


#endif /* DIRECTEDGRAPH_H_ */
