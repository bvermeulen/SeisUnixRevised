/* SUCWIGNMAX: $Revision: 1.4 $ ; $Date: 90/10/19 10:33:39 $		*/

/*----------------------------------------------------------------------
 * Copyright (c) Colorado School of Mines, 1990.
 * All rights reserved.
 *
 * This code is part of SU.  SU stands for Seismic Unix, a processing line
 * developed at the Colorado School of Mines, partially based on Stanford
 * Exploration Project (SEP) software.  Inquiries should be addressed to:
 *
 *  Jack K. Cohen, Center for Wave Phenomena, Colorado School of Mines,
 *  Golden, CO 80401  (jkc@dix.mines.colorado.edu)
 *----------------------------------------------------------------------
 */

#include "su.h"
#include "segy.h"

/*********************** self documentation ******************************/
string sdoc = "\
									\n\
SUCWIGNMAX - wiggle plot of su data traces above a graph of 		\n\
	    trace-by-trace maximum amplitude (optional running average)	\n\
									\n\
sucwignmax <data [optional parameters] | pen				\n\
									\n\
Optional Parameters:							\n\
	title   = null		plot title				\n\
	label1	= Time 		time axis label				\n\
	label2	= Trace		trace axis label 			\n\
	fill	= 1		flag for positive fill (=0 for no fill)	\n\
	m	= 1		running average length for amp plot	\n\
				( must be odd, positive integer )	\n\
	xmin    = 1		tic label for first trace		\n\
	dx      = 1		tic labeling increment			\n\
	max   	= (from data)	max on amplitude graph			\n\
	sizet	= 6.0		horiz plot size (inches)		\n\
	sizex	= 4.5		horiz plot size (inches)		\n\
	zerot	= 0.75		vertical offset (inches)		\n\
	zerox	= 0.7		horizontal offset (inches)		\n\
	ntict   = 5		# tics on time axis of wiggle plot	\n\
	nticx   = 4		# tics on trace axis of both plots	\n\
	ntica   = 2		# tics on amplitude axis of graph plot	\n\
	hcopy	= 0		honors user/default parameters		\n\
				= 1   forces thesis format		\n\
";
/**************** end self doc ***********************************/

/* Credits:
 *	CWP: Chris
 *
 * Caveats:
 *	The graph sub is very messy because user (x,y)
 *	is internal (t,x)...
 */


/* Set gain defaults (default scale to largest amp) */
#define TPOW	0.0
#define EPOW	0.0
#define GPOW	1.0
#define AGC 	0
#define WAGC	20
#define TRAP	0.0
#define CLIP	0.0
#define QCLIP	1.0
#define QBAL	1	
#define PBAL	0

segy tr;

