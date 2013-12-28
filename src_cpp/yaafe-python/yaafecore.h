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

#ifndef YAAFECORE_H_
#define YAAFECORE_H_

extern "C" {

  void destroyFactory();

  const char* getYaafeVersion();
  int loadComponentLibrary(char* filename);
  int isComponentAvailable(char* id);

  char** getComponentList();
  char** getOutputFormatList();
  void freeComponentList(char** strList);

  char* getOutputFormatDescription(char* id);
  void freeOutputFormatDescription(char* desc);

  struct ComponentParameter {
    char* identifier;
    char* defaultValue;
    char* description;
  };

  struct ComponentParameter** getComponentParameters(const char* id);
  struct ComponentParameter** getOutputFormatParameters(const char* id);
  void freeComponentParameters(struct ComponentParameter** params);

  void setPreferedDataBlockSize(int size);
  void setVerbose(int value);

}

#endif /* YAAFECORE_H_ */
