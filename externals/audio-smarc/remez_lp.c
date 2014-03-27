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


/********************************************************************************
 * Adaptation de l'algorithme Parks-McClellan pour les FIR de type 1 et 2
 * d'apres le livre "Theory and applications of digital signal processing" de
 * L.R. Rabiner et B. Gold et le programme fortran  "A computer program for
 * designing optimum linear phase digital filters" de J.H. McClellan, T.W. Parks
 * et L.R. Rabiner.
 ********************************************************************************/


#include "remez_lp.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define GRIDDENSITY 16
#define MAXITERATIONS 250

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

double remlpord(double f1, double f2, double d1, double d2) {
	static double A1A3[3] = {5.309e-03, 7.114e-02, -4.761e-01};
	static double A4A6[3] = {-2.660e-03, -5.941e-01, -4.278e-01};

	d1 = log10(d1);
	d2 = log10(d2);

	double lh[3] = { d1 * d1, d1, 1};
	double DD1[3] = {0,0,0};
	for (int i=0;i<3;i++)
		DD1[i] = d2*A1A3[i] + A4A6[i];
	double D = 0.0;
	for (int i = 0; i < 3; i++)
		D += DD1[i]*lh[i];
	double fK = 11.01217 + (d1 - d2) * 0.51244;
	double df = f2 - f1;
	return D / df - fK * df + 1;
}

int remez_lp_order(const double* fcuts, const double* mag, const double* dev,
		double* weight) {
	int N = ceil(remlpord(fcuts[1], fcuts[2], dev[0], dev[1]));
	double maxdev = (dev[0] > dev[1] ? dev[0] : dev[1]);
	weight[0] = maxdev / dev[0];
	weight[1] = maxdev / dev[1];
	return N;
}


void build_grid(int lgfiltre, int nfcns, const double* bands, int nbands, int lgrid, int nodd, const double* dev, const double* weight,
		double** FreqGrille, int* taillegrilledense, double** des, double** wt)
{
	int np = floor(0.5+(nfcns*bands[1]) / (0.5+(bands[1]-bands[2])));
	if (np==0)
		np++;
	int ns = nfcns+1-np;
	if (ns<=1)
		ns = 2;
	while ((np+ns-1)*lgrid+2 <= lgfiltre)
		lgrid *= 2;

	*taillegrilledense = (np+ns-1)*lgrid+2;
	*FreqGrille = malloc(*taillegrilledense * sizeof(double));
	*des = malloc(*taillegrilledense * sizeof(double));
	*wt = malloc(*taillegrilledense * sizeof(double));

	double delf = 1.0 / (lgrid * np);
	for (int k=0;k<np*lgrid+1;k++)
	{
		(*FreqGrille)[k] = k*bands[1]*delf;
		(*des)[k] = dev[0];
		(*wt)[k] = weight[0];
	}
	delf = 1.0 /((ns-1)*lgrid);
	for (int k=0;k<(ns-1)*lgrid+1;k++)
	{
		(*FreqGrille)[(np*lgrid)+1+k] = bands[2] + k*(bands[3]-bands[2])*delf;
		(*des)[(np*lgrid)+1+k] = dev[1];
		(*wt)[(np*lgrid)+1+k] = weight[1];
	}
	// Dans les cas pair on supprime le point en 0.5 ou en 0.5-eps
	if (nodd==0 && (*FreqGrille)[*taillegrilledense-1]>(0.5-delf))
		(*taillegrilledense)--;
}

/**
 *	Coefficients pour l'interpolation de Lagrange
 *	fonction D du Fortran
 */
double dd(int k,int n, int m, const double* x)
{
	double d=1;
	double q=x[k];
	for (int l=0;l<m;l++)
	{
		for (int ll=l;ll<n;ll+=m)
		{
			double xx = 2*(q -x[ll]);
			if (xx!=0)
				d *= xx;
		}
	}
	return 1.0 / d;
}

/**
 * fonction gee issu du Fortran
 */
double gee(const double* FreqGrille, const double* ad, const double* x, const double* y, const double* des, const double* wt,
		double comp,int nut,int l, int nz, double* E)
{
	double sum = 0;
	double dot = 0;
	for (int i=0;i<nz;i++)
	{
		double v = ad[i]/(cos(PI2*FreqGrille[l])-x[i]);
		sum += v;
		dot += v*y[i];
	}
	dot /= sum;
	*E = (dot - des[l])*wt[l];
	return nut*(*E) - comp;
}