main(int argc, char **argv)
{
	float *data;		/* mega-vector to contain data set	*/
	float givenmax;		/* user given maximum for plotting	*/
	float gmax1;		/* global absolute max before gaining	*/
	float gmax2;		/* global absolute max after gaining	*/
	float *trmaxs;		/* collection of trace-by-trace maxs	*/
	float *tmpvec1;		/* temporary vector			*/
	float *tmpvec2;		/* temporary vector			*/
	float absmax;		/* absolute max for tmpvec1		*/
	float scale = 1.0;	/* scale for gaining			*/
	float tmin, dt;		/* for gain sub 	*/
	float xmin, dx;		/* for gain sub 	*/
	float sizex, sizet;	/* */
	float zerox, zerot;	/* */
	string title; 
	string label1; 
	string label2;
	int m;			/* running average length for amp curve	*/
	int j;
	int hcopy;
	int itr;
	int ndata;		/* allocation parameter			*/
	int nfloats;		/* number of floats in the data set	*/
	int nt;			/* length of input traces		*/
	int ntsize;		/* ... in bytes				*/
	int ntr;		/* traces in input data			*/
	int tracl2;		/* traces in input data			*/
	register int i;		/* counter				*/
	void subplotg();	/* isolate cplot commands		*/
	void subplotw();	/* isolate cplot commands		*/

	/* Initialize */
	initargs(argc, argv);
	askdoc(1);


	/* Prevent bytes from spilling onto screen */
	if (isatty(STDOUT)) {
		err("must redirect or pipe byte code output");
	}


	/* Get info from first trace	*/ 
	/* Read first trace */ 
	if (!gettr(&tr)) err("can't get first trace");

	/* Get number of time samples & calc some constants */	
	nt = tr.ns; 
	xmin = tr.tracl;	fgetpar("xmin", &xmin);
	ntsize = nt * FSIZE;

	tmin = tr.delrt/1000.0;	fgetpar("tmin", &tmin);
				/* tr.delrt is in millisecs */

	if (!fgetpar("dt", &dt)) {
	    if (tr.dt) {  /* is dt field set? */
		    dt = tr.dt / 1000000.0;
	    } else {		/* dt not set, assume 4 ms */
		    dt = 0.004;
		    warn("tr.dt not set, for labeling assume dt=%g", dt);
	    }
	}
	

	/* Allocate block of memory for data float mega-vector */
	ndata = MAX(NFALLOC, nt); /* alloc at least one trace */
	data = ealloc1float(ndata);


	/* Loop over input traces & put them into data mega-vector */
	ntr = 0;
	do {
		++ntr;
		if (ntr == 2) tracl2 = tr.tracl;/* needed for dx    */
		if (ntr*nt > ndata) {		/* need more memory */
			ndata += NFALLOC;
			data = erealloc1float(data, ndata);
		}
		bcopy(tr.data, data + (ntr - 1)*nt, ntsize); 
	} while(gettr(&tr));

	dx = tracl2 - xmin;	fgetpar("dx", &dx);


	/* Find max value in data before gain for labels */
	nfloats = nt*ntr;
	gmax1 =  ABS(data[0]);
	for (i = 1; i < nfloats; ++i)
		gmax1 = MAX(gmax1, ABS(data[i]));


	/* Check if user has given a max */
	givenmax = 0.0; 	fgetpar("max", &givenmax);
	if (givenmax) {
		scale = gmax1 / givenmax;
		gmax1 = givenmax;
	} 


	/* Normalize data to unity for wgl1 subroutine */
	gain(data, TPOW, EPOW, GPOW, AGC, TRAP, CLIP, QCLIP,
			QBAL, PBAL, scale, tmin, dt, WAGC, nt, ntr);

	/* Find global max value in data after gain (should be one) */
	gmax2 =  ABS(data[0]);
	for (i = 1; i < nfloats; ++i)
		gmax2 = MAX(gmax2, ABS(data[i]));


	/* Alloc space for vector of trace maxima */
	trmaxs = ealloc1float(ntr);
	tmpvec1 = ealloc1float(nt);
	tmpvec2 = ealloc1float(ntr);


	/* Build a 'trace' of maximum value on each trace */
	/* trace is nt long, so loop over nt samples      */
	for (itr = 0; itr < ntr ; itr++) { 
		bcopy(data + itr*nt, tmpvec1, ntsize); 
		absmax =  ABS(tmpvec1[0]);
		for (i = 1; i < nt; ++i)
			absmax = MAX(absmax, ABS(tmpvec1[i]));
		trmaxs[itr] = absmax; 
	} 


	/* get running average parameter */
	if (!igetpar("m", &m))	m = 1;
	if ( m <= 0 ) err("m = %d ... must be odd, positive integer/n",m);

	/* Compute m-point running average of max amp curve */
	for (itr = 0; itr < ntr ; itr++) { 
	    if ( itr - (m+1)/2 + 1 >= 0 && itr + (m+1)/2 -1 < ntr ) {
	        for ( j = -(m+1)/2 + 1 ; j <= (m+1)/2 - 1 ; j++ ) {
		    tmpvec2[itr] = tmpvec2[itr] + trmaxs[itr + j] / m; 
	        } 
	    } else {
	        tmpvec2[itr] = trmaxs[itr]; 
	    } 
	} 


	/* Echo some info to user */
	warn("nt=%d   ntr=%d", nt, ntr);	
	if (givenmax) {
		warn("global data max = %g    given plot max = %g",
			scale*givenmax, givenmax);	
	} else {
		warn("global max = %g ", gmax1, gmax2);	
 	}


	/* Plot getpars */
	title = "Title";		sgetpar("title",  &title);
	label1 = "Time";		sgetpar("label1", &label1);
	label2 = "";		

	sizet = 6.4;		fgetpar("sizet", &sizet); 
	sizex = 4.5;		fgetpar("sizex", &sizex);
	zerot = 0.3;		fgetpar("zerot",&zerot); 
	zerox = 0.7;		fgetpar("zerox",&zerox); 

	if (!igetpar("hcopy", &hcopy))		hcopy = 0;
	if (hcopy) {
	   	sizet = 5;
	   	sizex = 4.4;
	   	zerot = 1.2;
	   	zerox = 1.0;
	}

	/* Wiggle plot */
	subplotw(data,nt, ntr, title, label1, label2, tmin, 
			dt, xmin, dx, sizex, sizet, zerox, zerot);   


	/* Plot getpars */
	title = "";	
	label1 = "Max Amp";		
	label2 = "Trace";		sgetpar("label2", &label2);


	/* Graph plot */
	nt = ntr;
	ntr = 1;
	tmin = xmin;
	dt = dx;
	subplotg(tmpvec2, nt, ntr, title, label1, label2, 
			gmax1, gmax2, tmin, dt, sizex, sizet, zerox, zerot );   

	endplot ();


	return EXIT_SUCCESS;
}

