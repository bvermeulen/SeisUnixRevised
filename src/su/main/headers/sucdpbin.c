/* Copyright (c) Colorado School of Mines, 2011.*/
/* All rights reserved.                       */
/* correction made for generating the cmp line, bruno.vermeulen@hotmail.com */

#include "su.h"
#include "segy.h"

/*********************** self documentation **********************/
char *sdoc[] = {
" 									",
" SUCDPBIN - Compute CDP bin number					",
" 									",
" sucdpbin <stdin >stdout xline= yline= dcdp=				",
" 									",
" Required parameters:							",
" xline=		array of X defining the CDP line		",
" yline=		array of Y defining the CDP line		",
" dcdp=			distance between bin centers			",
"									",
" Optional parameters							",
" verbose=0		<>0 output informations				",
" cdpmin=1001		min cdp bin number				",
" distmax=dcdp		search radius					",
" 									",
" xline,yline defines the CDP line made of continuous straight lines. 	",
" If a smoother line is required, use unisam to interpolate.		",
" Bin centers are located at dcdp constant interval on this line. 	",
" Each trace will be numbered with the number of the closest bin. If no  ",
" bin center is found within the search radius. cdp is set to 0		",
"									",
NULL};

/* Credits:
 * 2009 Dominique Rousset - Mohamed Hamza
 *      Université de Pau et des Pays de l'Adour (France)
 */

/**************** end self doc ***********************************/

segy tr;

int
main(int argc, char **argv)
{
	double *xline = NULL; /* array of binline vertices in x	*/
	double *yline = NULL; /* ...    binline vertices in x 	*/
	double *segment_lengths = NULL;
	double dcdp;	/* distance between bin centers */
	double *xbin = NULL, *ybin = NULL;
	int *cbin = NULL;
	double xmp, ymp, dist, distmin;
	double distmax;
	int nxline, nyline, nbin, total_trace_count;
	int verbose;
	int ipoint;
	int cdp_min;
	double scale;
	int max_bin, ibin;

	/* Initialize */
	initargs(argc, argv);
	requestdoc(1);

	/* Get parameters */
	if (!(nxline = countparval("xline"))) {
		err("must give xline= vector");
	}
	if (!(nyline = countparval("yline"))) {
		err("must give yline= vector");
	}
	if (nxline != nyline) {
		err("lengths of xline, yline must be the same");
	}
	/* allocate space */
	xline = ealloc1double(nxline);
	yline = ealloc1double(nyline);
	segment_lengths = ealloc1double(nxline);
	getpardouble("xline", xline);
	getpardouble("yline", yline);

	if (!getpardouble("dcdp", &dcdp)) {
		err("must give dcdp");
	}
	if (!getparint("cdpmin", &cdp_min)) {
		cdp_min = 1001;
	}
	if (!getpardouble("distmax", &distmax)) {
		distmax = dcdp;
	}
	if (!getparint("verbose", &verbose)) {
		verbose = 0;
	}
	checkpars();

	if (verbose > 1) {
		warn ("%d points on the line", nxline);
		ipoint=0 ;
		do  {
			warn ("point # %d x= %f y= %f", ipoint, xline[ipoint], yline[ipoint]);
			++ipoint;
		} while (ipoint < nxline);
	}

	/* Process by distance along the polyline, not by x-coordinate */
	float total_length = 0;
	for (int i = 0; i < nxline-1; i++) {
    	float dx_seg = xline[i+1] - xline[i];
    	float dy_seg = yline[i+1] - yline[i];
    	segment_lengths[i] = sqrt(dx_seg*dx_seg + dy_seg*dy_seg);
    	total_length += segment_lengths[i];
	}

	max_bin= total_length / dcdp + 1;
	xbin = ealloc1double(max_bin+1);
	ybin = ealloc1double(max_bin+1);
	cbin = ealloc1int(max_bin+1);
	ibin = 0;

	if (verbose > 1)
	{
		warn("ibin=%d x=%f y=%f ", ibin, xbin[ibin], ybin[ibin]);
	}
	
	/* Sample at regular distance intervals along the entire curve */
	int current_seg = 0;
	float current_dist = 0.0;
	float seg_start_dist = 0.0;
	 
	while (current_dist <= total_length) {
	    /* Find which segment we're in */
	    while (current_seg < nxline-1 && current_dist > seg_start_dist + segment_lengths[current_seg]) {
	        seg_start_dist += segment_lengths[current_seg];
	        current_seg++;
	    }
        /* Interpolate within the segment */
	    float dist_fraction = (current_dist - seg_start_dist) / segment_lengths[current_seg];
	    
		xbin[ibin] = xline[current_seg] + dist_fraction * (xline[current_seg+1] - xline[current_seg]);
	    ybin[ibin] = yline[current_seg] + dist_fraction * (yline[current_seg+1] - yline[current_seg]);
       	ibin++;
    	current_dist += dcdp;  /* Step by fixed distance */
	}
	nbin=ibin;

	if (verbose > 1) {
		warn ("length of the line :%f ", total_length);
		warn ("maximum number of bin:%d", max_bin);
		warn ("actual number of bin:%d", nbin);
	}

	/* Get info from first trace */
	if (!gettr(&tr)) err("can't read first trace");

	/* Loop over traces */
	do {
		if (tr.scalco < 0 )
			scale=1./abs(tr.scalco);
		else if (tr.scalco > 0)
			scale=tr.scalco;
		else {
			warn ("scalco = 0 ; 1 assumed") ;
			scale=1;
		}
		xmp = (tr.gx + tr.sx) * 0.5 * scale;
		ymp = (tr.gy + tr.sy) * 0.5 *scale;
		distmin = 2 * distmax;

		for (ibin=0 ; ibin < nbin ; ++ibin) {
			dist = sqrt(
				(xmp - xbin[ibin]) * (xmp - xbin[ibin]) + 
				(ymp - ybin[ibin]) * (ymp - ybin[ibin])
			);
			if (dist < distmin) {
				distmin = dist;
				tr.cdp = ibin + cdp_min;
			}
		}

		if (distmin > distmax)
			tr.cdp=0;

		else {
			ibin = tr.cdp - cdp_min;
			cbin[ibin]++;
			if (verbose > 2) {
				distmin = sqrt(pow(xmp - xbin[ibin], 2) + pow(ymp - ybin[ibin], 2));
				warn("ep=%d cdp=%d distmin=%f", tr.ep, tr.cdp, distmin);
			}
		}

		puttr(&tr);
	} while (gettr(&tr));

	total_trace_count = 0;
	if (verbose > 0) {
		for (ibin=0; ibin < nbin ; ++ibin) {
			total_trace_count += cbin[ibin];
			warn("cdp: %d (%6.0f, %6.0f), bincount: %d ", ibin + cdp_min, xbin[ibin], ybin[ibin], cbin[ibin]);
		}
		warn("Total trace count: %d", total_trace_count);
	}

	return(CWP_Exit());
}