/**
 * Algorithme d'echange
 * Les drapeaux flag et semaphore permmettent de remplacer les goto
 * du programme original. Il y a donc plusieurs copies des memes portions
 * de code pour des conditions differentes.
 */
int echange(int nfcns,int taillegrilledense,int* iExt,double* FreqGrille,double* des,double* wt,
		double* x, double* y, double* ad)
{
	double comp = 0.0;
	double deltal = -1;
	double delta = 0.0;
	int nz = nfcns + 1;
	int nzz = nz+1;
	int iter = 0;
	int luck = 0;
	int jchnge = 1;
	int jet = floor((nfcns-1)/15) + 1;
	double y1 = 0.0;
	double E = 0.0;
	double dtemp = 0.0;
	int k1 = 0;
	int knz = 0;
	int klow = 0;
	int kup = 0;
	int nu = 0;
	int nut = 0;
	int nut1 = 0;
	int j=1;
	int flag = 0;
	int l=0;
	while (jchnge>0)
	{
		iter++;
		if (iter>MAXITERATIONS)
			break;
		for (int i=0;i<nz;i++)
			x[i] = cos(PI2*FreqGrille[iExt[i]]);
		// coefficients ak de l'article
		for (int nn=0;nn<nz;nn++)
			ad[nn] = dd(nn,nz,jet,x);
		// Calcul de delta (dev dans le Fortran)
		{
			double dnum = 0.0;
			double dden = 0.0;
			int add = 1;
			for (int i=0;i<nz;i++)
			{
				dnum += ad[i]*des[iExt[i]];
				dden += add*ad[i]/wt[iExt[i]];
				add = -add;
			}
			delta = dnum / dden;
		}
		nu=(delta>0? -1 : 1);
		delta *= -nu;
		{
			int add = 1;
			for (int i=0;i<nz;i++)
			{
				y[i] = des[iExt[i]] + nu*delta*add/wt[iExt[i]];
				add = -add;
			}
		}
		if (delta <= deltal) // OUCH
		{
			printf("ERROR: remez exchange algorithm do not converge\n");
			printf("ERROR: Filter designed will probably be incorrect\n");
			return -1;
		}
		deltal = delta; // etiquette 150
		jchnge = 0; // on se prepare a sortir si les extrema n'ont pas change
		k1 = iExt[0];
		knz = iExt[nz-1];
		klow = -1;
		nut = -nu;
		j=1;
		while (j<nzz) // etiquette 200
		{
			kup = iExt[j];
			l = iExt[j-1] + 1;
			nut = -nut;
			if (j==2)
				y1 = comp;
			comp = delta;
			flag = 1; // test de bascule pour gerer les gotos
			if (l<kup) // on inverse le test goto 220
			{
				dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
				if (dtemp>0) // on inverse le test goto 220
				{
					comp = nut*E;
					l++; // etiquette 210
					while (l<kup) // on inverse le test goto 215
					{
						dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
						if (dtemp<=0) // test goto 215
							break;
						comp = nut*E;
						l++;
					}
					iExt[j-1] = l - 1; // etiquette 215
					j++;
					klow = l - 1;
					jchnge++; // extrema ont change
					flag = 0; // test de bascule pour le goto 200
				}
			}
			if (flag)
			{
				l -= 2; // etiquette 220 et 225
				while (l>klow) // on inverse le test goto 250
				{
					dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
					if (dtemp>0 || jchnge>0) // groupe les tests goto 230 et goto 225
						break;
					l--; // etiquette 235
				}
				if (l<=klow) // test goto 240
				{
					l = iExt[j-1] + 1; // etiquette 250
					if (jchnge>0) // test goto 215
					{
						iExt[j-1] = l-1; // on refait 215
						j++;
						klow = l-1;
						jchnge++; // extrema ont change
					} else { // sinon on a fait l-2 au lieu de l-1 (220 et 225)
						l++;
						while (l<kup) // test goto 260
						{
							dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
							if (dtemp>0)
								break;
							l++; // sinon on a fait l-2 au lieu de l-1 (220 et 225)
						}
						if (l<kup && dtemp>0) // on n'est pas passe dans le while precedent
						{
							comp = nut*E;
							l++;
							while (l<kup)
							{
								dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
								if (dtemp<=0)
									break;
								comp = nut*E;
								l++;
							}
							iExt[j-1] = l-1; // on est revenu a l >= kup
							j++;
							klow = l-1;
							jchnge++;
						} else { // if l < kup && dtemp > 0 n'est pas verifie
							klow = iExt[j-1];
							j++;
						}
					}
				} else if (dtemp>0) { // ici l > klow
					comp = nut*E;
					l--;
					while (l>klow) // tant que l > klow on fait la couble entre 200 et goto 200
					{
						dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
						if (dtemp<=0)
							break;
						comp = nut*E;
						l--;
					}
					klow = iExt[j-1];
					iExt[j-1] = l+1;
					j++;
					jchnge++;
				} else {
					klow = iExt[j-1];
					j++;
				}
			}
		}
		while (j==nzz) // etiquette 300
		{
			k1 = min(k1,iExt[0]); // if k1.GT.iExt(1)
			knz = max(knz,iExt[nz-1]); // if knz.LT.iExt(nz)
			nut1 = nut;
			nut = -nu;
			l = 0;
			kup = k1;
			comp = comp*1.00001;
			luck = 1;
			flag = 1; // test de bascule pour le goto 310
			while (l<kup) // on inverse le test goto 315
			{
				dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
				if (dtemp>0)
				{
					comp = nut*E;
					j = nzz;
					l++;
					while (l<kup)
					{
						dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
						if (dtemp<=0)
							break;
						comp = nut*E;
						l++;
					}
					iExt[j-1] = l-1;
					j++;
					klow = l-1;
					jchnge++; // extrema ont change
					flag=0; // gestion bascule
					break;
				}
				l++;
			}
			if (flag) // on fait 315 si bascule == 1
			{
				luck = 6;
				l = taillegrilledense; // etiquette 325
				klow = knz;
				nut = -nut1;
				comp = y1*1.00001;
				l--;
				while (l>klow)
				{
					dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
					if (dtemp>0) // on inverse le test pour le break dans le if suivant
					{
						j = nzz;
						comp = nut*E;
						luck += 10;
						l--; // etiquette 330
						while (l>klow) // on inverse le test goto 340
						{
							dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
							if (dtemp<=0)
								break;
							comp = nut*E;
							l--;
						}
						klow = iExt[j-1];
						iExt[j-1] = l+1;
						j++;
						jchnge++; // extrema ont change
						flag = 0; // on est passe, bascule = 0
						break;
					}
					l--;
				}
				if (flag)
				{
					if (luck != 6) // on inverse le test en 340
					{
						for (int i=nfcns-1;i>0;i--)
							iExt[i] = iExt[i-1];
						iExt[0] = k1;
						jchnge ++;
					}
					break;
				}
			}
		}
		if (j>nzz) // arrive-t-on en 320 ?
		{
			if (luck>9) // oui, et luck>9 alors goto 350
			{
				for (int i=0;i<nfcns;i++)
					iExt[i] = iExt[i+1];
				iExt[nfcns] = iExt[nz];
				jchnge++; // extrema ont change
			} else { // oui, et luck<9
				y1 = max(y1,comp);
				k1 = iExt[nzz-1];
				l = taillegrilledense;
				klow = knz;
				nut = -nut1;
				comp = y1*1.00001;
				l--;
				while (l>klow) // tant que l > klow, on fait ce qui est sous 330
				{
					dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
					if (dtemp>0)
					{
						j = nzz;
						comp = nut*E;
						luck += 10;
						l--;
						while (l > klow)
						{
							dtemp = gee(FreqGrille,ad,x,y,des,wt,comp,nut,l,nz,&E);
							if (dtemp<=0)
								break;
							comp = nut*E;
							l--;
						}
						klow = iExt[j-1];
						iExt[j-1] = l+1;
						j++;
						jchnge++;
						for (int i=0;i<nfcns;i++)
							iExt[i] = iExt[i+1];
						iExt[nfcns] = iExt[nz];
						break;
					}
					l--;
				}
				if (luck!=6) // on arrive en 340 avec semaphore = 1
				{
					for (int i=nfcns;i>0;i--)
						iExt[i] = iExt[i-1];
					iExt[0] = k1;
					jchnge ++;
				}
			}
		}
	}
	return 0;
}

