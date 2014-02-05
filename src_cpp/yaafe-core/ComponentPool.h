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

#ifndef COMPONENTPOOL_H_
#define COMPONENTPOOL_H_

#include "Component.h"
#include <map>

namespace YAAFE {

  class ComponentPool {
   public:
     ComponentPool();
     virtual ~ComponentPool();

     Component* get(const std::string& id, const ParameterMap& params, const Ports<StreamInfo>& in);
     void release(Component* c);

   private:

     class ComponentProxy;
     typedef std::multimap<std::string, ComponentProxy*> PoolType;
     PoolType m_pool;

  };

}

#endif /* COMPONENTPOOL_H_ */
