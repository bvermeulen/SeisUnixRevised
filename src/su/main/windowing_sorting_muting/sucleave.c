/* Copyright (c) Colorado School of Mines, 2022.*/
/* All rights reserved.			*/

/* SUCLEAVE: $Revision: 1.2 $ ; $Date: 2022/10/28 11:00:01 $	*/

#include "su.h"
#include "segy.h"
#include "headcase.h"

/*********************** self documentation **********************/
char *sdoc[] = {
"                                                                       ",
" SUCLEAVE - Cleave traces from 1 input into multiple output files.     ",
"           (split to multiple output files, retaining input order).    ",
"                                                                       ",
" sucleave <stdin key=offset low=50 size=100 abs=1                      ",
"                                                                       ",
" key=offset  name of key used to seperate traces to output files.      ",
" 									",
"    abs=1    Use absolute value of key.                                ",
"       =0    Do not use absolute value of key.                         ",
" 									",
"  size=100   size of key ranges for output files. Must be > 0.         ",
" 									",
"  low=size/2 Base value in lowest magnitude range for output files.    ",
"             The default is size/2. Must be <= size/2 and > 0.         ",
"             For each output file the centre of the range of traces    ",
"             output to it is low+size*N (where N is an integer).       ",
"              ( The lower boundary is included in the N range,         ", 
"               but the higher boundary belongs to the N+1 range ).     ", 
" 									",
" high=100000 base value in highest magnitude range for output files.   ",
"             (This value is adjusted to a multiple of low+size*N).     ",
"             If abs=1 the range limits are from  low  to high.         ",
"             If abs=0 the range limits are from -high to high.         ",
" 									",
"  outbase=   This value is used as part of the output file names.      ",
"             By default outbase is cleave_key_ where key is key= name. ",
"             The nearest integer of the range base value (low+size*N)  ",
"             is always appended after outbase. For example:            ",
"               cleave_offset_50.su   cleave_offset_150.su              ",
" 									",
"  print=0    Do not print.                                             ",
"       =1    Print filename, trace count, average key value for        ",
"             each created output file. Note that ranges with 0 traces  ",
"             will make no print.                                       ",
" 									",
"  ATTENTION: No traces are deleted. All traces less than the minimum   ",
"             key range are written to one extra file, and all traces   ",
"             greater than the maximum key range are written to another ",
"             extra file.     WARNINGS are printed if this occurs.      ",
"             These extra files can be recognized because the number in ",
"             their names is outside the low,high ranges you specified. ",
" 									",
"  NOTES                                                                ",
"   1. See simple tests (examples) in src/demos/Utilities/Sucleave      ",
"   2. Output files are not created until a trace is written to them.   ",
"      So, specifying excessive low= and high= values has little        ",
"      consequences except for some small memory allocation herein.     ",
"   3. Bad choice of size can create an output file for every trace.    ",
" 									",
" EXAMPLE USES:                                                         ",
"   1. You may want to deliberately partition your traces for different ",
"      processing. You can cleave your traces into offset ranges, while ",
"      retaining cdp order in those offset-ranged files.                ",
"      You can then use subraid to bring them back together.            ",
"   2. In streamer-cable-marine you may want to partition your shot     ",
"      gathers by cables (or port-starboard shot, cable combinations).  ",
"      This is sometimes called nominal subsurface line processing.     ",
"      You can then use subraid to bring them back together.            ",
"                                                                       ",
NULL};

/* Author:
 *	Andre Latour.        
 *	Looked at suop2.c code to get started (efopen,fgettr). 
 */
/**************** end self doc ***********************************/

segy tr;

