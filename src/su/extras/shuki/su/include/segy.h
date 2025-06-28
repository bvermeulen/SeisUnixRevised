/* segy.h - SEGY structures */ 

#ifndef INCLUDE_SEGY_H
#define INCLUDE_SEGY_H

#define	SY_EBCBYTES	3200  /* Bytes in the card image EBCDIC block */
#define	SY_BNYBYTES	400   /* Bytes in the binary coded block	*/
#define SY_NDAT		16384 /* Arbitrary limit on data array size	*/

typedef struct segy_tr {

	long tracl;	/* trace sequence number within line */

	long tracr;	/* trace sequence number within reel */

	long fldr;	/* field record number */

	long tracf;	/* trace number within field record */

	long ep;	/* energy source point number */

	long cdp;	/* CDP ensemble number */

	long cdpt;	/* trace number within CDP ensemble */

	short trid;	/* trace identification code:
				1 = seismic data
				2 = dead
				3 = dummy
				4 = time break
				5 = uphole
				6 = sweep
				7 = timing
				8 = water break
				SEGY: 9---, N = optional use (N = 32,767)
				From here on these are CWP id flags:
				9 = autocorrelation
				10 = Fourier transformed - no packing
				11 = Fourier transformed - unpacked Nyquist
				12 = Fourier transformed - packed Nyquist
				13 = Complex signal in the time domain
				101 = 1 byte packing (chars)
				102 = 2 byte packing (short ints)
			*/
	short nvs;	/* number of vertically summed traces (see vscode
			   in Bhed structure) */

	short nhs;	/* number of horizontally summed traces (see vscode
			   in Bhed structure) */
	short duse;	/* data use:
				1 = production
				2 = test */

	long offset;	/* distance from source point to receiver
			   group (negative if opposite to direction
			   in which the line was shot) */

	long gelev;	/* receiver group elevation from sea level
			   (above sea level is positive) */

	long selev;	/* source elevation from sea level
			   (above sea level is positive) */

	long sdepth;	/* source depth (positive) */

	long gdel;	/* datum elevation at receiver group */

	long sdel;	/* datum elevation at source */

	long swdep;	/* water depth at source */

	long gwdep;	/* water depth at receiver group */

	short scalel;	/* scale factor for previous 7 entries
			   with value plus or minus 10 to the
			   power 0, 1, 2, 3, or 4 (if positive,
			   multiply, if negative divide) */
	short scalco;	/* scale factor for next 4 entries
			   with value plus or minus 10 to the
			   power 0, 1, 2, 3, or 4 (if positive,
			   multiply, if negative divide) */

	long  sx;	/* X source coordinate */

	long  sy;	/* Y source coordinate */

	long  gx;	/* X group coordinate */

	long  gy;	/* Y source coordinate */

	short counit;	/* coordinate units code:
				for previoius four entries
				1 = length (meters or feet)
				2 = seconds of arc (in this case, the
				X values are longitude and the Y values
				are latitude, a positive value designates
				the number of seconds east of Greenwich
				or north of the equator */
	short wevel;	/* weathering velocity */

	short sweves;	/* subweathering velocity */
	short sut;	/* uphole time at source */

	short gut;	/* uphole time at receiver group */
	short sstat;	/* source static correction */

	short gstat;	/* group static correction */
	short tstat;	/* total static applied */

	short laga;	/* lag time A, time in ms between end of 240-
			   byte trace identification header and time
			   break, positive if time break occurs after
			   end of header, time break is defined as
			   the initiation pulse which maybe recorded
			   on an auxiliary trace or as otherwise
			   specified by the recording system */
	short lagb;	/* lag time B, time in ms between the time break
			   and the initiation time of the energy source,
			   may be positive or negative */

	short delrt;	/* delay recording time, time in ms between
			   initiation time of energy source and time
			   when recording of data samples begins
			   (for deep water work if recording does not
			   start at sero time) */
	short muts;	/* mute time--start */

	short mute;	/* mute time--end */
	unsigned short ns;	/* number of samples in this trace */

	unsigned short dt;	/* sample interval; in micro-seconds */
	short gain;	/* gain type of field instruments code:
				1 = fixed
				2 = binary
				3 = floating point
				4 ---- N = optional use */

	short igc;	/* instrument gain constant */
	short igi;	/* instrument early or initial gain */

	short corr;	/* correlated:
				1 = no
				2 = yes */
	short sfs;	/* sweep frequency at start */

	short sfe;	/* sweep frequency at end */
	short slen;	/* sweep length in ms */

	short styp;	/* sweep type code:
				1 = linear
				2 = cos-squared
				3 = other */
	short stas;	/* sweep trace length at start in ms */

	short stae;	/* sweep trace length at end in ms */
	short tatyp;	/* taper type: 1=linear, 2=cos^2, 3=other */

	short afilf;	/* alias filter frequency if used */
	short afils;	/* alias filter slope */

	short nofilf;	/* notch filter frequency if used */
	short nofils;	/* notch filter slope */

	short lcf;	/* low cut frequency if used */
	short hcf;	/* high cut frequncy if used */

	short lcs;	/* low cut slope */
	short hcs;	/* high cut slope */

	short year;	/* year data recorded */
	short day;	/* day of year */

	short hour;	/* hour of day (24 hour clock) */
	short minute;	/* minute of hour */

	short sec;	/* second of minute */
	short timbas;	/* time basis code:
				1 = local
				2 = GMT
				3 = other */

	short trwf;	/* trace weighting factor, defined as 1/2^N
			   volts for the least sigificant bit */
	short grnors;	/* geophone group number of roll switch
			   position one */

	short grnofr;	/* geophone group number of trace one within
			   original field record */
	short grnlof;	/* geophone group number of last trace within
			   original field record */

	short gaps;	/* gap size (total number of groups dropped) */
	short otrav;	/* overtravel taper code:
				1 = down (or behind)
				2 = up (or ahead) */

	short unass[30];

	float  data[SY_NDAT];

} Segy_tr;