void build_rep_imp(int nfcns, const double* ad, double* x, const double* y, int nodd, double* h)
{
	// Transformee de Fourier inverse
	double* a = malloc((nfcns+1)*sizeof(double));
	double* alpha = malloc((nfcns+2)*sizeof(double));
	int nz = nfcns+1;
	int nzz = nz+1;
	int nm1 = nfcns-1;
	double fsh = 1.0e-6;
	x[nzz-1] = -2;
	double cn = 2*nfcns-1;
	double delf = 1.0 / cn;
	int l=0;
	for (int j=0;j<nfcns;j++)
	{
		double ft = j*delf;
		double xt = cos(PI2*ft);
		double xe = x[l];
		// Test apres 410
		// tant que xt<=xe && (xe-xt) >= fsh
		// on boucle sur l=l+1 et xe = x(l)
		while ((xt <= xe) && ((xe-xt) >= fsh))
			xe = x[++l];
		if ((xt-xe) < fsh)
			a[j] = y[l];
		else {
			double sum = 0;
			double dot = 0;
			for (int i=0;i<nz;i++)
			{
				double v = ad[i] / (xt-x[i]);
				sum += v;
				dot += v*y[i];
			}
			a[j] = dot / sum;
		}
		if (l > 1) // etiquette 430
			l--;
	}
	double dden = PI2 / cn;
	for (int j=0;j<nfcns;j++)
	{
		double dnum = j*dden;
		if (nm1 < 1)
			alpha[j] = a[0];
		else {
			double t = a[0];
			for (int i=1;i<nfcns;i++)
				t += 2*a[i]*cos(dnum*i);
			alpha[j] = t;
		}
	}
	alpha[0] /= cn;
	for (int i=1;i<nfcns;i++)
		alpha[i] *= 2.0 / cn;
	if (nfcns <= 3)
		alpha[nfcns+1] = alpha[nfcns] = 0;

	// On convertit les alpha en reponse impulsionnelle
	// La reponse est symetrique
	if (nodd != 0)
	{
		for (int i=0;i<nfcns-1;i++)
			h[i] = 0.5*alpha[nfcns-1-i];
		h[nfcns-1] = alpha[0];
		for (int i=0;i<nfcns-1;i++)
			h[nfcns+i] = h[nfcns-2-i];
	} else {
		h[0] = 0.25*alpha[nfcns-1];
		for (int i=1;i<nfcns-1;i++)
			h[i] = 0.25 * (alpha[nfcns-1-i] + alpha[nfcns-i]);
		h[nfcns-1] = 0.5*alpha[0]+0.25*alpha[1];
		for (int i=0;i<nfcns;i++)
			h[nfcns+i] = h[nfcns-1-i];
	}

	free(a);
	free(alpha);
}

