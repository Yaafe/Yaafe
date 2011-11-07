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

#include "yaafecoreH5.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "string.h"
#include "stdlib.h"
#include <sys/stat.h>

#ifdef WITH_HDF5
#include "H5Fpublic.h"
#include "H5Gpublic.h"
#include "H5Opublic.h"
#include "H5Dpublic.h"
#include "H5PTpublic.h"
#include "H5LTpublic.h"
#include "H5Tpublic.h"
#include "H5Apublic.h"
#endif

using namespace std;

struct H5FeatureDescription** readH5FeatureDescriptions(char* h5filename)
{
#ifndef WITH_HDF5
	cerr << "ERROR: please compile Yaafe with HDF5 support !" << endl;
	return NULL;
#else
    vector<struct H5FeatureDescription*> res;

    // open file
    struct stat stFileInfo;
    if (stat(h5filename, &stFileInfo) != 0)
    {
    	cerr << "ERROR: " << h5filename << " doesn't exists or is not a file !" << endl;
    	return NULL;
    }
	if (!H5Fis_hdf5(h5filename))
	{
		cerr << "ERROR: " << h5filename << " is not H5 file !" << endl;
		return NULL;
	}
    hid_t h5file = H5Fopen(h5filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (h5file<0)
    {
        cerr << "ERROR: cannot open H5 file " << h5filename << endl;
        return NULL;
    }

    // find features
    H5G_info_t group_info;
    herr_t errCode = H5Gget_info_by_name(h5file,".",&group_info, H5P_DEFAULT);
    if (errCode<0)
    {
        cerr << "ERROR: cannot list object in H5 root" << endl;
        return NULL;
    }

    for (hsize_t i=0;i<group_info.nlinks;i++)
    {
    	H5O_info_t oinfo;
    	H5Oget_info_by_idx(h5file,".",H5_INDEX_NAME,H5_ITER_INC,i,&oinfo,H5P_DEFAULT);

        if (oinfo.type!=H5O_TYPE_DATASET)
            continue;

        hid_t obj_id = H5Oopen_by_addr(h5file,oinfo.addr);
        char ds_name[50];
        ssize_t name_len = H5Iget_name(obj_id,ds_name,50);
    	H5Oclose(obj_id);
        if (name_len<1) {
        	cerr << "ERROR: cannot read Dataset name !" << endl;
        	continue;
        }

        H5FeatureDescription hfd;

        // retrieve nbframes
        hid_t ds_id = H5PTopen(h5file,ds_name);
        if (ds_id==H5I_BADID)
            continue;
        hsize_t nb_records;
        hfd.nbframes = H5PTget_num_packets(ds_id,&nb_records);
        hfd.nbframes = nb_records;
        H5PTclose(ds_id);

        // retrieve dims
        ds_id = H5Dopen(h5file,ds_name, H5P_DEFAULT);
        hid_t type_id = H5Dget_type(ds_id);
        if ((H5Tget_class(type_id)!=H5T_ARRAY) ||(H5Tget_array_ndims(type_id)!=1))
        {
            H5PTclose(ds_id);
            continue;
        }
        hsize_t dim;
        H5Tget_array_dims(type_id,&dim);
        hfd.dim = dim;
        H5Tclose(type_id);

        // copy name
        hfd.name = strdup(ds_name);

		// retrieve various attrs
        map<string,string> attrs;
		H5Oget_info(ds_id,&oinfo);
		for (int a=0;a<oinfo.num_attrs;a++)
		{
			char attr_name[50];
			H5Aget_name_by_idx(ds_id,".",H5_INDEX_NAME,H5_ITER_INC,a,attr_name,50,H5P_DEFAULT);

			if (strcmp(attr_name,"blockSize")==0)
			{
				H5LTget_attribute_int(ds_id,".",attr_name,&hfd.blockSize);
			} else if (strcmp(attr_name,"stepSize")==0)
			{
				H5LTget_attribute_int(ds_id,".",attr_name,&hfd.stepSize);
			} else if (strcmp(attr_name,"sampleRate")==0)
			{
				H5LTget_attribute_double(ds_id,".",attr_name,&hfd.sampleRate);
			} else {
				char attr_val[1024];
				H5LTget_attribute_string(ds_id,".",attr_name,attr_val);
				attrs[attr_name] = attr_val;
			}
		}

		// transform attrs into char**
		hfd.attrs = (char**) malloc((attrs.size()*2+1)*sizeof(char*));
		{
			char** strPtr = hfd.attrs;
			for (map<string,string>::const_iterator it=attrs.begin();it!=attrs.end();it++) {
				strPtr[0] = strdup(it->first.c_str());
				strPtr[1] = strdup(it->second.c_str());
				strPtr += 2;
			}
			*strPtr = NULL;
		}

		// save description
		struct H5FeatureDescription* ptr = (struct H5FeatureDescription*) malloc(sizeof(struct H5FeatureDescription));
		memcpy(ptr,&hfd,sizeof(struct H5FeatureDescription));
		res.push_back(ptr);

		// close dataset
		H5Dclose(ds_id);
    }

    // close file
    H5Fclose(h5file);

    // transformat vector to array
    struct H5FeatureDescription** h5featlist = (struct H5FeatureDescription**) malloc((res.size()+1)*sizeof(struct H5FeatureDescription*));
    for (int i=0;i<res.size();i++)
    	h5featlist[i] = res[i];
    h5featlist[res.size()] = NULL;

	return h5featlist;
#endif
}

void freeH5FeatureDescriptions(struct H5FeatureDescription** feats)
{
#ifdef WITH_HDF5
	if (feats!=NULL) {
		for (struct H5FeatureDescription** h5ptr = feats; *h5ptr!=NULL; h5ptr++)
		{
			struct H5FeatureDescription* hfd = *h5ptr;
			free(hfd->name);
			for (char** strPtr=hfd->attrs;*strPtr!=NULL;strPtr++)
				free(*strPtr);
			free(hfd->attrs);
			free(hfd);
		}
		free(feats);
	}
#endif
}
