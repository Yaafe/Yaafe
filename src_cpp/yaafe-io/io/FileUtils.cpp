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

#include "FileUtils.h"
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>

// define for path delimiter
#ifdef __WIN32
 #define YAAFE_PATH_DELIMITER "\\"
#else
 #define YAAFE_PATH_DELIMITER "/"
#endif

using namespace std;

namespace YAAFE
{

int preparedirs(const std::string& filename)
{
    struct stat st;
    for (size_t index=filename.find_first_of(YAAFE_PATH_DELIMITER);
         index!=string::npos;
         index=filename.find_first_of(YAAFE_PATH_DELIMITER,index+1))
    {
        std::string path = filename.substr(0,index+1);
        if (stat(path.c_str(),&st)==0)
        {
            // check it is a directory
            if (!S_ISDIR(st.st_mode))
            {
                cerr << path << " is not a directory !" << endl;
                return -1;
            }
            continue;
        }
        // create dir
#ifdef __WIN32
        int res = mkdir(path.c_str());
#else
        int res = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        if (res)
            return res;
    }
    return 0;
}

map<string,string> decodeAttributeStr(const std::string& thestr)
{
	map<string,string> attrs;
	string str = thestr;
	while (str.size()>0)
	{
		size_t attrEnd = str.find('#');
		if (attrEnd==string::npos)
			attrEnd = str.size();
		size_t keyEnd = str.find('%');
		if (keyEnd==string::npos)
		{
			cerr << "ERROR: cannot parse ParameterMap string : " << str.substr(0,attrEnd) << " !" << endl;
			return attrs;
		}
		string key = str.substr(0,keyEnd);
		string value = str.substr(keyEnd+1,attrEnd-keyEnd-1);
		attrs[key] = value;
		if (attrEnd==str.size())
			break;
		str = str.substr(attrEnd+1);
	}
	return attrs;
}

}
