/**
 * Smarc
 *
 * Copyright (c) 2009-2011 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Authors : Benoit Mathieu, Jacques Prado
 *
 * This file is part of Smarc.
 *
 * Smarc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Smarc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smarc.h"
#include "sndfile.h"
#include "argtable2.h"
#include "math.h"

#ifndef VERSION
#define VERSION "undefined"
#endif

#define BUF_SIZE 8192

#define BANDWIDTH "0.95"
#define RP "0.1"
#define RS "140"
#define TOL "0.000001"

struct arg_lit *h, *version, *verbose, *fast, *check;
struct arg_str *c, *b, *rp, *rs, *tol, *ratios;
struct arg_int *r, *nbits;
struct arg_file *files;
struct arg_end *end;

void resample_mono(struct PFilter* pfilter, SNDFILE* fin, SNDFILE* fout);
void resample_downmix(struct PFilter* pfilter, SNDFILE* fin, SNDFILE* fout,
		int nbChannels);
void resample_channel(struct PFilter* pfilter, SNDFILE* fin, SNDFILE* fout,
		int nbChannels, int channel);
void resample_separately(struct PFilter* pfilter, SNDFILE* fin, SNDFILE* fout,
		int nbChannels);

void check_filter(struct PFilter* pfilter,int fsin, int fsout, double bandwidth);

int main(int argc, char** argv) {
	int exitcode = 0;
	const char* progname = argv[0];
	// build option parser
	void* argtable[] = {
			h = arg_lit0("h", "help", "print this help and exit"),
			version = arg_lit0("v", "version", "print version information and exit"),
			verbose = arg_lit0(NULL, "verbose", "more logs"),
			r = arg_int1("r", "rate","int", "samplerate of output file"),
			nbits = arg_int0(NULL, "nbits","int","force output sample format to 16|24|32 bits. If not defined, use the input format."),
			c = arg_str0("c", "channels", "s|d|1|2|..", " used when input file has more than 1 channel"),
			arg_rem(NULL, " 's' resample each channels separately (default)"),
			arg_rem(NULL, " 'd' downmix using mean of channels"),
			arg_rem(NULL, " '1' keep only first channel"),
			arg_rem(NULL, " '2' keep only second channel"),
			arg_rem(NULL, " '<N>' keep only Nth channel"),
			b = arg_str0("b", "bandwidth", "0..1","define the bandpass filter bandwidth, in relation"),
			arg_rem(NULL, "with maximum possible bandwidth (default=" BANDWIDTH ")"),
			rp = arg_str0(NULL, "rp", RP,"ripple factor of pass band filter, in dB"),
			rs = arg_str0(NULL, "rs", RS,"ripple factor of stop band filter, in dB"),
			tol = arg_str0(NULL, "tol", TOL,"converter samplerate tolerance (default=" TOL ")"),
			ratios = arg_str0(NULL,"ratios", "L1/M1 L2/M2 ...","multistage ratios definitions. If not given, smarc will try to find appropriate ratios"),
			fast = arg_lit0(NULL, "fast", "search for fastest conversion stages. This search may fail."),
			check = arg_lit0(NULL, "check", "check filter is valid"),
			files = arg_filen(NULL, NULL, "audiofile", 2, 2, NULL),
			end	= arg_end(20) };

	nbits->ival[0] = 0;
	c->sval[0] = "s";
	b->sval[0] = BANDWIDTH;
	rp->sval[0] = RP;
	rs->sval[0] = RS;
	tol->sval[0] = TOL;
	ratios->sval[0] = "";

	int nerrors = arg_parse(argc, argv, argtable);

	/* special case: '--help' takes precedence over error reporting */
	if (h->count > 0) {
		printf(
				"%s resamples audio files using multi stage polyphase filters\n",
				progname);
		printf("\n");
		printf("Usage: smarc [-hv] [--verbose] -r int --nbits=int [-c s|d|1|2|..] /\n");
		printf("             [-b 0..1] [--rp=0.1] [--rs=100] [--tol=0.000001] /\n");
		printf("             [--ratios=L1/M1 L2/M2 ...] audiofile audiofile\n");	
		printf("	-h, --help print this help and exit\n");
		printf("	-v, --version print version information and exit\n");
		printf("	--verbose more logs\n");
		printf("	-r, --rate=int samplerate of output file\n");
		printf("	--nbits=int force output sample format to 16|24|32 bits.\n");
        printf("		If not defined, use the input format.\n");
		printf("	-c, --channels=s|d|1|2|.. used when input file has more than 1 channel\n");
        printf("		's' resample each channels separately (default)\n");
        printf("		'd' downmix using mean of channels\n");
        printf("		'1' keep only first channel\n");
        printf("		'2' keep only second channel\n");
        printf("		'<N>' keep only Nth channel\n");
		printf("	-b, --bandwidth=0..1 define the bandpass filter bandwidth, in relation\n");
        printf("		with maximum possible bandwidth (default=0.95)\n");
		printf("	--rp=0.1 ripple factor of pass band filter, in dB\n");
		printf("	--rs=140 ripple factor of stop band filter, in dB\n");
		printf("	--tol=0.000001 converter samplerate tolerance (default=0.000001)\n");
		printf("	--ratios=L1/M1 L2/M2 ... multistage ratios definitions.\n");
		printf("		If not given, smarc will try to find appropriate ratios\n");
		printf("    --fast search for fastest conversion stages.\n");
//		arg_print_syntax(stdout, argtable, "\n");
//		arg_print_glossary(stdout, argtable, "  %-25s %s\n");
		exitcode = 0;
		goto exit;
	}

	/* special case: '--version' takes precedence error reporting */
	if (version->count > 0) {
		printf("%s version %s\n", progname, VERSION);
		exitcode = 0;
		goto exit;
	}

	/* report errors */
	if (nerrors>0)
	{
		arg_print_errors(stdout, end, progname);
		exitcode = -1;
		goto exit;
	}


	if (r->count == 0) {
		printf("Please specify output sample rate !\n");
		exitcode = -1;
		goto exit;
	}

	{
		const char* inputfile = files->filename[0];
		const char* outputfile = files->filename[1];

		// open input file
		SF_INFO info;
		memset(&info, 0, sizeof(SF_INFO));
		SNDFILE* fh = sf_open(inputfile, SFM_READ, &info);
		if (!fh) {
			printf("ERROR: cannot open file %s\n", inputfile);
			exitcode = -1;
			goto exit;
		}
		int fsin = info.samplerate;
		int fsout = r->ival[0];

		// init filter
		double bandwidth = atof(b->sval[0]);
		if (b->sval[0][0]=='=') // workaround for common mistake in command line
			bandwidth = atof(b->sval[0]+1);
		if (bandwidth<=0 || bandwidth>=1)
		{
			printf("ERROR: invalid bandwidth parameter %f, should be in 0 < .. < 1\n",bandwidth);
			exitcode = -1;
			goto exit;
		}
		double rpvalue = atof(rp->sval[0]);
		if (rpvalue<=0)
		{
			printf("ERROR: invalid rp parameter %f, should be > 0\n",rpvalue);
			exitcode = -1;
			goto exit;
		}
		double rsvalue = atof(rs->sval[0]);
		if (rsvalue<=0)
		{
			printf("ERROR: invalid rs parameter %f, should be > 0\b",rsvalue);
			exitcode = -1;
			goto exit;
		}
		double tolerance = atof(tol->sval[0]);
		const char* userratios = ratios->sval[0];
		struct PFilter* pfilt = smarc_init_pfilter(fsin, fsout, bandwidth, rpvalue,
				rsvalue, tolerance, userratios, fast->count);
		if (pfilt == NULL)
			goto exit;

		if (verbose->count)
			smarc_print_pfilter(pfilt);

		if (check->count)
			check_filter(pfilt, fsin, fsout, bandwidth);

		// init channels
		int inc = info.channels;
		int outc = inc;
		int downmix = -1;
		if (inc > 1) {
			if (c->sval[0][0] == 's') {
				outc = inc;
				downmix = -1; // resample all channels separately
			} else if (c->sval[0][0] == 'd') {
				outc = 1;
				downmix = 0; // downmix
			} else {
				outc = 1;
				downmix = atoi(c->sval[0]);
				if (downmix <= 0) {
					printf("invalid channel parameter !\n");
					exitcode = -1;
					goto exit;
				}
			}
		} else {
			if (c->count) {
				printf(
						"WARNING: input file is mono, channel parameter is ignored\n");
			}
		}

		// open output file
		SF_INFO outinfo;
		outinfo.samplerate = fsout;
		outinfo.channels = outc;
		outinfo.format = info.format;
		if (nbits->ival[0]!=0)
		{
			if (nbits->ival[0]==16)
				outinfo.format = (outinfo.format & ~SF_FORMAT_SUBMASK) | SF_FORMAT_PCM_16;
			else if (nbits->ival[0]==24)
				outinfo.format = (outinfo.format & ~SF_FORMAT_SUBMASK) | SF_FORMAT_PCM_24;
			else if (nbits->ival[0]==32)
				outinfo.format = (outinfo.format & ~SF_FORMAT_SUBMASK) | SF_FORMAT_PCM_32;
			else {
				printf("ERROR: invalid nbits parameter value %i !",nbits->ival[0]);
				goto exit;
			}

		}
		SNDFILE* fhout = sf_open(outputfile, SFM_WRITE, &outinfo);
		sf_command(fhout, SFC_SET_CLIPPING, NULL, SF_TRUE);

		if (inc == 1) {
			if (verbose->count)
				printf("resample mono audio file\n");
			resample_mono(pfilt, fh, fhout);
		} else if (inc > 1 && downmix == -1) {
			if (verbose->count)
				printf("resample %i channels separately\n", inc);
			resample_separately(pfilt, fh, fhout, inc);
		} else if (inc > 1 && downmix == 0) {
			if (verbose->count)
				printf("downmix %i channels to 1 using mean of channels\n", inc);
			resample_downmix(pfilt, fh, fhout, inc);
		} else {
			if (verbose->count)
				printf(
						"input audio file has %i channels, resample only channel %i\n",
						inc, downmix);
			resample_channel(pfilt, fh, fhout, inc, downmix - 1);
		}

		// close files
		sf_close(fh);
		sf_close(fhout);

		// release filter
		smarc_destroy_pfilter(pfilt);

	}

	exit: arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	return 0;
}