/* plot subroutine for amplitude graphing */
void subplotg(data,nt,nx,title,label1,label2,truemax,
		max,tmin,dt,sizet,sizex,zerot,zerox)
string title, label1, label2;
int nt,nx;
float tmin,dt,*data,truemax,max,sizet,sizex, zerot, zerox;
{
	int i,plotfat,axisfat,dtict,ntict,nticx;
	float scalet,scalex,margint,marginx,dticx;
	float truedticx;
	int titlsz,lablsz,ticsz;
	char tval[8];		/* tic value string			*/

/* 	vertical size of amp graph is 30% total height */
	sizex = sizex * .3;

/* 	zerot = 0.5;		fgetpar("zerox",&zerot); */
/* 	zerox = 0.7;		fgetpar("zeroy",&zerox); */

	plotfat = 1;		igetpar("plotfat",&plotfat);
	axisfat = 0;		igetpar("axisfat",&axisfat);

	nticx = 2; 		igetpar("ntica",&nticx);
	ntict = 4;		igetpar("nticx",&ntict);

	margint = 0.01;		fgetpar("marginx",&margint);
	marginx = 0.09;		fgetpar("marginy",&marginx);

	titlsz = 3;		igetpar("titlsz",&titlsz);
	lablsz = 3;		igetpar("lablsz",&lablsz);
	ticsz = 2;		igetpar("ticsz",&ticsz);

	scalet = sizet/nt;	
	scalex = 0.5*sizex;
	setscl(scalet,scalex);
	set0(zerot, zerox + 0.5*sizex );
	setu0(0,0);
	setfat(axisfat);

	/* TITLE */
	setcol(1);  
	uText(0.5*nt, 1.0 + 1.0/titlsz, titlsz, 0, title);

	/* LABELS */
	setcol(8);  
	uText(-.3/scalet , 0.0, lablsz, 3, label1); 
	uText(0.5*nt, -1.0 - .35/scalex, lablsz, 0, label2);

	/* AXIS BOX AND ZERO LINE*/
	setcol(3);  
	umove( 0.0,        -1.0 );
	udraw( 0.0,         1.0 );
	udraw( (float) (nt-1) ,  1.0 );
	udraw( (float) (nt-1) , -1.0 );
	udraw( 0.0,        -1.0 );
	umove( 0.0,         0.0 );
	udraw( (float) (nt-1),   0.0 );

	/* Horiz tics */
	dtict = nt/ntict;
/* 	if (dtict > 10) dtict -= dtict%10;	 shuki */
	for (i = 0 ; i < nt ; i += dtict) {
	    if (dt > 1) {
		sprintf(tval, "%g", (float) tmin + i*dt);
	    } else {
		sprintf(tval, "%.3g", (float) tmin + i*dt);
	    }
	    uText( (float)i , -1.0 - 2.0*marginx, ticsz, 0, tval );
	    umove( (float)i , -1.0 - marginx );
	    udraw( (float)i , -1.0           );
	    umove( (float)i ,  1.0           );
	    udraw( (float)i ,  1.0 + marginx );
	}

	/* Amplitude tics */
	dticx = max/nticx;
	truedticx = truemax/nticx;

	for ( i = -nticx ; i <= nticx ; i++ ) {
		umove( (float) (nt-1) , i*dticx  / max );
		udraw( (1.0 + margint) * nt, i*dticx / max );
		sprintf( tval, "%.3g", i*truedticx );
		utext( (1.0 + 2*margint) * nt, i*dticx / max , ticsz, 0, tval);
	}


	/* DRAW N2 WIGGLE TRACES */
	for ( i = 0; i < nx; i++ ) {
		setfat(plotfat);
	 	setcol(2); 
		wgl1( data + nt*i, nt ); 
	}
}

