/* SUCQGRAPH: $Revision: 1.5 $ ; $Date: 90/10/19 10:33:30 $		*/

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
SUCQGRAPH - quick amplitude graph of su data traces			\n\
									\n\
sucqgraph <data [optional parameters] | pen				\n\
									\n\
Optional Parameters:							\n\
...MISCELLANEOUS ...							\n\
	max   	= (from data)	max for amplitude axis			\n\
	dash   	= 0		flag for no dashed lines in plot	\n\
				= 1  to cycle thru 6 dash settings	\n\
	nticy   = 1		number of tics on amplitude axis	\n\
	ticsz   = 3		tic labeling print size			\n\
...SIZE & LOCATION ...							\n\
	sizex	= 8.0		length of x axis (inches)		\n\
	sizey	= 5.0		length of y axis (...)			\n\
	zerox	= 0.5		left of plot to left of screen (...)	\n\
	zeroy	= 1.0		base of plot to bottom of screen (...)	\n\
	hcopy	= 0		hardcopy sizing flag 			\n\
				= 1 change defaults for hardcopy	\n\
				sizex=4.5 sizey=3.5 			\n\
";
/**************** end self doc ***********************************/

/* Credits:
 *	CWP: Chris
 *	SEP: Shuki
 *
 * Caveats:
 * 	graph subroutine is very messy because user (x,y)
 *	is internal (t,x) ...	
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
	float scale = 1.0;	/* scale for gaining			*/
	float tmin, dt;		/* for gain sub 	*/
	string title; 
	string label1; 
	string label2;
	int n;
	int ndata;		/* allocation parameter			*/
	int nfloats;		/* number of floats in data set		*/
	int nt;			/* length of input traces		*/
	int ntsize;		/* ... in bytes				*/
	int ntr;		/* traces in input data			*/
	register int i;		/* counter				*/
	void subplot(		/* cplot commands			*/
		float *data,
		int nt,
		int nx,
		string title,
		string label1,
		string label2,
		float truemax,
		float max);


	/* Initialize */
	initargs(argc, argv);
	askdoc(1);

	/* Read first trace */ 
	if (!gettr(&tr)) err("can't get first trace\n");

	/* Get number of time samples & calc some constants */	
	nt = tr.ns; 
	ntsize = nt * FSIZE;


	/* Allocate block of memory for data float mega-vector */
	ndata = MAX(NFALLOC, nt); /* alloc at least one trace */
	data = ealloc1float(ndata);


	/* Loop over input traces & put them into data mega-vector */
	ntr = 0;
	do {
		++ntr;
		if (ntr*nt > ndata) {	/* need more memory */
			ndata += NFALLOC;
			data = erealloc1float(data, ndata);
		}
		bcopy(tr.data, data + (ntr - 1)*nt, ntsize); 
	} while(gettr(&tr));


	/* Find max value in data before gain for labels */
	nfloats = nt*ntr;
	gmax1 =  ABS(data[0]);
	for (i = 1; i < nfloats; ++i)
		gmax1 = MAX(gmax1, ABS(data[i]));

	/* Check if user has given a max */
	if (!fgetpar("max", &givenmax))		givenmax = 0.0;
	if ( givenmax ) {
		scale = gmax1 / givenmax;
		gmax1 = givenmax;
	} 

	/* Normalize data to unity for wgl1 subroutine */
	gain(data, TPOW, EPOW, GPOW, AGC, TRAP, CLIP, QCLIP,
			QBAL, PBAL, scale, tmin, dt, WAGC, nt, ntr);

	/* Find max value in data after gain (should be one) */
	gmax2 =  ABS(data[0]);
	for (i = 1; i < nfloats; ++i)
		gmax2 = MAX(gmax2, ABS(data[i]));

	/* Echo some info to user */
	warn("nt=%d   ntr=%d",nt,ntr);	
	if ( givenmax ) {
		warn("global data max = %g    given plot max = %g",
			scale*givenmax,givenmax);	
	} else {
		warn("global max = %g ",gmax1,gmax2);	
	}

	/* Call the plotting program as a subroutine */
	subplot(data, nt, ntr, title, label1, label2, gmax1, gmax2 );  

	endplot ();
	

	return EXIT_SUCCESS;
}

void subplot(float *data, int nt, int nx, string title,
	string label1, string label2, float truemax, float max)
{
	int i,plotfat,axisfat,dtict,ntict,nticx;
	float sizet,sizex,scalet,scalex,zerot,zerox,margint,marginx,dticx;
	float dash,gap,truedticx,xmin,dx;
	int titlsz,lablsz,ticsz,hcopy,dashflag;
	char tval[8];		/* tic value string			*/

	zerot = 0.5;		fgetpar("zerox",&zerot);
	zerox = 1.0;		fgetpar("zeroy",&zerox);

	sizet = 8.0;		fgetpar("sizex",&sizet);
	sizex = 5.0;		fgetpar("sizey",&sizex);

	dashflag = 0; 		igetpar("dash",&dashflag);

	nticx = 1; 		igetpar("nticy",&nticx);
	ntict = 5;		fgetpar("nticx",&ntict);

	dtict = nt/ntict;
	if (dtict>10) dtict -= dtict%10;
	igetpar("dticx",&dtict);

	margint = 0.01;		fgetpar("marginx",&margint);
	marginx = 0.04;		fgetpar("marginy",&marginx);

	ticsz = 3;		igetpar("ticsz",&ticsz);

	hcopy = 0;		igetpar("hcopy",&hcopy);
	if ( hcopy == 1 ) {
		sizet=4.5; 
		sizex=3.5;
		ticsz=2;
	}

	scalet = sizet/nt;	
	scalex = 0.5*sizex;
	setscl(scalet,scalex);
	set0(zerot, zerox + 0.5*sizex );
	setfat(axisfat);


	/* ZERO LINE*/
	setcol(3);  
	umove( 0.0,         0.0 );
	udraw( (float)nt,   0.0 );

	/* Amplitude tics */

	dticx = max/nticx;
	truedticx = truemax/nticx;

	
	for ( i = -nticx ; i <= nticx ; i++ ) {
		umove( (float)nt , i*dticx  / max );
		udraw( (1.0 + margint) * nt, i*dticx / max ); 
		sprintf( tval, "%.3g", i*truedticx );
		utext( (1.0 + 2*margint) * nt, i*dticx / max , ticsz, 0, tval);
	}


	/* DRAW N2 WIGGLE TRACES */
	for ( i = 0 ; i < nx ; i++ ) {
		if ( dashflag ) {
			dash = ( i % 6 ) * .05;
			gap = dash;
			if ( dash ) {
				setdash(dash,gap,dash,gap);
			}
		}
	 	setcol(6 - i % 6); 
		wgl1( data + nt*i, nt); 
	}
}

wgl1(float *f, int n)
{
	int i,lp=0;
	static first=1;
	static float *xp,*yp;

	if (first) {
		xp = (float*) malloc((uint) 4*(n+2));
		yp = (float*) malloc((uint) 4*(n+2));
		first = 0;
	}
	umove(0.0,f[0]);
	for(i=1;i<n;i++)
	{
		udraw((float)i,f[i]);
	}
}
