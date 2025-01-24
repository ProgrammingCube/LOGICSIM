#ifndef	GEOFF_TRM_H
#define	GEOFF_TRM_H

/* #include "cpm.h" */
/* Change these values to suit your needs */
#define	CRTBASE	18		/* CRT base port */
#define	CRTBUFS	25		/* CRT Buffer Size */

/* Define these macros if you wish to use other forms
   of these functions
*/
/* #define	MEMSET	0 */

/* Macros for GEOFFTERM */
#define	FILL	1
#define	NOFILL	0

/* PRIMITIVES */

typedef struct sGeoff
{
	unsigned char _crtbase;
	unsigned char _crtio;
	unsigned char _buf[CRTBUFS];
	char (*getch)();
	char (*getchar)();
	void (*putchar)();
	void (*puts)();
	void (*cursor)();	/* not sure what to do with this, going to be modes */
	void (*gotoxy)();
	void (*clear)();
	void (*drawLine)();
	void (*drawRect)();
	void (*drawCirc)();
} Geoff;

/* #ifndef	MEMSET
#define	MEMSET */
void* memset();
/* #endif */

char _geofgch();
char _geofgcr();
void _geofpch();
void _geofpts();
void _geofcrs();
void _geofgxy();
void _geofclr();
void _geoflin();
void _geofbox();
void _geofcrc();

void initTerm();

#endif