typedef struct segy_bh {	/* Bhed - binary header */

	long jobid;	/* job identification number */

	long lino;	/* line number (only one line per reel) */

	long reno;	/* reel number */

	short ntrpr;	/* number of data traces per record */
    short nart;	/* number of auxiliary traces per record */

	short hdt;	/* sample interval in micro secs for this reel */
	short dto;	/* same for original field recording */

	short hns;	/* number of samples per trace for this reel */
	short nso;	/* same for original field recording */

	short format;	/* data sample format code:
				1 = floating point (4 bytes)
				2 = fixed point (4 bytes)
				3 = fixed point (2 bytes)
				4 = fixed point w/gain code (4 bytes) */
	short fold;	/* CDP fold expected per CDP ensemble */

	short tsort;	/* trace sorting code: 
				1 = as recorded (no sorting)
				2 = CDP ensemble
				3 = single fold continuous profile
				4 = horizontally stacked */
	short vscode;	/* vertical sum code:
				1 = no sum
				2 = two sum ...
				N = N sum (N = 32,767) */

	short hsfs;	/* sweep frequency at start */
	short hsfe;	/* sweep frequency at end */

	short hslen;	/* sweep length (ms) */
	short hstyp;	/* sweep type code:
				1 = linear
				2 = parabolic
				3 = exponential
				4 = other */

	short schn;	/* trace number of sweep channel */
	short hstas;	/* sweep trace taper length at start if
			   tapered (the taper starts at zero time
			   and is effective for this length) */

	short hstae;	/* sweep trace taper length at end (the ending
			   taper starts at sweep length minus the taper
			   length at end) */
	short htatyp;	/* sweep trace taper type code:
				1 = linear
				2 = cos-squared
				3 = other */

	short hcorr;	/* correlated data traces code:
				1 = no
				2 = yes */
	short bgrcv;	/* binary gain recovered code:
				1 = yes
				2 = no */

	short rcvm;	/* amplitude recovery method code:
				1 = none
				2 = spherical divergence
				3 = AGC
				4 = other */
	short mfeet;	/* measurement system code:
				1 = meters
				2 = feet */

	short polyt;	/* impulse signal polarity code:
				1 = increase in pressure or upward
				    geophone case movement gives
				    negative number on tape
				2 = increase in pressure or upward
				    geophone case movement gives
				    positive number on tape */
	short vpol;	/* vibratory polarity code:
				code	seismic signal lags pilot by
				1	337.5 to  22.5 degrees
				2	 22.5 to  67.5 degrees
				3	 67.5 to 112.5 degrees
				4	112.5 to 157.5 degrees
				5	157.5 to 202.5 degrees
				6	202.5 to 247.5 degrees
				7	247.5 to 292.5 degrees
				8	293.5 to 337.5 degrees */

	short hunass[170];	/* unassigned */

} Segy_bh;
#endif INCLUDE_SEGY_H