int remez_lp(double h[], int filterLen,
           double bands[], double dev[], double weight[])
{
	int lgrid = GRIDDENSITY;
	int nbands = 2; // design lowpass filter
	int nodd  = filterLen%2;
	int nfcns = filterLen/2;
	if (nodd)
		nfcns++;
	double* FreqGrille = NULL;
	int taillegrilledense = 0;
	double* des = NULL;
	double* wt = NULL;
	// remplissage de la grille, des et wt
	build_grid(filterLen, nfcns,bands,nbands,lgrid,nodd,dev,weight,&FreqGrille,&taillegrilledense,&des,&wt);
	// ponderation par q(w)=cos(w/2) si longueur paire
	if (nodd==0)
	{
		for (int i=0;i<taillegrilledense;i++)
		{
			double coeff = cos(PI * FreqGrille[i]);
			des[i] /= coeff;
			wt[i] *= coeff;
		}
	}
	// Extrema initiaux
	int* iExt = malloc((nfcns+2)*sizeof(int));
	{
		double temp = (double)(taillegrilledense-1)/nfcns;
		for (int i=0;i<nfcns;i++)
			iExt[i] = floor(i*temp);
		iExt[nfcns] = taillegrilledense-1;
		iExt[nfcns+1] = taillegrilledense;
	}
	// Algorithme d'echange
	double* x = malloc((nfcns+2)*sizeof(double));
	double* y = malloc((nfcns+1)*sizeof(double));
	double* ad = malloc((nfcns+1)*sizeof(double));
	int iRc = echange(nfcns,taillegrilledense,iExt,FreqGrille,des,wt,x,y,ad);

	if (iRc == 0)
		build_rep_imp(nfcns,ad,x,y,nodd,h);

	// release memory
	free(FreqGrille);
	free(des);
	free(wt);
	free(iExt);
	free(x);
	free(y);
	free(ad);

	return iRc;
}
