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

#include "H5DatasetWriter.h"
#include "FileUtils.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "H5Fpublic.h"
#include "H5Tpublic.h"
#include "H5Apublic.h"
#include "H5LTpublic.h"
#include "H5PTpublic.h"

using namespace std;

#define SAMPLE_RATE_ATTR "sampleRate"
#define BLOCKSIZE_ATTR "blockSize"
#define STEPSIZE_ATTR "stepSize"

namespace YAAFE
{

H5DatasetWriter::H5DatasetWriter() :
    m_h5file(-1), m_table(-1)
{
}

H5DatasetWriter::~H5DatasetWriter()
{
    if (m_table>=0)
    	H5PTclose(m_table);
    if (m_h5file>=0)
    	closeH5File(m_h5file);
}

ParameterDescriptorList H5DatasetWriter::getParameterDescriptorList() const
{
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "H5File";
	p.m_description = "H5 output file";
	p.m_defaultValue = "";
	pList.push_back(p);

	p.m_identifier = "Dataset";
	p.m_description = "Dataset name";
	p.m_defaultValue = "";
	pList.push_back(p);

	p.m_identifier = "Mode";
	p.m_description = "'overwrite' force overwrite already existing dataset, 'update' overwrite already existing dataset only if feature parameters have changed, 'create' fails if dataset already exists";
	p.m_defaultValue = "update";
	pList.push_back(p);

	p.m_identifier = "H5Attrs";
	p.m_description = "attributes to set to the dataset, defined as : 'key1%value1#key2%value2#...'";
	p.m_defaultValue = "";
	pList.push_back(p);

	return pList;
}


bool H5DatasetWriter::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	string outputFile = getStringParam("H5File", params);
	string dataset = getStringParam("Dataset", params);
	string mode = getStringParam("Mode", params);
	string h5attrs = getStringParam("H5Attrs", params);
	// parse attributes
	map<string,string> attrs;
	while (h5attrs.size()>0)
	{
		size_t attrEnd = h5attrs.find('#');
		if (attrEnd==string::npos)
			attrEnd = h5attrs.size();
		size_t keyEnd = h5attrs.find('%');
		if (keyEnd==string::npos)
		{
			cerr << "ERROR: cannot parse H5 Attribute : " << h5attrs.substr(0,attrEnd) << " !" << endl;
			return false;
		}
		string key = h5attrs.substr(0,keyEnd);
		string value = h5attrs.substr(keyEnd+1,attrEnd-keyEnd-1);
		attrs[key] = value;
		if (attrEnd==h5attrs.size())
			break;
		h5attrs = h5attrs.substr(attrEnd+1);
	}

	// open or create outputFile
	m_h5file = openH5File(outputFile);
	if (m_h5file<0)
		return false;

    if (H5LTfind_dataset(m_h5file,dataset.c_str()))
    {
    	if (mode=="overwrite")
    	{
            if (verboseFlag)
                cerr << "INFO: overwrite H5 dataset " << dataset << endl;
            herr_t res = H5Ldelete(m_h5file, dataset.c_str(), H5P_DEFAULT);
            if (res < 0)
            {
                cerr << "ERROR: H5 error while deleting dataset "
                        << dataset << endl;
                return false;
            }
    	} else if (mode=="update")
    	{
    		bool hasNotChange = check_attr_double(m_h5file, dataset, SAMPLE_RATE_ATTR, in.sampleRate);
    		hasNotChange = hasNotChange && check_attr_int(m_h5file, dataset, BLOCKSIZE_ATTR, in.frameLength);
    		hasNotChange = hasNotChange && check_attr_int(m_h5file,dataset,STEPSIZE_ATTR,in.sampleStep);
    	    for (map<string,string>::const_iterator it=attrs.begin();
    	    		it!=attrs.end();it++)
    	    	hasNotChange = hasNotChange && check_attr_string(m_h5file,dataset,it->first.c_str(),it->second);
    	    if (hasNotChange) {
    	    	m_table = -1;
    	    	cerr << "INFO: H5 dataset " << dataset << " is up to date, do not write." << endl;
    	    	return true;
    	    } else {
    	    	cerr << "INFO: H5 dataset " << dataset << " is out of date, overwrite it !" << endl;
                herr_t res = H5Ldelete(m_h5file, dataset.c_str(), H5P_DEFAULT);
                if (res < 0)
                {
                    cerr << "ERROR: H5 error while deleting dataset "
                            << dataset << endl;
                    return false;
                }
    	    }
    	} else {
			cerr << "ERROR: dataset " << dataset << " already exists" << endl;
			m_table = -1;
			return false;
    	}
    }

    // create dataset
    const hsize_t dims[] = { in.size };
    hid_t atype_id = H5Tarray_create2(H5T_NATIVE_DOUBLE,1, dims);
    size_t dataSize = H5Tget_size(atype_id);
    hsize_t chunk_size = 500000 / dataSize; // chunk size should not exceed 1MB. 500KB is a good tradeoff
    if (chunk_size == 0)
        chunk_size = 1; // but should not be null :-)
    m_table = H5PTcreate_fl(m_h5file, dataset.c_str(), atype_id,
        chunk_size, 1);
    H5Tclose(atype_id);
    if (m_table == H5I_BADID)
    {
        cerr << "ERROR: cannot create H5 table " << dataset
                << endl;
        return false;
    }
    // set metadata
    double sampleRate = in.sampleRate;
    H5LTset_attribute_double(m_h5file, dataset.c_str(), SAMPLE_RATE_ATTR,
    &sampleRate, 1);
    int blockSize = in.frameLength;
    H5LTset_attribute_int(m_h5file, dataset.c_str(), BLOCKSIZE_ATTR,
    &blockSize, 1);
    int stepSize = in.sampleStep;
    H5LTset_attribute_int(m_h5file, dataset.c_str(), STEPSIZE_ATTR,
    &stepSize, 1);
    for (map<string,string>::const_iterator it=attrs.begin();
    		it!=attrs.end();it++)
    	H5LTset_attribute_string(m_h5file, dataset.c_str(), it->first.c_str(), it->second.c_str());

