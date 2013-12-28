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

#include "smarc.h"
#include "stage_impl.h"
#include "multi_stage.h"
#include "polyfilt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FIRST_BUFFER_SIZE 512

struct PFilter
{
	int fsin;
	int fsout;
	double fpass;
	double fstop;
	double rp;
	double rs;
	int nb_stages;
	struct PSFilter** filter;
};

int smarc_get_fs_in(struct PFilter* pfilt)
{
	return pfilt->fsin;
}

int smarc_get_fs_out(struct PFilter* pfilt)
{
	return pfilt->fsout;
}

int smarc_get_output_buffer_size(struct PFilter* pfilt,int inSize)
{
	int outSize = 1 + (int) ceil((double)inSize * (double) pfilt->fsout / (double) pfilt->fsin);
	double stage_fsout = pfilt->fsin;
	for (int i=0;i<pfilt->nb_stages;i++)
	{
		stage_fsout *= (double)pfilt->filter[i]->L / (double)pfilt->filter[i]->M;
		outSize += ceil( pfilt->fsout * pfilt->filter[i]->filter_delay / stage_fsout);
	}
	return outSize;
}


struct PFilter* smarc_init_pfilter(int fsin, const int fsout, double bandwidth, double rp, double rs, double tol, const char* userratios, int searchfastconversion)
{
    if (fsout==fsin)
    {
        printf("ERROR: in and out samplerates are equals ! (%i Hz)\n",fsin);
        return NULL;
    }

	struct PMultiStageDef* pdef;
	if (userratios!=NULL && strlen(userratios)>0)
	{
		pdef = get_user_ratios(fsin,fsout,userratios);
		if (!pdef)
			return NULL;
	} else if (searchfastconversion) {
		pdef = build_fast_ratios(fsin,fsout,tol,bandwidth,rp,rs);
	} else {
		pdef = get_predef_ratios(fsin,fsout);
		if (!pdef)
		{
			pdef = build_auto_ratios(fsin,fsout, tol);
		}
	}

	if (!pdef)
	{
		printf("ERROR: cannot design multistage samplerate converter ! try to increase tolerance or define it yourself.\n");
		return NULL;
	}

	struct PFilter* pfilt = malloc(sizeof(struct PFilter));
	pfilt->fsin = fsin;
	pfilt->fsout = fsout;
	pfilt->rp = rp;
	pfilt->rs = rs;

	pfilt->nb_stages = pdef->nb_stages;
	pfilt->filter = malloc(pfilt->nb_stages*sizeof(struct PSFilter*));

	pfilt->fstop = (fsin>fsout ? fsout/2 : fsin/2);
	double fpass = bandwidth*pfilt->fstop;
	pfilt->fpass = fpass;
	double stage_fsin = fsin;
	for (int i=0;i<pdef->nb_stages;i++)
	{
		double fstop = 0;
		double fmax = pdef->L[i]*stage_fsin;
		if (pdef->L[i] > pdef->M[i])
		{
			// interpolation
			fstop = stage_fsin - pfilt->fstop;
		} else {
			// decimation
			fstop = ((stage_fsin * pdef->L[i]) / pdef->M[i]) - (pfilt->fstop);
		}
		pfilt->filter[i] = init_psfilter(pdef->L[i],pdef->M[i],
				fpass / fmax,fstop / fmax,rp,rs,pdef->nb_stages);
		if (pfilt->filter[i]==NULL)
			return NULL;
		stage_fsin = (stage_fsin * pdef->L[i]) / pdef->M[i];
	}
	if (fabs(stage_fsin - fsout) > tol*fsin)
	{
		printf("ERROR: multistage filter output %f != %i ! (there should be an error in multistage definition)\n", stage_fsin,fsout);
		return NULL;
	} else if (stage_fsin!=fsout)
	{
		printf("WARNING: output samplerate is %f\n",stage_fsin);
	}

	destroy_multistagedef(pdef);

	return pfilt;
}

void smarc_destroy_pfilter(struct PFilter* pfilt)
{
	for (int i=0;i<pfilt->nb_stages;i++)
		destroy_psfilter(pfilt->filter[i]);
	free(pfilt->filter);
	free(pfilt);
}

void smarc_print_pfilter(struct PFilter* pfilt)
{
	printf("multi-stage polyphase resample from %iHz to %iHz\n",pfilt->fsin,pfilt->fsout);
	printf("  passband to %0.2fHz, passband ripple factor %0.2fdB\n",pfilt->fpass,pfilt->rp);
	printf("  stopband from %0.2fHz, stopband ripple factor %0.2fdB\n", pfilt->fstop,pfilt->rs);
	printf("successive resample stages are :\n");
	for (int s=0;s<pfilt->nb_stages;s++)
		printf("  %i / %i : filter length = %i, delay = %i\n",pfilt->filter[s]->L,pfilt->filter[s]->M,pfilt->filter[s]->flen,pfilt->filter[s]->filter_delay);
}

