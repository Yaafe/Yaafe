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

#include "ComponentFactory.h"
#include <vector>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace YAAFE {

  ComponentFactory::ComponentFactory() {
  }

  ComponentFactory::~ComponentFactory() {
    for (vector<const Component*>::const_iterator it =
        m_prototypes.begin(); it != m_prototypes.end(); it++) {
      delete *it;
    }
    for (map<string,LIBRARY_HANDLER>::const_iterator it=m_libs.begin();
        it!=m_libs.end(); it++)
    {
      dlclose(it->second);
    }
  }

  const char* ComponentFactory::version()
  {
    static string v;
    if (v.size()==0) {
      ostringstream oss;
      oss << VERSION;
      v = oss.str();
    }
    return v.c_str();
  }


  ComponentFactory* ComponentFactory::instance() {
    static ComponentFactory* instance = new ComponentFactory();
    return instance;
  }

  void ComponentFactory::destroy() {
    ComponentFactory* fact = instance();
    delete fact;
  }

  const std::vector<const Component*>& ComponentFactory::getPrototypeList() {
    return m_prototypes;
  }

  const Component* ComponentFactory::getPrototype(const std::string& identifier) {
    for (vector<const Component*>::const_iterator it = m_prototypes.begin(); it
        != m_prototypes.end(); it++) {
      if ((*it)->getIdentifier() == identifier)
        return (*it);
    }
    cerr << "ERROR: unknown component " << identifier << endl;
    return NULL;
  }

  bool ComponentFactory::exists(const std::string& id)
  {
    for (vector<const Component*>::const_iterator it=m_prototypes.begin();
        it!=m_prototypes.end(); it++)
      if ((*it)->getIdentifier()==id)
        return true;
    return false;
  }

  Component* ComponentFactory::createComponent(const std::string& identifier) {
    const Component* p = getPrototype(identifier);
    if (p)
      return p->clone();
    return NULL;
  }

  ParameterDescriptor ComponentFactory::getParameterDescriptor(
      const std::string& component, const std::string& param) {
    const Component* c = getPrototype(component);
    if (!c)
      return ParameterDescriptor();
    const ParameterDescriptorList& pList = c->getParameterDescriptorList();
    for (size_t i = 0; i < pList.size(); i++) {
      if (pList[i].m_identifier == param)
        return pList[i];
    }
    assert(false);
    return ParameterDescriptor();
  }

  int ComponentFactory::loadLibrary(const std::string& libnamestr) {

    if (m_libs.find(libnamestr)!=m_libs.end())
    {
      // library already loaded
      return 0;
    }

    // Linux specific code
    std::string complete_name = "lib" + libnamestr + ".so";

    // first look at YAAFE_LIBRARY_DIR
    if (getenv("YAAFE_PATH"))
    {
      string tmp = string(getenv("YAAFE_PATH")) + string("/") + complete_name;
      struct stat st;
      if (stat(tmp.c_str(),&st)==0)
      {
        // file exists
        complete_name = tmp;
      }
    } // else look at library in default library accessible dirs

    const char* libname = complete_name.c_str();
    LIBRARY_HANDLER library = dlopen(libname, RTLD_LAZY);
    if (library == NULL) {
      cerr << "ERROR: cannot load yaafe component library \"" << libname << "\" !" << endl;
      cerr << dlerror() << endl;
      return -1;
    }

    void* initializer = dlsym(library, "registerYaafeComponents");
    if (initializer == NULL) {
      cerr << "ERROR: cannot find 'registerYaafeComponents' function in component library '" << libname << "' !" << endl;
      dlclose(library);
      return -2;
    }

    m_libs[libnamestr] = library;

    typedef void (*yaafelib_register_function_type)(void*);
    yaafelib_register_function_type func =
      *((yaafelib_register_function_type*) (&initializer));

    func((void*) this);

    return 0;
  }

  void ComponentFactory::registerPrototype(const Component* p) {
    m_prototypes.push_back(p);
  }

}
