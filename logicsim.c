/* #include "cpm.h" */
#include "stdio.h"
#include "geofftrm.h"
/* If not using the include sources,
please link against my cpm and geofftrm libraries 
*/
/* maximum number of Cnxtions */
#define	MAX_CNX	256
/* maximum number of Devices */
#define	MAX_DEV	256
/* size of scanf buffer */
#define	SCANBUF	70
/* IO port base */
#define TTYBASE	16

/* Aztec C does not include NULL but does do EOF/EOL */
#define	NULL	0

/* mode for state machine, Device mode or Cnxtion mode */
#define	DEV_MODE	1
#define	CNX_MODE	2

/* disconnected Cnxtion */
#define	DISCNNCT	-1

/* K&R C did not have enums. What a biiiitch... */
#define	WIRE		0
#define	XNR_GATE	1
#define	XOR_GATE	2
#define	NOT_GATE	3
#define	NOR_GATE	4
#define	OR_GATE		5
#define	NND_GATE	6
#define	AND_GATE	7
#define	SIGNAL		8

/* Modes */
#define	VID_MODE	0
#define	DEV_MODE	1
#define	CNX_MODE	2

unsigned char num_dev;		/* number of devices */
unsigned char num_cnx;		/* number of connections */
char sel_dev;		/* selected device */
char sel_cnx;		/* selected connection */

char PRG_MODE = VID_MODE;

/* shape contains a 1D array describing the shape
   First element is number of continuous lines
   Next three lines are the input and output lines ( 2 input and 1 output )
   Successive lines make up the continuous lines to draw
   First element of these lines show the number of points on that line
*/

static char sig_dat[] = { 1,			/* length */
			2, 0, 0, 20, 0		/* line 1 */
			};

static char wire_dat[] = { 1,			/* length */
			2, 0, 0, 20, 0		/* line 1 */
			};
	
static char and_dat[] = { 4,			/* length */
			2, 0, 5, 5, 5,		/* input 1 */
			2, 0, 30, 5, 30,	/* input 2 */
			2, 40, 17, 45, 17,	/* output */
			7, 5, 0, 30, 0, 40, 10, 40, 25, 30, 35, 5, 35, 5, 0	/* outline */
			};
static char nand_dat[] = { 5,			/* length */
			2, 0, 5, 5, 5,		/* input 1 */
			2, 0, 30, 5, 30,	/* input 2 */
			2, 46, 17, 51, 17,	/* output */
			7, 5, 0, 30, 0, 40, 10, 40, 25, 30, 35, 5, 35, 5, 0,	/* outline */
			5, 40, 17, 43, 15, 46, 17, 43, 19, 40, 17		/* negate circle */
			};

static char nor_dat[] = { 5,			/* length */
			2, 0, 5, 9, 5,		/* input 1 */
			2, 0, 30, 9, 30,	/* input 2 */
			2, 46, 17, 51, 17,	/* output */
			9, 5, 0, 30, 0, 40, 10, 40, 25, 30, 35, 5, 35, 15, 25, 15, 10, 5, 0,	/* outline */
			5, 40, 17, 43, 15, 46, 17, 43, 19, 40, 17				/* negate circle */
			};

static char or_dat[] = { 4,			/* length */
			2, 0, 5, 9, 5,		/* input 1 */
			2, 0, 30, 9, 30,	/* input 2 */
			2, 40, 17, 45, 17,	/* output */
			9, 5, 0, 30, 0, 40, 10, 40, 25, 30, 35, 5, 35, 15, 25, 15, 10, 5, 0	/* shape outline */
			};

static char xor_dat[] = { 5,			/* length */
			2, 0, 5, 9, 5,		/* input 1 */
			2, 0, 30, 9, 30,	/* input 2 */
			2, 40, 17, 45, 17,	/* output */
			9, 8, 0, 30, 0, 40, 10, 40, 25, 30, 35, 8, 35, 18, 25, 18, 10, 8, 0,	/* shape outline */
			4, 5, 35, 15, 25, 15, 10, 5, 0						/* xor bow */
			};