struct PStageBuffer
{
	double* data;
	int size;
	int pos;
};

struct PState
{
	int nb_stages;
	struct PSState** state;
	struct PStageBuffer** buffer;
	// flush vars
	double* flush_buf;
	int flush_size;
	int flush_pos;
	int flush_stage;
};

struct PState* smarc_init_pstate(struct PFilter* pfilt)
{
	struct PState* pstate = malloc(sizeof(struct PState));
	pstate->nb_stages = pfilt->nb_stages;
	pstate->flush_buf = NULL;

	// init states
	pstate->state = malloc(pstate->nb_stages*sizeof(struct PSState*));
	for (int i=0;i<pstate->nb_stages;i++)
		pstate->state[i] = init_psstate(pfilt->filter[i]);

	// init buffers
	pstate->buffer = malloc((pstate->nb_stages+1)*sizeof(struct PStageBuffer*));
	int total_size = 0;
	int current_buffer_size = 0;
	for (int i=0;i<pstate->nb_stages+1;i++)
	{
		struct PStageBuffer* cbuf = malloc(sizeof(struct PStageBuffer));
		pstate->buffer[i] = cbuf;
		if (i==0) {
			current_buffer_size = FIRST_BUFFER_SIZE;
		}
		else {
			current_buffer_size = current_buffer_size * pfilt->filter[i-1]->L / pfilt->filter[i-1]->M + 1;
		}
		int filter_len = 0;
		if (i<pstate->nb_stages)
			filter_len = pfilt->filter[i]->K - 1;
		cbuf->size = current_buffer_size + filter_len;
		cbuf->pos = 0;
//		printf("buffer %i has buffer size %i + %i = %i \n",i,current_buffer_size,filter_len,cbuf->size);
		total_size += cbuf->size;
	}

	// allocate all buffer contiguously
	pstate->buffer[0]->data = (double*) malloc(total_size*sizeof(double));
	for (int i=1;i<pstate->nb_stages+1;i++)
		pstate->buffer[i]->data = pstate->buffer[i-1]->data + pstate->buffer[i-1]->size;

	// reset pstate before returning it
	smarc_reset_pstate(pstate,pfilt);
	return pstate;
}

void smarc_destroy_pstate(struct PState* pstate)
{
	for (int i=0;i<pstate->nb_stages;i++)
		destroy_psstate(pstate->state[i]);
	free(pstate->buffer[0]->data);
	for (int i=0;i<pstate->nb_stages+1;i++)
		free(pstate->buffer[i]);
	if (pstate->flush_buf)
		free(pstate->flush_buf);
	free(pstate->buffer);
	free(pstate);
}

void smarc_reset_pstate(struct PState* pstate, struct PFilter* pfilt)
{
	for (int i=0;i<pstate->nb_stages;i++)
		reset_psstate(pstate->state[i],pfilt->filter[i]);
	for (int i=0;i<pstate->nb_stages;i++) {
		struct PStageBuffer* buf = pstate->buffer[i];
		buf->pos = pfilt->filter[i]->K - 1;
		for (int k=0;k<buf->pos;k++)
			buf->data[k] = 0;
	}
	pstate->buffer[pstate->nb_stages]->pos = 0;
	if (pstate->flush_buf) {
		free(pstate->flush_buf);
		pstate->flush_buf = NULL;
	}
	pstate->flush_stage = 0;
	pstate->flush_pos = 0;
	pstate->flush_size = 0;
}

