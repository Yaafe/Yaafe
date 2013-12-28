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

#include <algorithm>
#include <iostream>
#include <string>
#include "argtable2.h"
#include "yaafe-core/DataFlow.h"
#include "yaafe-core/AudioFileProcessor.h"
#include "yaafe-core/OutputFormat.h"
#include "yaafe-core/Engine.h"
#include "yaafe-core/ComponentFactory.h"

using namespace YAAFE;
using namespace std;

void listComponents()
{
	vector<string> components;
    const vector<const Component*>& cList = ComponentFactory::instance()->getPrototypeList();
    for (size_t i = 0; i < cList.size(); i++)
    {
    	components.push_back(cList[i]->getIdentifier());
    }
    sort(components.begin(),components.end());
	cout << "Available components : " << endl;
    for (vector<string>::const_iterator it = components.begin(); it
            != components.end(); it++)
        cout << " - " << *it << endl;
}

void describeComponent(const std::string component)
{
    const Component* c = ComponentFactory::instance()->getPrototype(component);
    if (c)
    {
        cout << c->getIdentifier() << " : ";
        if (c->getDescription() != "")
            cout << c->getDescription();
        cout << endl;
        const ParameterDescriptorList& params = c->getParameterDescriptorList();
        for (ParameterDescriptorList::const_iterator it = params.begin(); it
                != params.end(); it++)
        {
            cout << " - " << it->m_identifier << " : " << it->m_description;
            cout << " (default=" << it->m_defaultValue << ")" << endl;
        }
        return;
    }
}

void printOutputFormats()
{
	vector<string> formats = OutputFormat::availableFormats();
	printf("Available output formats are:\n");
	for (int i=0;i<formats.size();i++)
	{
		const OutputFormat* f = OutputFormat::get(formats[i]);
		printf("[%s] %s\n",formats[i].c_str(),f->getDescription().c_str());
		printf("     Parameters:\n");
		ParameterDescriptorList pList = f->getParameters();
		for (int p=0;p<pList.size();p++) {
			printf("     - %s: %s (default=%s)\n", pList[p].m_identifier.c_str(),
					pList[p].m_description.c_str(), pList[p].m_defaultValue.c_str());
		}
		printf("\n");
	}
}

struct arg_lit *h, *version, *verbose, *l;
struct arg_str *d, *libs, *outdir, *format, *formatparams;
struct arg_file *files, *dataflow;
struct arg_int *datablock;
struct arg_end *end_;

int main(int argc, char **argv)
{
	int exitcode = 0;
	const char* progname = argv[0];

    // build option parser
    void* argtable[] = {
        h = arg_lit0("h","help", "print this help and exit"),
        version = arg_lit0("v","version", "print version information and exit"),
        verbose = arg_lit0(NULL, "verbose", "more logs"),
        l = arg_lit0("l", NULL, "list all available components"),
        d = arg_str0("d","describe","component", "Describe a component, show its parameters"),
        datablock = arg_int0("s",NULL, "datablocksize", "prefered data block size"),
        libs = arg_strn("x","loadlibrary","libnames",0,10,"yaafe component library name to load."),
        dataflow = arg_file0("c",NULL,"file","dataflow to process"),
        format = arg_str0("o", NULL,"format","output format, see available output formats below."),
        formatparams = arg_strn("p", NULL,"key=value",0,10,"output format parameters (see below)"),
        outdir = arg_str0("b", NULL,"dir","output base directory"),
        files = arg_filen(NULL,NULL,"FILES",0,argc,"audio files to process"),
        end_ = arg_end(20) };

    int nerrors = arg_parse(argc, argv, argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (h->count > 0)
    {
    	printf("%s is a dataflow processing engine\n", progname);
    	printf("\n");
    	printf("Usage: %s", progname);
    	arg_print_syntax(stdout,argtable,"\n");
    	arg_print_glossary(stdout,argtable, " %-25s %s\n");
    	printf("\n");
        exitcode = ComponentFactory::instance()->loadLibrary("yaafe-io");
        if (exitcode) {
        	printf("error while loading yaafe-io library !\n");
        } else {
    	   	printOutputFormats();
    	   	exitcode = 0;
        }
    	goto exit;
    }

    /* special case: '--version' takes precedence error reporting */
    if (version->count > 0)
    {
        printf("%s version %s\n", progname, ComponentFactory::version());
        exitcode = 0;
        goto exit;
    }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0)
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout,end_, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = 1;
        goto exit;
    }

    if (verbose->count)
        verboseFlag = true;

    if (datablock->count)
    	DataBlock::setPreferedBlockSize(datablock->ival[0]);

    // register io components
    exitcode = ComponentFactory::instance()->loadLibrary("yaafe-io");
    if (exitcode)
    	goto exit;

    // register given dynamic libraries
    for (int i=0;i<libs->count;i++)
    {
        exitcode = ComponentFactory::instance()->loadLibrary(libs->sval[i]);
        if (exitcode)
            goto exit;
    }

    if (l->count)
    {
        listComponents();
        goto exit;
    }
    if (d->count)
    {
        describeComponent(d->sval[0]);
        goto exit;
    }


    if (files->count)
    {
    	if (!dataflow->count) {
    		cerr << "ERROR: please specify a dataflow file with -c option !" << endl;
    		exitcode = -1; goto exit;
    	}

    	DataFlow df;
    	if (!df.load(dataflow->filename[0])) {
    		cerr << "ERROR: cannot load dataflow from file " << dataflow->filename[0] << endl;
    		exitcode = -1; goto exit;
    	}

		Engine engine;
		if (!engine.load(df)) {
			cerr << "ERROR: cannot initialize dataflow engine" << endl;
			exitcode = -1; goto exit;
		}

		AudioFileProcessor processor;
		{
			string formatStr = "csv";
			if (format->count)
				formatStr = format->sval[0];
			string outDirStr = "";
			if (outdir->count)
				outDirStr = outdir->sval[0];
			ParameterMap params;
			for (int p=0;p<formatparams->count;p++) {
				string str = formatparams->sval[p];
				size_t keyEnd = str.find('=');
				if (keyEnd==string::npos) {
					cerr << "Malformed parameter " << formatparams->sval[p] << endl;
					continue;
				}
				params[str.substr(0,keyEnd)] = str.substr(keyEnd+1,str.size()-keyEnd-1);
			}
			if (!processor.setOutputFormat(formatStr,outDirStr,params)) {
				exitcode = -1; goto exit;
			}
		}

		for (int i=0;i<files->count; i++)
		{
			int res = processor.processFile(engine, files->filename[i]);
			if (res!=0) {
				cerr << "ERROR: error while processing " << files->filename[i] << endl;
			}
		}
    }

#ifdef WITH_TIMERS
    Timer::print_all_timers();
#endif

    exit:
    // release components to avoid definitly lost blocks in valgrind
    ComponentFactory::destroy();
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	return exitcode;
}
