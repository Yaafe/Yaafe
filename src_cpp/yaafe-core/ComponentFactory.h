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

#ifndef COMPONENTFACTORY_H_
#define COMPONENTFACTORY_H_

#include "Component.h"
#include "DataFlow.h"

#define LIBRARY_HANDLER void*

namespace YAAFE {

  class ComponentFactory {
   public:
     virtual ~ComponentFactory();

     static const char* version(); // yaafe version
     static ComponentFactory* instance(); // sigleton access method
     static void destroy(); // release memory

     const std::vector<const Component*>& getPrototypeList();
     const Component* getPrototype(const std::string& identifier);
     bool exists(const std::string& identifier);
     Component* createComponent(const std::string& identifier);
     ParameterDescriptor getParameterDescriptor(const std::string& component, const std::string& param);

     int loadLibrary(const std::string& libname);
     void registerPrototype(const Component* p);

   private:
     ComponentFactory(); // singleton: cannot be instanciated by others

     std::map<std::string,LIBRARY_HANDLER> m_libs;
     std::vector<const Component*> m_prototypes;

  };

}

#endif /* COMPONENTFACTORY_H_ */
