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


#include "multi_stage.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "remez_lp.h"

int find_pgcd(int a, int b) {
	int r;
	if (b > a) {
		r = a;
		a = b;
		b = r;
	}
	r = a % b;
	while (r != 0) {
		a = b;
		b = r;
		r = a % b;
	}
	return b;
}

struct PPredef {
	int Q;
	int P;
	char* def;
};

static const int s_predefs_size = 40;
// less operations
static const struct PPredef s_predefs[] = {
        { 1, 4, "2/1 2/1"},
		{ 1, 6, "2/1 3/1"},
		{ 1, 8, "2/1 2/1 2/1"},
		{ 1, 12, "2/1 2/1 3/1"},
		{ 1, 16, "2/1 2/1 2/1 2/1"},
		{ 1, 24, "2/1 2/1 2/1 3/1"},
		{ 3, 4, "2/1 2/3"},
		{ 3, 8, "2/1 4/3"},
		{ 4, 1, "1/2 1/2"},
		{ 6, 1, "1/2 1/3"},
		{ 8, 1, "1/2 1/2 1/2"},
		{ 4, 3, "3/2 1/2"},
		{ 8, 3, "1/2 3/4"},
		{ 12, 1, "1/2 1/2 1/3"},
		{ 16, 1, "1/2 1/2 1/2 1/2"},
		{ 24, 1, "1/2 1/2 1/2 1/3"},
		{ 40, 147, "1/2 3/2 7/2 7/5"},
		{ 80, 147, "7/5 7/4 3/2 1/2"},
		{ 80, 441, "3/2 3/2 7/4 7/5"},
		{ 147, 40, "2/1 2/3 2/7 5/7"},
		{ 147, 80, "5/7 4/7 2/3 2/1"},
		{ 147, 160, "4/7 2/1 4/3 5/7"},
		{ 147, 320, "4/7 4/1 4/3 5/7"},
		{ 147, 640, "2/1 4/1 4/7 4/3 5/7"},
		{ 147, 1280, "4/1 4/1 4/7 4/3 5/7"},
		{ 147, 2560, "2/1 4/1 4/1 4/7 4/3 5/7"},
		{ 160, 147, "7/5 3/4 1/2 7/4"},
		{ 160, 441, "7/5 7/4 3/2 3/2 1/2"},
		{ 320, 147, "7/4 1/4 3/4 7/5"},
		{ 320, 441, "3/4 3/4 7/4 7/5"},
		{ 441, 80, "2/3 2/3 4/7 5/7"},
		{ 441, 160, "5/7 4/7 2/3 2/3 2/1"},
		{ 441, 320, "4/7 4/3 4/3 5/7"},
		{ 441, 640, "2/1 4/7 4/3 4/3 5/7"},
		{ 441, 1280, "4/1 4/7 4/3 4/3 5/7"},
		{ 640, 147, "1/2 1/4 7/4 3/4 7/5"},
		{ 640, 441, "1/2 7/4 3/4 3/4 7/5"},
		{ 1280, 147, "1/4 1/4 7/4 3/4 7/5"},
		{ 1280, 441, "1/4 7/4 3/4 3/4 7/5"},
		{ 2560, 147, "1/2 1/4 1/4 7/4 3/4 7/5"}
};

void destroy_multistagedef(struct PMultiStageDef* pdef) {
	if (pdef) {
		if (pdef->nb_stages > 0) {
			free(pdef->L);
		}
		free(pdef);
	}
}

int compare_ratios(const void* e1, const void* e2) {
	const int* p1 = (const int*) e1;
	const int* p2 = (const int*) e2;
	if ((p1[0] > p1[1]) && (p2[0] < p2[1]))
		return -1;
	if ((p2[0] > p2[1]) && (p1[0] < p1[1]))
		return 1;
	return (p1[0] * p2[1] - p1[1] * p2[0]);
//	return 0; // keep original order
}

void reorder_stages(struct PMultiStageDef* pdef) {
	int* ratios = malloc(2 * pdef->nb_stages * sizeof(int));
	for (int s = 0; s < pdef->nb_stages; s++) {
		ratios[2 * s] = pdef->L[s];
		ratios[2 * s + 1] = pdef->M[s];
	}
	qsort(ratios, pdef->nb_stages, 2 * sizeof(int), compare_ratios);
	for (int s = 0; s < pdef->nb_stages; s++) {
		pdef->L[s] = ratios[2 * s];
		pdef->M[s] = ratios[2 * s + 1];
	}
	free(ratios);
}