int main(int argc, char **argv) {
  cwp_String key=NULL;      /* for key= name                                 */
  int kase;                 /* for case code of key       (from GetCase)     */
  FILE **fps;               /* file pointers	                             */
  int  *icount;             /* flag and counter for traces in this range     */
  double *dsum;             /* sum of the key values in this range           */
  int ifile=0;              /* a file number                                 */
  int nfile=0;              /* maximum number of files                       */
  int minf= 2147483645;     /* minimum file number written to.               */
  int maxf=-2147483645;     /* maximum file number written to.               */
  cwp_String fname=NULL;    /* a file name                                   */
  double dsize;             /* defines size of ranges.                       */
  double dlow;              /* defines middle of lowest range.               */
  double dhigh;             /* defines middle of highest range.              */
  double dval;              /* value of key from input trace                 */
  cwp_String outbase=NULL;  /* part of output file names                     */
  int iabs=1;               /* use absolute of key values?                   */
  int iprint=0;             /* print option.                                 */
  int n=0;                  /* range number                                  */
  int i=0;                  /* general                                       */

/* hook up getpar */

  initargs(argc, argv);
  requestdoc(1);

/* Get key case numbers from GetCase.                                        */

  if (!getparstring("key", &key)) key = "offset";
  kase = GetCase(key);
  if(kase<1) err("error: key= is not a recognized header key name");

  if (!getparint("abs", &iabs)) iabs = 1;
  if(iabs>1 || iabs<0) err("error: abs= is out of range.");

  if (!getpardouble("size", &dsize)) dsize = 100;
  if(dsize <= 0.) err("error: size= must be greater than zero.");

  if (!getpardouble("low", &dlow)) dlow = dsize / 2.0;
  if(dlow<0.)        err("error: low= must be greater than zero.");
  if(dlow>dsize/2.0) err("error: low= must be less than or equal to size/2.");

  if (!getpardouble("high", &dhigh)) dhigh = 100000;
  if(dlow > dhigh) err("error: high= must be greater than low=.");

  if (!getparstring("outbase", &outbase)) {
    outbase = ealloc1(8+strlen(key),1);        
    strcpy(outbase,"cleave_");
    strcpy(outbase+7,key);
    strcpy(outbase+7+strlen(key),"_");
  }

  fname = ealloc1(strlen(outbase)+20,1); 

  if (!getparint("print", &iprint)) iprint = 0;
  if(iprint>1 || iprint<0) err("error: print= is out of range.");

  checkpars();

/* Remember that low is constrained to be >0 and <= size/2.                  */
/* When not absolute-valuing the key, the range is from low to high.         */
/* When absolute-valuing the key, the range is from -high to high.           */
/* But in both cases, middle of ranges hits the low value exactly with       */
/* both high and -high also effectively being at multiples of low+size*N.    */
/*                                                                           */
/* Further complications: we also want an extra output file to get all the   */
/* traces greater than high. And another extra output file to get all the    */
/* traces less than -high. That and other fiddly stuff is why 3.50000000001  */
/* for round-offs and negatives and so on.  The 0000000001 part is there to  */
/* attempt to get the exact same results on all compilers and hardware.      */

  nfile = (int) (3.50000000001 + (dhigh-dlow) / dsize);  
  if(iabs==1) {
    dlow  -= dsize;
  }
  else {
    nfile -= 1;
    dlow  -= dsize*nfile;
    nfile  = nfile*2;
  }

/* Allocate, initialize. */

  fps = calloc(nfile,sizeof(FILE *)); 
  icount = ealloc1int(nfile);        
  dsum   = ealloc1double(nfile);        
  for (i=0; i<nfile; i++) {
    icount[i] = 0;
    dsum[i] = 0.0;
  }

  if(iprint==1) warn(" Output file         trace count      key average ");

/* Loop over the input traces.                                               */

  while(gettr(&tr)) {
 
    dval = fromhead(tr,kase);
    if(dval<0. && iabs>0) dval = 0.0 - dval; 

    ifile = (int) (0.50000000001 + (dval-dlow)/dsize) ;

    if(ifile>nfile-1) ifile = nfile - 1;
    else if(ifile<1)  ifile = 0;

    if(icount[ifile] < 1) {
      n = lrint(ifile*dsize+dlow+0.00000000001); 
      sprintf(fname,"%s%d.su",outbase,n); 
      fps[ifile] = efopen(fname,"w");
      if(ifile<minf) minf = ifile;
      if(ifile>maxf) maxf = ifile;
    }

    fputtr(fps[ifile],&tr);
    icount[ifile] += 1;     
    dsum[ifile] += dval;

  } /* end of  while(gettr(&tr)) {  */

  for (i=minf; i<=maxf; i++) {
    if(icount[i] > 0) {
      efclose(fps[i]);
      if(iprint==1) {
        dsum[i] /= icount[i];
        n = lrint(i*dsize+dlow+0.00000000001);
        sprintf(fname,"%s%d.su",outbase,n); 
        warn("%s       %d       %f",fname,icount[i],dsum[i]);
      }
    }
  }

  if(minf==0) { 
    n = lrint(dlow);
    sprintf(fname,"%s%d.su",outbase,n); 
    warn("%d traces have key < low range. Output in extra file %s",icount[0],fname);
  }
  if(maxf==nfile-1) { 
    n = lrint(maxf*dsize+dlow);
    sprintf(fname,"%s%d.su",outbase,n); 
    warn("%d traces have key > high range. Output in extra file %s",icount[maxf],fname);
  }

  return(CWP_Exit());
}

