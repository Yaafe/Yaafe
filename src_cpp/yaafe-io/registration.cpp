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

#include "registration.h"

#include "yaafe-core/ComponentFactory.h"

#include "yaafe-io/io/CSVWriter.h"
#ifdef WITH_SNDFILE
#include "yaafe-io/io/AudioFileReader.h"
#endif
#ifdef WITH_MPG123
#include "yaafe-io/io/MP3FileReader.h"
#endif
#ifdef WITH_HDF5
#include "yaafe-io/io/H5DatasetWriter.h"
#endif


using namespace YAAFE;

void registerYaafeComponents(void* componentFactory)
{
    ComponentFactory* factory = reinterpret_cast<ComponentFactory*>(componentFactory);

	factory->registerPrototype(new CSVWriter());
#ifdef WITH_SNDFILE
	factory->registerPrototype(new AudioFileReader());
#endif
#ifdef WITH_MPG123
	factory->registerPrototype(new MP3FileReader());
#endif
#ifdef WITH_HDF5
	factory->registerPrototype(new H5DatasetWriter());
#endif
}
