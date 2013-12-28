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

#ifndef H5FEATUREWRITER_H_
#define H5FEATUREWRITER_H_

#include "yaafe-core/Component.h"
#include <vector>
#include "hdf5.h"

#define H5_DATASET_WRITER_ID "H5DatasetWriter"

namespace YAAFE {

  class H5DatasetWriter: public YAAFE::ComponentBase<H5DatasetWriter> {
   public:
     H5DatasetWriter();
     virtual ~H5DatasetWriter();

     const std::string getIdentifier() const {
       return H5_DATASET_WRITER_ID;
     }

     virtual bool stateLess() const { return false; };

     virtual ParameterDescriptorList getParameterDescriptorList() const;

     virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
     virtual void reset();
     virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);
     virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);


   private:
     hid_t m_h5file;
     hid_t m_table;


     struct H5FileHandler {
       std::string filename;
       hid_t id;
       int count;
     };
     static std::vector<H5FileHandler> s_files;
     static hid_t openH5File(const std::string& filename);
     static void closeH5File(hid_t h5file);

     static bool exists_attr(hid_t h5file, const std::string& obj, const char* attrname);
     static bool check_attr_string(hid_t h5file, const std::string& obj, const char* attrname, const std::string& attrvalue);
     static bool check_attr_int(hid_t h5file, const std::string& obj, const char* attrname, int attrvalue);
     static bool check_attr_double(hid_t h5file, const std::string& obj, const char* attrname, double attrvalue);
  };

}

#endif /* H5FEATUREWRITER_H_ */