void check_stages(struct PMultiStageDef* pdef) {
	// sort ratios
	int* at_end = malloc(2* pdef->nb_stages * sizeof(int));
	int end_index = 0;
	int pos_index = 0;
	int Mprod = 1;
	int Lprod = 1;
	for (int s=0;s<pdef->nb_stages;s++) {
		Mprod *= pdef->M[s];
		Lprod *= pdef->L[s];
	}
	int minPQ = (Mprod < Lprod) ? Mprod : Lprod;
	int fin = Mprod;
	Mprod = 1;
	Lprod = 1;
	for (int s=0;s<pdef->nb_stages;s++) {
		if (fin*Lprod*pdef->L[s] < minPQ*Mprod*pdef->M[s]) {
			// invalid step, put it at the end
			at_end[2*end_index] = pdef->L[s];
			at_end[2*end_index+1] = pdef->M[s];
			end_index++;
		} else {
			// keep it
			if (pos_index!=s) {
				pdef->L[pos_index] = pdef->L[s];
				pdef->M[pos_index] = pdef->M[s];
			}
			pos_index++;
			Lprod *= pdef->L[s];
			Mprod *= pdef->M[s];
		}
	}
	// report at_end
	for (int e=0;e<end_index;e++) {
		pdef->L[pos_index] = at_end[2*e];
		pdef->M[pos_index] = at_end[2*e+1];
		pos_index++;
	}
	free(at_end);
}

int factors(int value, int* res, int* resSize) {
	int i = 0;
	int p = 2;
	while (value > 1 && i < *resSize && p <= value) {
		if (value % p == 0) {
			res[i++] = p;
			value /= p;
		} else {
			// find next prime
			int notprime = 1;
			while (notprime) {
				p++;
				notprime = 0;
				for (int f = 2; f <= (p / 2); f++) {
					if (p % f == 0) {
						notprime = 1;
						break;
					}
				}
			}
		}
	}
	*resSize = i;
	return value;
}

struct PMultiStageDef* get_predef_ratios(int fsin, int fsout) {
	int pgcd = find_pgcd(fsin,fsout);
	int P = fsout / pgcd;
	int Q = fsin / pgcd;
	for (int i = 0; i < s_predefs_size; i++) {
		if ((s_predefs[i].P == P) && (s_predefs[i].Q == Q)) {
			struct PMultiStageDef* pdef = malloc(sizeof(struct PMultiStageDef));
			char* def = s_predefs[i].def;
			pdef->nb_stages = 0;
			for (int c=0;c<strlen(def);c++)
				if (def[c]=='/') pdef->nb_stages++;
			pdef->L = malloc(2 * pdef->nb_stages * sizeof(int));
			pdef->M = &pdef->L[pdef->nb_stages];
			char* pos = def;
			for (int s = 0; s < pdef->nb_stages; s++) {
				pdef->L[s] = atoi(pos);
				while (*pos!='/') pos++;
				pos++;
				pdef->M[s] = atoi(pos);
				if (s==pdef->nb_stages-1)
					break;
				while (*pos!=' ') pos++;
				pos++;
			}
			reorder_stages(pdef);
			return pdef;
		}
	}
	return NULL;
}

struct PMultiStageDef* get_user_ratios(int fsin, int fsout, const char* userdef) {
	struct PMultiStageDef* pdef = malloc(sizeof(struct PMultiStageDef));

	// compute nb stages
	pdef->nb_stages = 0;
	const char* pch = strchr(userdef, '/');
	while (pch != NULL) {
		pdef->nb_stages++;
		pch = strchr(pch + 1, '/');
	}

	// parse L M
	pdef->L = malloc(2 * pdef->nb_stages * sizeof(int));
	pdef->M = &pdef->L[pdef->nb_stages];
	pch = userdef;
	for (int s = 0; s < pdef->nb_stages; s++) {
		pdef->L[s] = atoi(pch);
		if (pdef->L[s] <= 0)
			goto error;
		pch = strchr(pch, '/');
		if (pch == NULL)
			goto error;
		pch++;
		pdef->M[s] = atoi(pch);
		if (pdef->M[s] <= 0)
			goto error;
		if (s < (pdef->nb_stages - 1)) {
			pch = strchr(pch, ' ');
			if (pch == NULL)
				goto error;
			pch++;
		}
	}
	reorder_stages(pdef);
	return pdef;

	error: printf("ERROR: cannot parse multistage user definition '%s'\n",
			userdef);
	free(pdef);
	return NULL;
}

void find_ratio(double rat, double tol, int* L, int* M) {
	double r = rat;
	int p2 = 0, q2 = 1;
	int p1 = 1, q1 = 0;
	int p, q;
	while (1) {
		int a = (int) floor(r);

		p = a * p1 + p2;
		q = a * q1 + q2;
		double c = (double) p / q;
		if (fabs(rat - c) < tol)
			break;
		r = 1.0 / (r - a);
		p2 = p1;
		q2 = q1;
		p1 = p;
		q1 = q;
	}
	*L = p;
	*M = q;
}