static char not_dat[] = { 5,			/* length */
			2, 0, 17, 9, 17,	/* input 1 */
			2, 9, 0, 9, 0,		/* hidden input 2 */
			2, 46, 17, 51, 17,	/* output */
			4, 9, 0, 40, 17, 9, 35, 9, 0,			/* shape outline */
			5, 40, 17, 43, 15, 46, 17, 43, 19, 40, 17	/* negate circle */
			};

static char xnr_dat[] = { 6,			/* length */
			2, 0, 5, 9, 5,		/* input 1 */
			2, 0, 30, 9, 30,	/* input 2 */
			2, 40, 17, 45, 17,	/* output */
			9, 8, 0, 30, 0, 40, 10, 40, 25, 30, 35, 8, 35, 18, 25, 18, 10, 8, 0,	/* shape outline */
			4, 5, 35, 15, 25, 15, 10, 5, 0,						/* xor bow */
			5, 40, 17, 43, 15, 46, 17, 43, 19, 40, 17				/* negate circle */
			};

/* shape pointer table
use this later for setting shape based on type
*/
char* shapeTbl[] = {
			wire_dat,		/* wire_dat */
			xnr_dat,
			xor_dat,
			not_dat,
			nor_dat,
			or_dat,
			nand_dat,
			and_dat,
			sig_dat,
			NULL		/* null terminator */
			};

/* device lookup table */
char devtyplu[]  = {
			WIRE,
			XNR_GATE,
			XOR_GATE,
			NOT_GATE,
			NOR_GATE,
			OR_GATE,
			NND_GATE,
			AND_GATE,
			SIGNAL,
			NULL
			};

/* Device struct
A Device in this sense is any elemnt you draw on the screen that is not
text, the cursor, or airwires
*/
typedef struct sDevice
{
	char type;			/* type of Device */
	int x, y;			/* coordinates to draw */
	char* shape;			/* pointer to which Device to draw */
	char value;			/* Voltage value of Device */
	/*float scale;*/			/* Scale to draw locally */
	unsigned char in_dev[2];	/* Two intputs. Designate which devices are wired up */
	unsigned char output;		/* Designates which output device is hooked up */
} Device;

/* Cnxtion struct
A Cnxtion is a connection between two Devices
*/
typedef struct sCnxtion
{
	unsigned char src_dev;		/* source Device index */
	unsigned char trg_dev;		/* target Device index */
	unsigned char input;		/* input Device index */
} Cnxtion;

/* isdigit
determines if c is an ascii digit
*/
char isdigit(c)
char c;
{
  return (c >= '0' && c <= '9');
}

/* scanf
hand-rolled scanf allows programmer to designate which Terminal object to use
parameter 'f' designates the format of the input to read
parameter 'a' designates the pointer into which to store the input
*/
void _scanf( term, f, a )
Geoff* term;
char f;
void* a;
{
	char buf[SCANBUF+1];
	char c;
	char i = 0;

	memset( buf, 0, sizeof(buf));

	while (i < SCANBUF)
	{
		c = term->getch( term );
		/* handle ^M or ENTER */
		if (c == '\r' || c == '\n')
		{
			i++;
			break;
		}
		/* handle BACKSPACE */
		else if (c == 0x08 && i > 0)
		{
			term->tputs( term, "\010 \010" );
			i--; 
			continue;
		}
		/* print char and increment buffer */
		term->tputchar( term, buf[i++] = c );
	}
	/* eol */
	buf[i] = 0;

	/* format check */
	if ( f == 'd' )
		*(int*)a = atoi(buf);
	else if ( f == 's' )
	{
		char j;
		for ( j = 0; buf[ j ] != '\0'; ++j )
		{
			*((char*)a) = buf[ j ];
			((char*)a)++;
			
		}
		*(char*)a = '\0';
	}
	/*
	else if ( f == 'f' )
		*(double*)a = atof(buf);
	*/
}

/* clear bottom of screen */
void clrLwrSn( term )
Geoff* term;
{
	term->gotoxy( term, 0, 21 );
	term->tputs( term, "\033[0J" );
}

/* Draw cursor
cursor will be 10 pixels tall and wide
*/
void drawCurs( term, x, y )
Geoff* term;
int x, y;
{
	term->drawLine( term, x, y - 5, x, y + 5 );
	term->drawLine( term, x - 5, y, x + 5, y );
}

