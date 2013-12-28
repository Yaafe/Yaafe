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

#include "ComponentPool.h"
#include "ComponentFactory.h"

using namespace std;

namespace YAAFE {


  class ComponentPool::ComponentProxy {
    friend class ComponentPool;
   public:
    ComponentProxy(const ParameterMap& params, const Ports<StreamInfo>& in, Component* component);
    ~ComponentProxy();

    bool usable(const ParameterMap& params, const Ports<StreamInfo>& in) const;

   private:
    ParameterMap m_params;
    Ports<StreamInfo> m_in;
    int m_used;
    Component* m_component;
  };

  ComponentPool::ComponentProxy::ComponentProxy(const ParameterMap& params, const Ports<StreamInfo>& in, Component* component) :
    m_params(params), m_in(in), m_used(0), m_component(component)
  {};

  ComponentPool::ComponentProxy::~ComponentProxy()
  {
    delete m_component;
  }

  bool ComponentPool::ComponentProxy::usable(const ParameterMap& params, const Ports<StreamInfo>& in) const
  {
    return (m_component->stateLess() && (m_params==params) && (m_in==in));
  }


  ComponentPool::ComponentPool() {
  }

  ComponentPool::~ComponentPool() {
    for (PoolType::iterator it=m_pool.begin();it!=m_pool.end();it++)
    {
      if (it->second)
      {
        delete it->second;
        it->second = NULL;
      }
    }
  }

  Component* ComponentPool::get(const std::string& id, const ParameterMap& params, const Ports<StreamInfo>& in)
  {
    ComponentProxy* p(NULL);
    // check if suitable proxy already exists
    pair<PoolType::iterator,PoolType::iterator> range = m_pool.equal_range(id);
    for (PoolType::iterator it=range.first;it!=range.second; it++)
      if (it->second->usable(params,in))
      {
        p = it->second;
        break;
      }
    // if no proxy found create one
    if (!p)
    {
      Component* c = ComponentFactory::instance()->createComponent(id);
      if (!c)
      {
        cerr << "ERROR: cannot create component " << id << endl;
        return NULL;
      }
      if (!c->init(params, in))
        return NULL;
      p = new ComponentProxy(params,in,c);
      m_pool.insert(make_pair(id,p));
    }
    // mark component as used, and return id
    p->m_used++;
    return p->m_component;
  }

  void ComponentPool::release(Component* c)
  {
    if (!c)
      return;
    pair<PoolType::iterator,PoolType::iterator> range = m_pool.equal_range(c->getIdentifier());
    for (PoolType::iterator it=range.first;it!=range.second; it++)
    {
      if (it->second && c==it->second->m_component)
      {
        it->second->m_used--;
        if (it->second->m_used==0) {
          delete it->second;
          it->second = NULL;
          m_pool.erase(it);
        }
        return;
      }
    }
    cerr << "WARNING: component " << c->getIdentifier() << " not found in ComponentPool ! cannot release it !" << endl;
  }

}
