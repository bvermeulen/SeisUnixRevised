/* Copyright (c) Bruno Vermeulen, 2024*/
/* All rights reserved.               */


#include "su.h"
#include "segy.h"

/*********************** self documentation **********************/
char *sdoc[] = {
" 								                            	",
" SUBVSOG - Compute and set the offset header					",
" 									                            ",
" Required parameters: None                                     ",
" geometry for sx,sy, gx,gy need to be defined                  ",
"                           									",
" Optional parameters               							",
" verbose=0		<>0 output informations				            ",
"                            									",
"                           									",
NULL};

/**************** end self doc ***********************************/

segy tr;

int
main(int argc, char **argv)
{
	int verbose;
	float invScalco, invScalel;
	float sx, sy, selev, gx, gy, gelev, offset;
    int counter;

	/* Initialize */
	initargs(argc, argv);
	requestdoc(1);

	/* Get verbose parameter */
	if (!getparint("verbose", &verbose)) {
        verbose = 0;
    }
    checkpars();

	/* Get info from first trace */
	if (!gettr(&tr)) {
		err("can't read first trace");
	}

	if (tr.scalco < 0)
	{
		invScalco = -1.0 / tr.scalco;
	}
	else if (tr.scalco > 0)
	{
		invScalco = 1.0;
	}
	else
	{
		warn("scalco = 0 ; 1 assumed");
		invScalco = 1.0;
	}

	if (tr.scalel < 0)
	{
		invScalel = -1.0 / tr.scalel;
	}
	else if (tr.scalel > 0)
	{
		invScalel = 1.0;
	}
	else
	{
		warn("scalel = 0 ; 1 assumed");
		invScalel = 1.0;
	}
	if (verbose)
	{
		warn("scalco: %d (inverse scalar: %.2f), scalel: %d (inverse scalar %.2f)",
			tr.scalco, invScalco, tr.scalel, invScalel);
	}

	/* Loop over traces */
    counter = 0;
	do {
		/* normalize scaling */
		sx = invScalco * tr.sx;
		sy = invScalco * tr.sy;
		selev = invScalel * tr.selev;
		gx = invScalco * tr.gx;
		gy = invScalco * tr.gy;
		gelev = invScalel * tr.gelev;

		offset=sqrt(pow(sx - gx, 2)+pow(sy - gy, 2));
		if ((sx != gx) & (sx - gx < 0)) {
			offset = - offset;
		}

		tr.sx = sx;
		tr.sy = sy;
		tr.selev = selev;
		tr.gx = gx;
		tr.gy = gy;
		tr.gelev = gelev;
		tr.offset = offset;
		tr.scalco = 1.0;
		tr.scalel = 1.0;
		puttr(&tr);
        counter++;

	} while (gettr(&tr));

    if (verbose) {
        warn ("%d traces have been normalized and offset calculatated ...", counter);
    }

	return(CWP_Exit());
}
