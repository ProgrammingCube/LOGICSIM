#include "geofftrm.h"

/* DEFINITIONS */
/* #ifndef	MEMSET
#define	MEMSET*/
void *memset( d, b, s )
void* d;
unsigned char b;
unsigned int s;
{
	unsigned char* p = d;
	while ( s-- )
		*p++ = b;
	return d;
}
/* #endif */

char _geofgch( term )
Geoff* term;
{
	while ( ! ( in( term->_crtbase ) & 1 ) );
	return in( term->_crtio );
}

char _geofgcr( term )
Geoff* term;
{
	char c, first_c = 0;
	while ( 1 )
	{
		/*
		while (!(in(term->_crtbase) & 1));
		term->putchar(term, c = in(term->_crtio) );
		*/
		term->putchar( term, c = ( term->getch( term ) ) );

		if ( !first_c ) first_c = c;

		if ( c == '\r' || c == '\n' ) return first_c;
	}
}

void _geofpch( term, c )
Geoff* term;
char c;
{
	out( term->_crtio, c );
}

void _geofpts( term, s )
Geoff* term;
char* s;
{
	char* p = s;
	while ( *p )
		term->putchar( term, *p++ );
}

void _geofcrs( term, m )
Geoff* term;
char m;
{
	
}
void _geofgxy( term, x, y )
Geoff* term;
int x, y;
{
	memset( term->_buf, 0, CRTBUFS );
	sprintf( term->_buf, "\033[%d;%dH", y, x );
	term->puts( term, term->_buf );
}

void _geofclr( term )
Geoff* term;
{
	term->puts( term, "\033[2J\033[H" );
}

void _geoflin( term, x, y, u, v )
Geoff* term;
int x, y, u, v;
{
	memset( term->_buf, 0, CRTBUFS );
	sprintf( term->_buf, "\033[Z1;%d;%d;%d;%dZ", x, y, u, v );
	term->puts( term, term->_buf );
}

void _geofbox( term, x, y, w, h, m )
Geoff* term;
int x, y, w, h;
char m;
{
	if ( !m )
		m = 2;
	else
		m = 3;
	memset( term->_buf, 0, CRTBUFS );
	sprintf( term->_buf, "\033[Z%d;%d;%d;%d;%dZ", m, x, y, x + w, y + h );
	term->puts( term, term->_buf );
}

void _geofcrc( term, x, y, r, m )
Geoff* term;
int x, y, r;
char m;
{
	if ( !m )
		m = 4;
	else
		m = 5;
	memset( term->_buf, 0, CRTBUFS );
	sprintf( term->_buf, "\033[Z%d;%d;%d;%dZ", m, x, y, r );
	term->puts( term, term->_buf );
}

void initTerm( term, crtbase )
Geoff* term;
unsigned char crtbase;
{
	term->_crtbase = crtbase;
	term->_crtio   = crtbase + 1;
	term->getch    = &_geofgch;
	term->putchar  = &_geofpch;
	term->getchar  = &_geofgcr;
	term->puts     = &_geofpts;
	term->cursor   = &_geofcrs;
	term->gotoxy   = &_geofgxy;
	term->clear    = &_geofclr;
	term->drawLine = &_geoflin;
	term->drawRect = &_geofbox;
	term->drawCirc = &_geofcrc;
}