/* Currently only draws in line segments
   How to
*/
void drawDv( term, device)
Geoff* term;
Device* device;
{
	unsigned char i, j;
	int x1, y1, x2, y2;

	unsigned char vcounter;						/* vertex counter */
	unsigned char ecounter = 1;					/* element counter */
	unsigned char num_lins = device->shape[ 0 ];

	/* for each line block */
	for ( i = 0; i < num_lins; ++i )
	{
		/* grab loop to count */
		vcounter = device->shape[ ecounter ];
		ecounter++;
		/* for loop */
		for ( j = 0; j < vcounter - 1; j++ )
		{
			x1 = device->x + device->shape[ ecounter ];
			y1 = device->y + device->shape[ ecounter + 1 ];
			x2 = device->x + device->shape[ ecounter + 2 ];
			y2 = device->y + device->shape[ ecounter + 3 ];
			ecounter += 2;
			term->drawLine( term, x1, y1, x2, y2 );
		}
		ecounter += 2;
	}
}

/* Set up Device
Takes a pointer to the location in the Devices array and imbues it with
startup values
parameter 't' is Device type
parameter 'x' is x coord
parameter 'y' is y coord
*/
void setupDev( device, t, x, y )
Device* device;
int t, x, y;
{
	device->type = t;
	device->value = 0;
	device->in_dev[0] = DISCNNCT;
	device->in_dev[1] = DISCNNCT;
	device->output	= DISCNNCT;
	device->x = x;
	device->y = y;
	/*device->scale = 1.0;*/
	device->shape = shapeTbl[t];
}

/* Render Device String
prints parameter 'i's Device type to terminal
*/
void rndrDvS( term, devices, i )
Geoff* term;
Device* devices;
int i;
{
	char *txtstr = "WIRE\0\0\0\0\0\0XNOR GATE\0XOR GATE\0\0NOT GATE\0\0NOR GATE\0\0OR GATE\0\0\0NAND GATE\0AND GATE\0\0SIGNAL\0\0\0\0";
	term->tputs( term, txtstr + ( devices[i].type * 10 ) );
}

/* Print Devices
populates screen's display with varioius information about Devices and selected Device
*/
void prntDev( term, devices )
Geoff* term;
Device* devices;
{
	char loc_buf[16];
	unsigned char i;
	int loc_x, loc_y = 0;

	/* Selected Device - bottom of screen */
	term->gotoxy( term, 0, 21 );
	term->tputs( term, "Device: ");
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", sel_dev );
	term->tputs(term, loc_buf);
	term->tputs( term, "\t" );
	rndrDvS( term, devices, sel_dev );
	term->tputs(term, "\tX: ");
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", devices[sel_dev].x );
	term->tputs(term, loc_buf);
	term->tputs(term, "\tY: ");
	sprintf( loc_buf, "%d", devices[sel_dev].y );
	term->tputs(term, loc_buf);
	term->tputs(term, "\tValue: ");
	if ( devices[ sel_dev ].value )
		term->tputs(term, "HIGH");
	else
		term->tputs(term, "LOW");
	term->tputchar( term, '\t' );
	if ( PRG_MODE == DEV_MODE )
		term->tputchar( term, 'D' );
	else if ( PRG_MODE == CNX_MODE )
		term->tputchar( term, 'C' );
	else if ( PRG_MODE == VID_MODE )
		term->tputchar( term, 'V' );

	/* Devices - left side of screen */
	loc_y = 1;
	/* loc_x = 57; */
	loc_x = 63;
	term->gotoxy( term, loc_x - 2, sel_dev + 1 );
	term->tputs( term, ">>");
	term->gotoxy( term, loc_x, loc_y );
	for (i = 0; i < num_dev; ++i )
	{
		memset( loc_buf, 0, sizeof(loc_buf) );
		sprintf( loc_buf, "%d", i );
		term->tputs( term, loc_buf );
		term->tputs( term, "\t" );

		rndrDvS( term, devices, i );

		loc_y++;
		term->gotoxy( term, loc_x, loc_y );
	}
}

