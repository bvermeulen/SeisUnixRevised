#ifndef	_SFSTDIO_H
#define	_SFSTDIO_H	1

#include "no_stdio.h"

extern int	_Stdstream;

extern int	_Stdextern;

#define _uflow	__uflow
#define FILBUF(f)	int _uflow(FILE* f)
#define _do_uflow      1

#define _overflow	__overflow
#define FLSBUF(c,f)	int _overflow(FILE* f, int c)
#define _do_overflow       1

#define _NFILE	16

/* preventing FILE to be defined in silly places */
#define	_FILE_DEFINED	1	/* Windows */
#define	_FILEDEFED	1	/* SUNOS5.8 */

#undef	FILE
typedef struct _std_s	FILE;

/* Linux7.2 requires __FILE in wchar.h - we fake it here */
#include	"FEATURE/sfio"
#if _typ___FILE
#if CWP_RED_HAT_7_3
#endif

#endif

#include	"sfhdr.h"
#include	"FEATURE/stdio"

#define stdfpos_t	long

#define stdoff_t	long


struct _std_s
{
	int	std_flag;
	uchar	std_gap0[4];
	uchar*	std_readptr;
	uchar*	std_readend;
	uchar	std_gap1[16];
	uchar*	std_writeptr;
	uchar*	std_writeend;
	uchar	std_gap2[56];
	int	std_file;
	uchar	std_gap3[100];
};

#define BUFSIZ		8192
#define _IOERR		040
#define _IOEOF		020
#define _IONBF		02
#define _IOLBF		01
#define _IOFBF		0

#define _seteof(fp)	((fp)->std_flag |= _IOEOF)
#define _seterr(fp)	((fp)->std_flag |= _IOERR)

#define _stdclrerr(fp)	((fp)->std_flag &= ~(_IOEOF|_IOERR))
#define _stdseterr(fp,sp)	((sfeof(sp) ? _seteof(fp) : 0), (sferror(sp) ? _seterr(fp) : 0) )


_BEGIN_EXTERNS_

#if _BLD_sfio && defined(__EXPORT__)
#define extern	__EXPORT__
#endif
#define _do_self_stdin	1

extern FILE		*stdin, *stdout, *stderr;

extern void		_sfunmap _ARG_((FILE*));
extern Sfio_t*		_sfstream _ARG_((FILE*));
extern FILE*		_stdstream _ARG_((Sfio_t*, FILE*));
extern char*		_stdgets _ARG_((Sfio_t*,char*,int,int));
extern void		clearerr _ARG_((FILE*));
extern int		_doprnt _ARG_((const char*, va_list, FILE*));
extern int		_doscan _ARG_((FILE*, const char*, va_list));
extern int		fclose _ARG_((FILE*));
extern FILE*		fdopen _ARG_((int, const char*));
extern int		feof _ARG_((FILE*));
extern int		ferror _ARG_((FILE*));
extern int		fflush _ARG_((FILE*));
extern int		fgetc _ARG_((FILE*));
extern char*		fgets _ARG_((char*, int, FILE*));
extern int		_filbuf _ARG_((FILE*));
extern int		_uflow _ARG_((FILE*));
extern int		_srget _ARG_((FILE*));
extern int		_sgetc _ARG_((FILE*));
extern int		fileno _ARG_((FILE*));
extern int		_flsbuf _ARG_((int, FILE*));
extern int		_overflow _ARG_((FILE*,int));
extern int		_swbuf _ARG_((int,FILE*));
extern int		_sputc _ARG_((int,FILE*));
extern void		_cleanup _ARG_((void));
extern FILE*		fopen _ARG_((char*, const char*));
extern int		fprintf _ARG_((FILE*, const char* , ...));
extern int		fputc _ARG_((int, FILE*));
extern int		fputs _ARG_((const char*, FILE*));
extern size_t		fread _ARG_((void*, size_t, size_t, FILE*));
extern FILE*		freopen _ARG_((char*, const char*, FILE*));
extern int		fscanf _ARG_((FILE*, const char* , ...));
extern int		fseek _ARG_((FILE*, long , int));
extern long		ftell _ARG_((FILE*));
extern int		fgetpos _ARG_((FILE*,stdfpos_t*));
extern int		fsetpos _ARG_((FILE*,stdfpos_t*));
extern int		fpurge _ARG_((FILE*));
extern size_t		fwrite _ARG_((const void*, size_t, size_t, FILE*));
extern int		getc _ARG_((FILE*));
extern int		getchar _ARG_((void));
extern char*		gets _ARG_((char*));
extern int		getw _ARG_((FILE*));
extern int		pclose _ARG_((FILE*));
extern FILE*		popen _ARG_((const char*, const char*));
extern int		printf _ARG_((const char* , ...));
extern int		putc _ARG_((int, FILE*));
extern int		putchar _ARG_((int));
extern int		puts _ARG_((const char*));
extern int		putw _ARG_((int, FILE*));
extern void		rewind _ARG_((FILE*));
extern int		scanf _ARG_((const char* , ...));
extern void		setbuf _ARG_((FILE*, char*));
extern int		setbuffer _ARG_((FILE*, char*, size_t));
extern int		setlinebuf _ARG_((FILE*));
extern int		setvbuf _ARG_((FILE*,char*,int,size_t));
extern int		sprintf _ARG_((char*, const char* , ...));
extern int		sscanf _ARG_((const char*, const char* , ...));
extern FILE*		tmpfile _ARG_((void));
extern int		ungetc _ARG_((int,FILE*));
extern int		vfprintf _ARG_((FILE*, const char* , va_list));
extern int		vfscanf _ARG_((FILE*, const char* , va_list));
extern int		vprintf _ARG_((const char* , va_list));
extern int		vscanf _ARG_((const char* , va_list));
extern int		vsprintf _ARG_((char*, const char* , va_list));
extern int		vsscanf _ARG_((char*, const char* , va_list));
extern void		flockfile _ARG_((FILE*));
extern void		funlockfile _ARG_((FILE*));
extern int		ftrylockfile _ARG_((FILE*));
extern int		snprintf _ARG_((char*, size_t, const char*, ...));
extern int		vsnprintf _ARG_((char*, size_t, const char*, va_list));
extern int		__snprintf _ARG_((char*, size_t, const char*, ...));
extern int		__vsnprintf _ARG_((char*, size_t, const char*, va_list));

_END_EXTERNS_

#endif /* _SFSTDIO_H */