wgl1(f,n)
float *f;
int n;
{
	int i;

	umove(0.0,f[0]);
	for(i=1;i<n;i++) {
		udraw((float)i,f[i]);
	}
}

/* Wiggle plot subroutine for vertical plotting */
void subplotw(dataptr, nt, ntr, title, label1, label2, 
		tmin, dt, xmin, dx, sizex, sizet, zerox, zerot)
float *dataptr;
int nt, ntr;
string title, label1, label2;
float tmin, dt, xmin, dx, sizex, sizet, zerox, zerot;
{
	float gap;		/* 					*/
	float ltic;		/* length of tic marks (inches)		*/
	float margint;		/* top/bot gap between box and traces	*/
	float marginx;		/* side gap between box and traces 	*/
	float mt;		/* margint/scalet			*/
	float mx;		/* marginx/scalex			*/
	float scalet;		/* time axis scale			*/
	float scalex;		/* trace axis scale			*/
	float tpos;		/* temp for time position		*/
	float xpos;		/* temp for trace position		*/
	int axisfat;		/* line thickness of box & tics		*/
	int dtict;		/* distance between time tics		*/
	int dticx;		/* distance between trace tics		*/
	int fill;		/* fill flag				*/
	int lablsz;		/* label print size			*/
	int ntict;		/* number of tics on time axis		*/
	int nticx;		/* number of tics on trace axis		*/
	int overlap;		/* maximum trace overlap		*/
	int plotfat;		/* line thickness of traces		*/
	int ticsz;		/* tic labeling print size		*/
	int titlsz;		/* title print size			*/
	int tlines;		/* 1=timing lines (0=no timing lines)	*/
	register int i;		/* counter				*/
	string tval[8];		/* local string for tic label		*/


/* 	vertical size of wiggle plot is 60% total height */
	sizet = sizet * .7;

	gap = 0.2;		fgetpar("gap", &gap);

	fill = 1;		igetpar("fill", &fill);

	overlap = 2.0;		igetpar("overlap", &overlap);

	margint = 0.1;		fgetpar("margint", &margint);
	marginx = 0.1;		fgetpar("marginx", &marginx);

	zerot = zerot + .5*sizet + margint + gap;

	scalet = -sizet/nt;
	scalex = sizex/MAX(ntr, 8);

	ltic = 0.05;		fgetpar("ltic", &ltic);

	plotfat = 0;		igetpar("plotfat", &plotfat);
	axisfat = 0;		igetpar("axisfat", &axisfat);

	titlsz = 3;		igetpar("titlsz", &titlsz);
	lablsz = 3;		igetpar("lablsz", &lablsz);
	ticsz = 2;		igetpar("ticsz", &ticsz);

	tlines = 1;		igetpar("tlines", &tlines);

	ntict = 5;	igetpar("ntict", &ntict);
	dtict = nt/ntict;

	nticx = 4;	igetpar("nticx", &nticx);
	dticx = ntr/nticx;

	setscl(scalex, scalet);
	set0(zerox, zerot + sizet);
	setu0(0,0);
	setfat(axisfat);

	mx = marginx/scalex;  mt = margint/scalet; 

	/* TITLE */
	setcol(1);  
	uText(0.5*ntr, mt + 0.45/scalet, titlsz, 0, title);

	/* LABELS */
	setcol(8);  
	uText(-mx-0.3/scalex, 0.5*nt, lablsz, 3, label1); 
	uText(0.5*ntr, nt-mt-0.35/scalet, lablsz, 0, label2);

	/* AXIS BOX */
	setcol(3);  
	umove( -mx                  ,                  mt ); 
	udraw( -mx                  , (float) (nt-1) - mt );
	udraw( (float) (ntr-1) + mx , (float) (nt-1) - mt ); 
	udraw( (float) (ntr-1) + mx ,                  mt );
	udraw( -mx                  ,                  mt );

	/* VERTICAL AXIS TIC MARKS  */
	for (i = 0; i <= nt; i += dtict) {
		umove( -mx, (float) i );
		where( &xpos , &tpos );
		draw( xpos-ltic, tpos );
		if (tlines == 1) {
			umove( -mx , (float) i );
			udraw( (float) (ntr-1) + mx , (float) i); 
		}	
	        umove( mx + (ntr-1) , (float) i );
	        where( &xpos , &tpos);
	        draw( xpos + ltic , tpos );
	        where( &xpos , &tpos );
		if ( dt > 1 ) {
			sprintf(tval, "%g", (float) tmin + i*dt);
		} else {
			sprintf(tval, "%.3g", (float) tmin + i*dt);
		}
	        text(xpos + 0.1, tpos, ticsz, 0, tval);
	}

	/* HORIZ AXIS TIC MARKS  */
	for (i = 0; i <ntr; i++) {
	        umove( (float) i , mt );
	        where( &xpos , &tpos );
	        draw( xpos , tpos + ltic );
	        if (!(i % dticx)) {
		    where(&xpos, &tpos);
	    	    draw(xpos, tpos + ltic);
	        }
	        umove( (float) i , - mt + (nt-1) );
	        where(&xpos, &tpos);
	        draw(xpos, tpos - ltic);
	        if (!(i % dticx)) {
		    where(&xpos, &tpos);
	    	    draw(xpos, tpos - ltic);
		    if ( dx > 1 ) {
		        sprintf(tval, "%g", (float) xmin + i*dx);
		    } else {
		        sprintf(tval, "%.3g", (float) xmin + i*dx);
		    }
		    Text(xpos, tpos - 0.22, ticsz, 0, tval);
	        }
	}

	/* DRAW N2 WIGGLE TRACES */
	setcol(2);  
	setfat(plotfat);
	setscl(scalex*overlap, scalet);
	for (i = 0; i < ntr; i++) {
		setu0(-(float) i / (float) overlap, 0.0);
		vertwigl(dataptr + nt*i, nt, fill, overlap);
	}

}