#define MAX_FACTORS 10

struct PMultiStageDef* build_auto_ratios(int fsin, int fsout, double tol) {
	int L = 0;
	int M = 0;
	double rat = (double) fsin / fsout;
	find_ratio(rat, tol * rat, &M, &L);

	int nbL = MAX_FACTORS;
	int* LL = malloc(nbL * sizeof(int));
	if (factors(L, LL, &nbL) != 1) {
		free(LL);
		printf("WARNING: too many factors for %i !\n", L);
		return NULL;
	}

	int nbM = MAX_FACTORS;
	int* MM = malloc(nbM * sizeof(int));
	if (factors(M, MM, &nbM) != 1) {
		free(MM);
		printf("ERROR: too many factors for %i !\n", M);
		return NULL;
	}

	if (nbL < nbM) {
		memmove(&LL[nbM - nbL], LL, nbL * sizeof(int));
		for (int i = 0; i < (nbM - nbL); i++)
			LL[i] = 1;
		nbL = nbM;
	}
	if (nbM < nbL) {
		for (int i = nbM; i < nbL; i++)
			MM[i] = 1;
		nbM = nbL;
	}

	struct PMultiStageDef* pdef = malloc(sizeof(struct PMultiStageDef));
	pdef->nb_stages = nbL;
	pdef->L = malloc(2 * nbL * sizeof(int));
	pdef->M = &pdef->L[nbL];
	for (int s = 0; s < nbL; s++) {
		pdef->L[s] = LL[s];
		pdef->M[s] = MM[s];
	}
	free(LL);
	free(MM);

	reorder_stages(pdef);
	return pdef;
}

#define MAX_INT_NUMBER (1<<30)
#define MAX_NB_STAGES 5
#define FACTOR_MAX 21
#define MAX_FREQ_FACTOR 4
#define MAX_FILTER_LENGTH 4000

void next_factor(int* current, int* remains, int factormax) {
	int N = *current * *remains;
	for (int i= 1 + *current;(i<=factormax) && (i*2<=N);i++) {
		if (N%i==0) {
			*current = i;
			*remains = N / i;
			return;
		}
	}
	if (*current!=N && N<=factormax) {
		*current = N;
		*remains = 1;
		return;
	}
	*remains = N;
	*current = 1;
	return;
}

void print_stage_def(char* msg, struct PMultiStageDef* def) {
	printf("%s ",msg);
	for (int i=0;i<def->nb_stages;i++)
		printf(" %i/%i",def->L[i],def->M[i]);
	printf("\n");
}

int find_next_stages(int L, int M, struct PMultiStageDef* def, int factormax) {
//	print_stage_def("find next stage of",def);
	int minLM = (L<M) ? L : M;
	int maxFreq = (L<M) ? M*MAX_FREQ_FACTOR : L*MAX_FREQ_FACTOR;
	// pop last stage
	int stage = def->nb_stages - 1;
	int rL = def->L[stage];
	int rM = def->M[stage];
	stage--;
	// iterate to find next valid stage with
	while (stage>=0) {
//		printf("stage %i: %i/%i rL=%i rM=%i\n",stage,def->L[stage],def->M[stage],rL,rM);
		// if no more L nor M factors we are done
		if (rL==1 && rM==1) {
			def->nb_stages = stage;
			return 1;
		}
		// if maximum number of stage has been reach we are done
		if (stage==MAX_NB_STAGES-1) {
			if (rL>factormax || rM>factormax) {
				// invalid stage, pop
				stage--;
				continue;
			}
			def->nb_stages = MAX_NB_STAGES;
			def->L[stage] = rL;
			def->M[stage] = rM;
			return 1;
		}
		// try to increase M
//		printf("try to increment M: %i %i\n",def->M[stage],rM);
		next_factor(&def->M[stage],&rM,factormax);
		if (def->M[stage] == 1) {
			// try to increase L
//			printf("try to increment L: %i %i\n",def->L[stage],rL);
			next_factor(&def->L[stage],&rL,factormax);
			if (def->L[stage]==1) {
				// pop current stage
//				printf("no more possibilities, pop stage\n");
				stage--;
				continue;
			}
		}
		// check stage is valid
		{
			int currentFS = L * rM / rL;
			if (currentFS>maxFreq)
				continue;
			if (currentFS<minLM) {
				// do not need to examine next M values
				def->M[stage] *= rM;
				rM = 1;
				continue;
			}
		}
		// push stage
//		printf("choose %i/%i\n",def->L[stage],def->M[stage]);
		stage++;
		def->L[stage] = 1;
		def->M[stage] = 1;
	}
	// no more stages found
	return 0;
}