/* Simulate Devices
simulates Device's voltages using input nodes voltages and boolean operations
*/
void simDv( devices )
Device* devices;
{
	char inp1, inp2;
	unsigned char i, j;
	for ( j = 0; j < num_cnx; ++j )
	{
		for ( i = 0; i < num_dev; ++i )
		{
			inp1 = devices[devices[i].in_dev[0]].value;
			inp2 = devices[devices[i].in_dev[1]].value;
			switch ( devices[i].type )
			{
				case WIRE:
					break;
				case XNR_GATE:
					devices[i].value = (inp1 > 0 ^ inp2 > 0 ) ? 0 : 1;
					break;
				case XOR_GATE:
					devices[i].value = (inp1 > 0 ^ inp2 > 0 ) ? 1 : 0;
					break;
				case NOT_GATE:
					devices[i].value = (inp1 > 0) ? 0 : 1;
					break;
				case NOR_GATE:
					devices[i].value = (inp1 > 0 || inp2 > 0 ) ? 0 : 1;
					break;
				case OR_GATE:
					devices[i].value = (inp1 > 0 || inp2 > 0 ) ? 1 : 0;
					break;
				case NND_GATE:
					devices[i].value = (inp1 > 0 && inp2 > 0 ) ? 0 : 1;
					break;
				case AND_GATE:
					devices[i].value = (inp1 > 0 && inp2 > 0 ) ? 1 : 0;
					break;
				case SIGNAL:
					break;
				default:
					break;
			}
		}
	}
}


char loadCirc( term, filename, devices, cnxtions )
Geoff* term;
char* filename;
Device** devices;
Cnxtion** cnxtions;
{
	return 0;
}

char saveCirc( term, filename, devices, cnxtions )
Geoff* term;
char* filename;
Device* devices;
Cnxtion* cnxtions;
{
	return 0;
}

char updt_scn( term, devices, cnxtions )
Geoff* term;
Device* devices;
Cnxtion* cnxtions;
{
	int k;
	int cdx1, cdy1, cdx2, cdy2;
	term->clear( term );
	/* if number of Devices > 0, draw Devices */
	for (k=0; k<num_dev; ++k)
	{
		drawDv( term, &devices[k] );
	}
	
	/* if number of Cnxtions > 0, draw Cnxtions */
	for (k = 0; k < num_cnx; ++k)
	{
		char x1_off, y1_off, x2_off, y2_off;
		unsigned char devsptr =  cnxtions[ k ].src_dev;
		unsigned char devtptr =  cnxtions[ k ].trg_dev;

		/* check if SIGNAL, if so, use 5 and 6 */
		if ( devices[ devsptr ].type == SIGNAL )
		{
			x1_off = 4;
			y1_off = 5;
		}
		/* else, draw from Device output line */
		else
		{
			x1_off = 14;
			y1_off = 15;
		}

		/* calculate endpoint based on input of target */
		if ( cnxtions[ k ].input == 1 )
		{
			x2_off = 2;
			y2_off = 3;
		}
		else
		{
			x2_off = 7;
			y2_off = 8;
		}

		cdx1 = devices[ devsptr ].x + devices[ devsptr ].shape[ x1_off ];
		cdy1 = devices[ devsptr ].y + devices[ devsptr ].shape[ y1_off ];
		cdx2 = devices[ devtptr ].x + devices[ devtptr ].shape[ x2_off ];
		cdy2 = devices[ devtptr ].y + devices[ devtptr ].shape[ y2_off ];
		
		term->drawLine( term,
				cdx1,
				cdy1,
				cdx2,	
				cdy2	
				);
	}

	/* draw cursor at Device if in DEV_MODE */
	if ( PRG_MODE == DEV_MODE )
	{
		if ( num_dev )
			drawCurs( term, devices[sel_dev].x, devices[sel_dev].y );
	}
	/* draw cursor at Cnxtion if in CNX_MODE */
	else if ( PRG_MODE == CNX_MODE )
	{
		if (num_cnx)
		{
			unsigned char srcIdx;
			unsigned char trgIdx;
			int srcX;
			int srcY;
			int trgX;
			int trgY;
			int srcXOff;
			int srcYOff;
			int trgXOff;
			int trgYOff;
			int midX;
			int midY;

			srcIdx = cnxtions[sel_cnx].src_dev;
			trgIdx = cnxtions[sel_cnx].trg_dev;

			if (srcIdx < num_dev && trgIdx < num_dev)
			{
				srcX = devices[srcIdx].x;
				srcY = devices[srcIdx].y;
				trgX = devices[trgIdx].x;
				trgY = devices[trgIdx].y;

				srcXOff = 0;
				srcYOff = 0;
				trgXOff = 0;
				trgYOff = 0;

				if (devices[srcIdx].type == SIGNAL)
				{
					srcXOff = 4;
					srcYOff = 5;
				}
				else
				{
					srcXOff = 14;
					srcYOff = 15;
				}

				if (cnxtions[sel_cnx].input == 1)
				{
					trgXOff = 2;
					trgYOff = 3;
				}
				else
				{
					trgXOff = 7;
					trgYOff = 8;
				}

				srcX += devices[srcIdx].shape[srcXOff];
				srcY += devices[srcIdx].shape[srcYOff];
				trgX += devices[trgIdx].shape[trgXOff];
				trgY += devices[trgIdx].shape[trgYOff];

				midX = (srcX + trgX) / 2;
				midY = (srcY + trgY) / 2;

				drawCurs( term, midX, midY );
			}
			else
			{
				
			}
		}
	}
	
	/* if number of Devices > 0, simulate then draw Device info */
	if ( num_dev )
	{
		simDv( devices );
		/* prntDev( term, devices ); */
	}
	
}