    return true;
}

void H5DatasetWriter::reset()
{
	// nothing to do
}

bool H5DatasetWriter::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	// if dataset dot initialized, do not write data
	if (m_table<0)
		return false;
	// write data
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==0);
    while (!in->empty()) {
    	H5PTappend(m_table, in->blockAvailableTokens(), in->readToken());
    	in->blockConsume();
    }
    return false;
}

void H5DatasetWriter::flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out)
{
    process(in, out);

    // close dataset
    if (m_table>=0)
    	H5PTclose(m_table);
    m_table = -1;
    // close h5file
    if (m_h5file>=0)
    	closeH5File(m_h5file);
    m_h5file = -1;
}

std::vector<H5DatasetWriter::H5FileHandler> H5DatasetWriter::s_files;

hid_t H5DatasetWriter::openH5File(const std::string& filename)
{
	// check if file is already opended
	for (std::vector<H5FileHandler>::iterator it=s_files.begin();
			it!=s_files.end(); it++)
	{
		if (it->filename==filename)
		{
			it->count++;
			return it->id;
		}
	}

	// open or create file
	H5FileHandler hfh;
	hfh.filename = filename;
	hfh.count = 1;
    struct stat stFileInfo;
    if (stat(filename.c_str(), &stFileInfo) == 0)
    {
    	if (H5Fis_hdf5(filename.c_str()))
		{
			hfh.id = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
			if (hfh.id < 0)
			{
				cerr << "ERROR: cannot open H5 file " << filename << endl;
				return hfh.id;
			}
		} else {
			cerr << "ERROR: " << filename << " is not H5 file !" << endl;
			return -1;
		}
    } else {
		preparedirs(filename.c_str());
		hfh.id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
			H5P_DEFAULT);
		if (hfh.id < 0)
		{
			cerr << "ERROR: cannot create H5 file " << filename << endl;
			return hfh.id;
		}
    }

    s_files.push_back(hfh);
    return hfh.id;
}

void H5DatasetWriter::closeH5File(hid_t h5file)
{
	for (std::vector<H5FileHandler>::iterator it=s_files.begin();
			it!=s_files.end(); it++)
	{
		if (it->id==h5file)
		{
			it->count--;
			if (it->count==0)
			{
				H5Fclose(it->id);
				s_files.erase(it);
			}
			return;
		}
	}
	cerr << "ERROR: H5DatasetWriter try to close an unregistered H5 file !" << endl;
}


bool H5DatasetWriter::exists_attr(hid_t h5file, const std::string& obj,
    const char* attrname)
{
    htri_t rc = H5Aexists_by_name(h5file, obj.c_str(), attrname, H5P_DEFAULT);
    if (rc < 0)
    {
        cerr << "WARNING: error in H5Aexists_by_name for attribute "
                << attrname << endl;
        return false;
    }
    return (rc > 0);
}

bool H5DatasetWriter::check_attr_string(hid_t h5file, const std::string& obj,
    const char* attrname, const std::string& attrvalue)
{
    htri_t rc = H5Aexists_by_name(h5file, obj.c_str(), attrname, H5P_DEFAULT);
    if (rc < 0)
    {
        cerr << "WARNING: error in H5Aexists_by_name for attribute "
                << attrname << endl;
        return false;
    }
    if (rc == 0)
        return false;
    char buf[1000];
    herr_t res = H5LTget_attribute_string(h5file, obj.c_str(), attrname, buf);
    if (res < 0)
    {
        cerr << "WARNING: error in H5LTget_attribute_string" << endl;
        return false;
    }
    return (strcmp(buf, attrvalue.c_str()) == 0);
}

bool H5DatasetWriter::check_attr_int(hid_t h5file, const std::string& obj,
    const char* attrname, int attrvalue)
{
    htri_t rc = H5Aexists_by_name(h5file, obj.c_str(), attrname, H5P_DEFAULT);
    if (rc < 0)
    {
        cerr << "WARNING: error in H5Aexists_by_name for attribute "
                << attrname << endl;
        return false;
    }
    if (rc == 0)
        return false;
    int tmp;
    herr_t res = H5LTget_attribute_int(h5file, obj.c_str(), attrname, &tmp);
    if (res < 0)
    {
        cerr << "WARNING: error in H5LTget_attribute_int" << endl;
        return false;
    }
    return (tmp == attrvalue);

}

bool H5DatasetWriter::check_attr_double(hid_t h5file, const std::string& obj,
    const char* attrname, double attrvalue)
{
    htri_t rc = H5Aexists_by_name(h5file, obj.c_str(), attrname, H5P_DEFAULT);
    if (rc < 0)
    {
        cerr << "WARNING: error in H5Aexists_by_name for attribute "
                << attrname << endl;
        return false;
    }
    if (rc == 0)
        return false;
    double tmp;
    herr_t res = H5LTget_attribute_double(h5file, obj.c_str(), attrname, &tmp);
    if (res < 0)
    {
        cerr << "WARNING: error in H5LTget_attribute_double" << endl;
        return false;
    }
    return (tmp == attrvalue);

}

}