int get_max_prime_factor(int N) {
	int maxp=1;
	while (N>maxp) {
		int p=1;
		for (int i=2;i*i<=N;i++) {
			if (N%i==0) {
				p = i;
				break;
			}
		}
		if (p==1)
			p=N;
		if (p>maxp)
			maxp = p;
		N /= p;
	}
	return maxp;
}

struct PMultiStageDef* build_fast_ratios(int fsin, int fsout, double tol, double bandwidth, double rp, double rs)
{
	int L = 0;
	int M = 0;
	double rat = (double) fsin / fsout;
	find_ratio(rat, tol * rat, &M, &L);

//	printf("build fast ratios for %i/%i\n",L,M);

	// allocate all memory contiguously
	struct PMultiStageDef* best = malloc(2*sizeof(struct PMultiStageDef) + 4*MAX_NB_STAGES*sizeof(int));
	best->L = (int*) (best + 1);
	best->M = best->L + MAX_NB_STAGES;
	struct PMultiStageDef* current = (struct PMultiStageDef*) (best->M + MAX_NB_STAGES);
	current->L = (int*) (current + 1);
	current->M = (int*) (current->L + MAX_NB_STAGES);

	// check factor_max
	int factor_max = FACTOR_MAX;
	{
		int max_prime = get_max_prime_factor(L);
		if (factor_max<max_prime)
			factor_max=max_prime;
		max_prime = get_max_prime_factor(M);
		if (factor_max<max_prime)
			factor_max=max_prime;
	}

	// initialize vars for computing filter orders
	double *bands, *mag, *dev, *weight;
	bands = malloc(10 * sizeof(double));
	mag = &bands[4];
	dev = &mag[2];
	weight = &dev[2];

	bands[0] = 0;
	bands[1] = 0; // fpass
	bands[2] = 0.5; // fstop
	bands[3] = 0.5;

	mag[0] = 1;
	mag[1] = 0;

	double fstop = (fsin<fsout) ? (double)fsin/2 : (double)fsout/2;
	double fpass = fstop * bandwidth;

	// iterate over all stages
	double best_ops = MAX_INT_NUMBER;
	current->nb_stages = 2;
	current->L[0] = 1;
	current->M[0] = 1;
	current->L[1] = L;
	current->M[1] = M;
	int nbTest = 0;
	while (find_next_stages(L,M,current,factor_max)) {
//		print_stage_def("got stage ",current);
		nbTest++;
		// compute ops
		dev[0] = (pow(10, rp / 20.0) - 1) / (current->nb_stages *(pow(10, rp / 20.0) + 1));
		dev[1] = pow(10, -rs / 20.0);
		double fs = (double) fsin;
		double ops = 0.0;
		for (int s=0;s<current->nb_stages;s++) {
			const int cL = current->L[s];
			const int cM = current->M[s];
			double fmax = fs * cL;
			bands[1] = fpass / fmax;
			bands[2] = (cL>cM) ? (fs-fstop) / fmax : (fs*cL/cM - fstop) / fmax;
			int flen = remez_lp_order(bands, mag, dev, weight);
			{
				int k=(flen-1)/(2*cM);
				while (2*k*cM+1 < flen)
					k++;
				flen = 2*k*cM+1;
			}
			if (flen>MAX_FILTER_LENGTH)
			{
//				printf("- %i/%i filter too long !\n",cL,cM);
				ops = MAX_INT_NUMBER;
				break;
			}
			double stage_ops = flen*fs / cM;
//			printf("- %i/%i : flen=%i ops=%0.2f\n",cL,cM,flen,stage_ops);
			ops += stage_ops;
			fs = (fs * cL) / cM;
		}
//		printf("total ops = %0.2f\n",ops);
		if (ops<best_ops) {
//			printf("best one !\n");
			best_ops = ops;
			best->nb_stages = current->nb_stages;
			for (int i=0;i<current->nb_stages;i++) {
				best->L[i] = current->L[i];
				best->M[i] = current->M[i];
			}
		}
	}
//	printf("have test %i combinaison\n",nbTest);
//	print_stage_def("best one is",best);
//	printf("width %0.2f operations\n",best_ops);

	// prepare output
	struct PMultiStageDef* pdef = malloc(sizeof(struct PMultiStageDef));
	pdef->nb_stages = best->nb_stages;
	pdef->L = malloc(2 * best->nb_stages * sizeof(int));
	pdef->M = &pdef->L[best->nb_stages];
	for (int s = 0; s < pdef->nb_stages; s++) {
		pdef->L[s] = best->L[s];
		pdef->M[s] = best->M[s];
	}

	// free working memory
	free(best);
	free(bands);

	return pdef;
}