/* parses the command buffer typed in */
/* M{D/C/V}   Mode switch
   S[num]     Select entity number
   E{params}  Edit selected entity's properties
   I          Insert entity based upon [M]ode
*/
char pars_buf(term, devices, cnxtions, com_buf)
Geoff* term;
Device* devices;
Cnxtion* cnxtions;
char* com_buf;
{
	char* ptr = com_buf;
	Device* _dev_tmp = NULL;
	Cnxtion* _cnx_tmp = NULL;
	while (*ptr != '\0')
	{
		int num;
		int numStrIndex;
		char numStr[16]; /* Buffer to store the number string */
		
		switch (*ptr)
		{
			case 'M':
				ptr++; /* Move to the next character (D/C/V) */
				if (*ptr == 'D') PRG_MODE = DEV_MODE;
				else if (*ptr == 'C') PRG_MODE = CNX_MODE;
				else if (*ptr == 'V') PRG_MODE = VID_MODE;
				else
				{
					term->tputs(term, "Invalid mode after 'M'.\r\n");
				}
				ptr++; /* Move past the mode character */
				break;
			case 'S':
				ptr++; /* Move past 'S' */
				num = 0;
				while (isdigit(*ptr))
				{
					num = num * 10 + (*ptr - '0');
					ptr++;
				}
				if (PRG_MODE == DEV_MODE) sel_dev = num;
				else if (PRG_MODE == CNX_MODE) sel_cnx = num;
				else
				{
					term->tputs(term, "Select requires DEV or CNX mode.\r\n");
				}
				break;
			case 'E':
				ptr++; /* Move past 'E' */
				switch (*ptr)
				{
					case 'T':
						ptr++; /* Move past 'T' */
						if (PRG_MODE == DEV_MODE)
						{
							char* dev_type = "wzxnrodas";
							int i;
							for (i = 0; i < 9; ++i)
							{
								if (*ptr == dev_type[i]) break;
							}
							devices[sel_dev].type = devtyplu[i];
							devices[sel_dev].shape = shapeTbl[i];
							ptr++;
						}
						else
						{
							term->tputs(term, "Type edit requires DEV mode.\r\n");
							ptr++;
						}
						break;
					case 'X':
					case 'Y':
						numStrIndex = 0;
						ptr++;
						while (isdigit(*ptr))
						{
							numStr[numStrIndex++] = *ptr;
							ptr++;
						}
						numStr[numStrIndex] = '\0';
						if (PRG_MODE == DEV_MODE)
						{
							int num = atoi(numStr);
							if (*(ptr - numStrIndex - 1) == 'X')
							{
								devices[sel_dev].x = num;
							}
							else
							{
								devices[sel_dev].y = num;
							}
						}
						else
						{
							term->tputs(term, "X or Y edit requires DEV mode.\r\n");
						}
						break;
				}
				break;
			case 'I':
				ptr++; /* Move past 'E' */
				if ( PRG_MODE == DEV_MODE )
				{
					num_dev++;
					_dev_tmp=(Device*)realloc(devices,num_dev*sizeof(Device));
					if (_dev_tmp==0)
					{
						term->tputs( term, "Memory reallocation failed!");
						free(devices);
						break;
					}
					devices=_dev_tmp;
					setupDev( &devices[num_dev-1], SIGNAL, 100, 100 );
					sel_dev=num_dev-1;
					break;
				}
				else if ( PRG_MODE == CNX_MODE )
				{
					int src_dev = 0, trg_dev = 0, input = 0;
					while (isdigit(*ptr))
					{
						src_dev = src_dev * 10 + (*ptr - '0');
						ptr++;
					}
					if (*ptr != ':')
					{
						term->tputs(term, "Invalid connection format.\r\n");
						break;
					}
					ptr++;
					while (isdigit(*ptr))
					{
						trg_dev = trg_dev * 10 + (*ptr - '0');
						ptr++;
					}
					if (*ptr != ':')
					{
						term->tputs(term, "Invalid connection format.\r\n");
						break;
					}
					ptr++;
					while (isdigit(*ptr))
					{
						input = input * 10 + (*ptr - '0');
						ptr++;
					}
					if (input < 1 || input > 2 || src_dev >= num_dev || trg_dev >= num_dev)
					{
						term->tputs(term, "Invalid connection details.\r\n");
						break;
					}
					num_cnx++;
					_cnx_tmp = (Cnxtion*)realloc(cnxtions, num_cnx * sizeof(Cnxtion));
					if (_cnx_tmp == NULL)
					{
						term->tputs(term, "Memory reallocation failed!\r\n");
						free(cnxtions);
						break;
					}
					cnxtions = _cnx_tmp;
					cnxtions[num_cnx - 1].src_dev = src_dev;
					cnxtions[num_cnx - 1].trg_dev = trg_dev;
					cnxtions[num_cnx - 1].input = input;
					devices[trg_dev].in_dev[input - 1] = src_dev;
					sel_cnx = num_cnx - 1;
				}
				break;
			case 'X':
				break;
			case 'R':
				updt_scn(term, devices, cnxtions);
				ptr++; /* Move past 'R' */
				break;
			case 'Q':
				exit(0);
				break;
			default:
				term->tputs(term, "Invalid command.\r\n");
				ptr++; /* Move past the invalid character */
				break;
		}
	}
	if (_dev_tmp != NULL) free(_dev_tmp);
	if (_cnx_tmp != NULL) free(_cnx_tmp);
	return 0;
}