int smarc_resample(struct PFilter* pfilt, struct PState* pstate,
		const double* signal,
		int signalLength,
		double* output,
		int outputLength)
{
	int nbRead = 0;
	int nbWritten = 0;
	unsigned char inputRemains = 1; // use it as a flag
	while (inputRemains && nbWritten<outputLength) {
		inputRemains = 0;
//		printf("process signal %i/%i, written %i/%i\n",nbRead,signalLength,nbWritten,outputLength);
		// fill first buffer
		{
			struct PStageBuffer* fbuf = pstate->buffer[0];
			int toRead = fbuf->size - fbuf->pos;
			if (toRead>signalLength-nbRead)
				toRead = signalLength - nbRead;
			else
				inputRemains = 1;
//			printf("Push %i sample in first buffer %i/%i\n",toRead,fbuf->pos,fbuf->size);
			if (toRead>0) {
				memcpy(fbuf->data + fbuf->pos, signal + nbRead, toRead*sizeof(double));
				fbuf->pos += toRead;
				nbRead += toRead;
			}
		}
		// process all stages
		for (int i=0;i<pfilt->nb_stages;i++)
		{
			struct PSFilter* filt = pfilt->filter[i];
			struct PSState* state = pstate->state[i];
			struct PStageBuffer* inbuf = pstate->buffer[i];
			struct PStageBuffer* outbuf = pstate->buffer[i+1];
			int nbStageRead;
			int nbStageWritten;
			polyfiltLM(filt,state,inbuf->data,inbuf->pos,&nbStageRead,outbuf->data + outbuf->pos,outbuf->size - outbuf->pos,&nbStageWritten);

//			printf("stage %i: read %i [%i/%i] write %i [%i/%i] K=%i\n",i,nbStageRead,inbuf->pos,inbuf->size,nbStageWritten,outbuf->pos,outbuf->size,filt->K);

			// keep non processed input
			if (nbStageRead<inbuf->pos) {
				memmove(inbuf->data, inbuf->data + nbStageRead, (inbuf->pos - nbStageRead)*sizeof(double));
			}
			inbuf->pos -= nbStageRead;
			if (inbuf->pos>filt->K-1)
				inputRemains = 1;

			// update output pos
			outbuf->pos += nbStageWritten;
		}
		// report last buffer to output
		{
			struct PStageBuffer* lbuf = pstate->buffer[pstate->nb_stages];
			int toWrite = lbuf->pos;
			if (nbWritten + toWrite >= outputLength) {
				printf("WARNING: cannot write all output samples, please provide larger output buffer !");
				toWrite = outputLength - nbWritten;
			}
//			printf("write %i samples from last buf %i/%i into output buffer %i/%i\n",toWrite,lbuf->pos,lbuf->size,nbWritten,outputLength);
			if (toWrite>0)
				memcpy(output + nbWritten,lbuf->data,toWrite*sizeof(double));
			if (toWrite<lbuf->pos)
				memmove(lbuf->data,lbuf->data+toWrite,(lbuf->pos-toWrite)*sizeof(double));
			nbWritten += toWrite;
			lbuf->pos -= toWrite;
		}
	}
	return nbWritten;
}

int smarc_resample_flush(struct PFilter* pfilt, struct PState* pstate,
		double* output,
		int outputLength)
{
	int nbWritten = 0;
	// flush all stages
	while (pstate->flush_stage<pfilt->nb_stages && nbWritten<outputLength)
	{
//		printf("flushing stage %i [%i/%i]\n",pstate->flush_stage,pstate->flush_pos,pstate->flush_size);
		struct PSFilter* filt = pfilt->filter[pstate->flush_stage];
		struct PStageBuffer* inbuf = pstate->buffer[pstate->flush_stage];
		if (pstate->flush_buf==NULL) {
			int toFlush = filt->K - 1 - inbuf->pos + (filt->filter_delay * filt->M) / filt->L;
			if (toFlush<(inbuf->size-inbuf->pos)) {
//				printf("flushing straight %i samples\n",toFlush);
				// just write flush samples into buffer
				for (int k=0;k<toFlush;k++)
					inbuf->data[inbuf->pos+k] = inbuf->data[inbuf->pos - 2 - k];
				inbuf->pos += toFlush;
			} else {
				// remember samples to flush
				pstate->flush_buf = (double*) malloc(toFlush*sizeof(double));
				pstate->flush_size = toFlush;
				for (int k=0;k<toFlush;k++)
					pstate->flush_buf[k] = inbuf->data[inbuf->pos - 2 - k];
				// fill inbuf
				for (int k=0;k<inbuf->size-inbuf->pos;k++)
					inbuf->data[inbuf->pos+k] = pstate->flush_buf[k];
//				printf("flushing %i/%i samples\n", inbuf->size-inbuf->pos, toFlush);
				pstate->flush_pos = inbuf->size-inbuf->pos;
				inbuf->pos = inbuf->size;
			}
		} else {
			// continue flushing
			int toWrite = inbuf->size - inbuf->pos;
			if (toWrite> (pstate->flush_size-pstate->flush_pos))
				toWrite = pstate->flush_size-pstate->flush_pos;
			for (int k=0;k<toWrite;k++)
				inbuf->data[inbuf->pos+k] = pstate->flush_buf[pstate->flush_pos+k];
//			printf("flushing next %i samples starting at %i/%i\n",toWrite,pstate->flush_pos,pstate->flush_size);
			pstate->flush_pos += toWrite;
			inbuf->pos += toWrite;
		}

		// process filtering
		nbWritten += smarc_resample(pfilt,pstate,NULL,0,output + nbWritten, outputLength - nbWritten);

		// check if all have been read
		if ((inbuf->pos<filt->K) && (pstate->flush_pos==pstate->flush_size)) {
			// end flushing this stage
			if (pstate->flush_buf) {
				free(pstate->flush_buf);
				pstate->flush_buf = NULL;
				pstate->flush_pos = 0;
				pstate->flush_size = 0;
			}
			pstate->flush_stage++;
		}
	}
	return nbWritten;
}