/**
 * resample mono file, output mono file
 */
void resample_mono(struct PFilter* pfilt, SNDFILE* fin, SNDFILE* fout) {
	// init state and buffer per channels
	struct PState* pstate = smarc_init_pstate(pfilt);
	const int IN_BUF_SIZE = BUF_SIZE;
	const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
	double* inbuf = malloc(IN_BUF_SIZE * sizeof(double));
	double* outbuf = malloc(OUT_BUF_SIZE * sizeof(double));

	int read = 0;
	int written = 0;

	// resample audio
	while (1) {
		read = sf_readf_double(fin, inbuf, IN_BUF_SIZE);
		if (read == 0) {
			// reached end of file, have to flush last values
			break;
		}
		written = smarc_resample(pfilt, pstate, inbuf, read, outbuf,
				OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
	}

	// flushing last values
	while (1) {
		written = smarc_resample_flush(pfilt, pstate, outbuf,
				OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
		if (written<OUT_BUF_SIZE)
			break;
	}

	// release memomry
	smarc_destroy_pstate(pstate);
	free(inbuf);
	free(outbuf);
}

/**
 * input file has more than 1 channel. resample_downmix compute mean of input file
 * channels and resample it, result is a mono file.
 */
void resample_downmix(struct PFilter* pfilt, SNDFILE* fin, SNDFILE* fout,
		int nbChannels) {
	// init state and buffer per channels
	struct PState* pstate = smarc_init_pstate(pfilt);
	const int IN_BUF_SIZE = BUF_SIZE;
	const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
	double* inbuf = malloc(IN_BUF_SIZE * sizeof(double));
	double* outbuf = malloc(OUT_BUF_SIZE * sizeof(double));
	double* readBuf = malloc(IN_BUF_SIZE * nbChannels * sizeof(double));

	int read = 0;
	int written = 0;

	// resample audio
	while (1) {
		read = sf_readf_double(fin, readBuf, IN_BUF_SIZE);
		if (read==0) {
			// reached end of file, have to flush last values
			break;
		}

		// downmix
		for (int i = 0; i < read; i++) {
			double d = 0.0;
			for (int c = 0; c < nbChannels; c++)
				d += readBuf[i * nbChannels + c];
			inbuf[i] = d / nbChannels;
		}

		written = smarc_resample(pfilt, pstate, inbuf, read, outbuf,
				OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
	}

	// flushing last values
	while (1) {
		written = smarc_resample_flush(pfilt, pstate,
				outbuf,	OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
		if (written<OUT_BUF_SIZE)
			break;
	}

	// release memomry
	smarc_destroy_pstate(pstate);
	free(inbuf);
	free(outbuf);
	free(readBuf);
}

/**
 * Resample only a specific channel of input file, output file is mono.
 */
void resample_channel(struct PFilter* pfilt, SNDFILE* fin, SNDFILE* fout,
		int nbChannels, int channel) {
	// init state and buffer per channels
	struct PState* pstate = smarc_init_pstate(pfilt);
	const int IN_BUF_SIZE = BUF_SIZE;
	const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
	double* inbuf = malloc(IN_BUF_SIZE * sizeof(double));
	double* outbuf = malloc(OUT_BUF_SIZE * sizeof(double));
	double* readBuf = malloc(IN_BUF_SIZE * nbChannels * sizeof(double));

	int read = 0;
	int written = 0;

	// resample audio
	while (1) {
		read = sf_readf_double(fin, readBuf, IN_BUF_SIZE);
		if (read==0) {
			// reached end of file, have to flush last values
			break;
		}

		// keep only one channel
		for (int i = 0; i < read; i++)
			inbuf[i] = readBuf[i * nbChannels + channel];

		written = smarc_resample(pfilt, pstate, inbuf, read,
				outbuf,	OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
	}

	// flushing last values
	while (1) {
		written = smarc_resample_flush(pfilt, pstate,
				outbuf,	OUT_BUF_SIZE);
		sf_writef_double(fout, outbuf, written);
		if (written<OUT_BUF_SIZE)
			break;
	}

	// release memomry
	smarc_destroy_pstate(pstate);
	free(inbuf);
	free(outbuf);
	free(readBuf);
}

/**
 * Resample each input file channels separately, output file has same
 * number of channels.
 */
void resample_separately(struct PFilter* pfilt, SNDFILE* fin, SNDFILE* fout,
		int nbChannels) {
	// init state and buffer per channels
	struct PState* pstate[nbChannels];
	for (int c = 0; c < nbChannels; c++)
		pstate[c] = smarc_init_pstate(pfilt);
	const int IN_BUF_SIZE = BUF_SIZE;
	const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
	double* inbuf = malloc(IN_BUF_SIZE * sizeof(double));
	double* outbuf = malloc(OUT_BUF_SIZE * sizeof(double));
	double* readBuf = malloc(IN_BUF_SIZE * nbChannels * sizeof(double));
	double* writeBuf = malloc(OUT_BUF_SIZE * nbChannels * sizeof(double));

	int read = 0;
	int written = 0;

	// resample audio
	while (1) {
		read = sf_readf_double(fin, readBuf, IN_BUF_SIZE);

		if (read == 0) {
			// reached end of file, have to flush last values
			break;
		}

		// resample each channels
		for (int c = 0; c < nbChannels; c++) {
			for (int i = 0; i < read; i++)
				inbuf[i] = readBuf[i * nbChannels + c];
			written = smarc_resample(pfilt, pstate[c], inbuf, read,
					outbuf, OUT_BUF_SIZE);
			for (int i = 0; i < written; i++)
				writeBuf[i * nbChannels + c] = outbuf[i];
		}

		sf_writef_double(fout, writeBuf, written);
	}

	// flushing last values
	while (1) {
		for (int c = 0; c < nbChannels; c++) {
			written = smarc_resample_flush(pfilt, pstate[c],
					outbuf, OUT_BUF_SIZE);
			for (int i = 0; i < written; i++)
				writeBuf[i * nbChannels + c] = outbuf[i];
		}
		sf_writef_double(fout, writeBuf, written);
		if (written<OUT_BUF_SIZE)
			break;
	}

	// release memory
	for (int c = 0; c < nbChannels; c++)
		smarc_destroy_pstate(pstate[c]);
	free(inbuf);
	free(outbuf);
	free(readBuf);
	free(writeBuf);
}

/*
 * Check filter
 */
void check_filter(struct PFilter* pfilt,int fsin, int fsout, double bandwidth) {
	// initialize state and buffers
	struct PState* pstate = smarc_init_pstate(pfilt);
	const int IN_BUF_SIZE = BUF_SIZE;
	const int OUT_BUF_SIZE = smarc_get_output_buffer_size(pfilt,IN_BUF_SIZE);
	double* inbuf = (double*) malloc(IN_BUF_SIZE*sizeof(double));
	double* outbuf = (double*) malloc(OUT_BUF_SIZE*sizeof(double));

	// open files
	SNDFILE* chirp_in;
	SNDFILE* chirp_out;
	SNDFILE* chirp_converted;
	{
		SF_INFO outinfo;
		outinfo.samplerate = fsin;
		outinfo.channels = 1;
		outinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		chirp_in = sf_open("chirp_in.wav", SFM_WRITE, &outinfo);
		outinfo.samplerate = fsout;
		chirp_out = sf_open("chirp_out.wav", SFM_WRITE, &outinfo);
		chirp_converted = sf_open("chirp_converted.wav",SFM_WRITE, &outinfo);
	}


	// initialize chirp params
	// 10s chirp from 20 Hz to fsin/2
	const double chirp_start = 1000.0;
	double slope = ((double)fsin/2 - chirp_start) / (10.0);
	double fstop = (fsin<fsout) ? fsin/2 : fsout/2;
	double fpass = bandwidth*fstop;

	printf("\n");
	printf("*******************\n");
	printf("* Checking filter *\n");
	printf("*******************\n");
	printf("\n");
	printf("Generate chirp from %0.0fHz to %iHz into chirp_in.wav (sr=%iHz)\n",chirp_start,fsin/2,fsin);
	printf("Use smarc to convert chirp_in.wav (sr=%iHz) to chirp_converted.wav (sr=%iHz)\n",fsin,fsout);
	printf("Generate ideal output chirp into chirp_out.wav (sr=%iHz)\n",fsout);
	printf("\n");

	// process chirp_in and chirp_converted
	int i = 0;
	int i_end = 10*fsin;
	int o = 0;
	double passerror = 0;
	double stoperror = 0;
	int nbtestpass = 0;
	int nbteststop = 0;
	while (i<i_end) {
		int r = 0;
		while (r<IN_BUF_SIZE && i<i_end) {
			double t = (double)i / fsin;
			double f = chirp_start + slope*t;
			inbuf[r] = 0.9 * sin(6.2831853071795865 * f * t / 2);
			++r;
			++i;
		}

		int w = smarc_resample(pfilt,pstate,inbuf,r,outbuf,OUT_BUF_SIZE);

		sf_writef_double(chirp_in,inbuf,r);
		sf_writef_double(chirp_converted,outbuf,w);

		for (r=0;r<w;r++,o++) {
			double t = (double)o / fsout;
			double f = (chirp_start + slope*t);
			double s = 0.9 * sin(6.2831853071795865 * f * t / 2);
			if (f<fpass) {
				passerror += (s-outbuf[r])*(s-outbuf[r]);
				nbtestpass++;
			}
			if (f>fstop) {
				stoperror += outbuf[r]*outbuf[r];
				nbteststop++;
			}
			outbuf[r] = s;
		}
		sf_writef_double(chirp_out,outbuf,w);
	}

	// flush last samples
	while (1) {
		int w = smarc_resample_flush(pfilt,pstate,outbuf,OUT_BUF_SIZE);
		if (w==0) break;
		sf_writef_double(chirp_converted,outbuf,w);
		for (int r=0;r<w;r++,o++) {
			double t = (double)o / fsout;
			double f = (chirp_start + slope*t);
			double s = 0.9 * sin(6.2831853071795865 * f * t / 2);
			if (f<fpass) {
				passerror += (s-outbuf[r])*(s-outbuf[r]);
				nbtestpass++;
			}
			if (f>fstop) {
				stoperror += outbuf[r]*outbuf[r];
				nbteststop++;
			}
			outbuf[r] = s;
		}
		sf_writef_double(chirp_out,outbuf,w);
	}

	// close files
	sf_close(chirp_in);
	sf_close(chirp_out);
	sf_close(chirp_converted);

	passerror = sqrt(passerror/nbtestpass);
	double passerrordb = 20*log(passerror)/log(10);
	double stoperrordb = -1000;
	printf("Compare pass band (%0.0fHz-%fHz): RMS error = %f (%0.1f dB)\n",chirp_start,fpass,passerror, passerrordb);
	if (nbteststop>0) {
		stoperror = sqrt(stoperror/nbteststop);
		stoperrordb = 20*log(stoperror)/log(10);
		printf("Compare stop band (%fHz-%fHz): RMS error = %f (%0.1f dB)\n",fstop,fsin/2.0,stoperror,stoperrordb);
	} else
		printf("No stop band to comprate in output.\n");
	printf("\n");


	printf("*******************\n");
	if (passerrordb>-40) {
		printf("* WARNING : converter pass band error (%0.1fdB) is greater than -40dB, filter seems to not efficient enough !\n",passerrordb);
	}
	if (stoperrordb>-40) {
		printf("* WARNING : converter stop band error (%0.1fdB) is greater than -40dB, filter seems to not efficient enough !\n",stoperrordb);
	}
	if (passerrordb<=-40 && stoperrordb<=-40) {
		printf("* SUCCESS !\n");
	}
	printf("*******************\n");
	printf("\n");

	// release state and buffers
	smarc_destroy_pstate(pstate);
	free(inbuf);
	free(outbuf);
}