int main()
{
	/* local vars */
	Geoff gterm;
	Device *devices;
	Cnxtion *cnxtions;

	int i;
	char filename[16];
	char com_buf[ 70 ];
	/* char *filename = "circuit.crt"; */

	/* definitions */
	initTerm( &gterm, TTYBASE );
	num_dev = 1;
	sel_dev = 0;
	sel_cnx = 0;
	num_cnx = 0;

	/* no initial Cnxtions */
	cnxtions = NULL;

	/* malloc first Device */
	devices = (Device*)malloc(sizeof(Device));
	if (devices == NULL)
	{
		gterm.tputs( &gterm, "Memory allocation failed!");
		return 1;
	}
	setupDev( &devices[i], SIGNAL, 20, 15 );
	
	updt_scn( &gterm, devices, cnxtions );

	/* --------- MAIN LOOP ---------- */
	/* read buffer */
	while ( 1 )
	{
		/* clear lower screen */
		clrLwrSn( &gterm );
		prntDev( &gterm, devices );
		/* put ':' prompt */
		gterm.gotoxy( &gterm, 0, 22 );
		gterm.tputchar( &gterm, ':' );

		/*
		if ( PRG_MODE != VID_MODE )
		{
			*/
			/* read buffer */
			_scanf( &gterm, 's', com_buf );
		/*
		}
		else
		{
			
		}
		*/

		/* parse command buffer */
		if ( pars_buf( &gterm, devices, cnxtions, com_buf ) < 0 )
			break;
	}

	gterm.clear( &gterm );
	
	free(devices);
	free(cnxtions);

	return 0;
}