#define WEPS	0.001	/* Patch to avoid filling zero width polygons */

/* Shuki's code and we don't understand it (jkc, chris) */
vertwigl(data, n, fill, overlap)
float *data;
int n, fill, overlap;
{
	int lp = 0;
	static bool first = true;
	static float *xp, *yp;
	float s;
	register int i;

	if (first) {
		xp = ealloc1float(n + 2);
		yp = ealloc1float(n + 2);
		first = false;
	}
	
	s = (fill) ? WEPS : (float) overlap;
	
	if (s <= data[0] && s < data[1]) {
		yp[0] = 0.0;
		xp[0] = s;
		yp[1] = 0.0;
		xp[1] = data[0];
		lp = 2;
	}
	umove(data[0], 0.0);
	for (i = 1; i < n; i++) {
		udraw(data[i], (float) i);
		if (data[i] > s) {
			if (data[i-1] <= s) {
				yp[0] = (s-data[i])/(data[i]-data[i-1]) + i;
				xp[0] = s;
				lp = 1;
			}
			yp[lp] = i;
			xp[lp] = data[i];
			lp++;
		}
		if (data[i] < s && data[i-1] > s) {
			yp[lp] = (s - data[i]) / (data[i] - data[i-1]) + i;
			xp[lp] = s;
			lp++;
			if (lp > 2) uarea(xp, yp, lp, 0, 1, 1);
			lp = 0;
		} else if (i == n-1 && lp) {
			yp[lp] = i;
			xp[lp] = s;
			lp++;
			if (lp > 2) uarea(xp, yp, lp, 0, 1, 1);
		}
	}
}
