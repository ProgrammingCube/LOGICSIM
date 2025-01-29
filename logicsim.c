/* #include "cpm.h" */
#include "geofftrm.h
/* If not using the include sources,
please link against my cpm and geofftrm libraries 
*/

/* maximum number of Cnxtions */
#define	MAX_CNX	256
/* maximum number of Devices */
#define	MAX_DEV	256
/* size of scanf buffer */
#define	SCANBUF	15

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

unsigned char num_dev;		/* number of devices */
unsigned char num_cnx;		/* number of connections */
unsigned char sel_dev;		/* selected device */
unsigned char sel_cnx;		/* selected connection */
char cursmode;		/* selection/cursor mode */

/* shape contains a 1D array describing the shape
   First element is number of continuous lines
   Next three lines are the input and output lines ( 2 input and 1 output )
   Successive lines make up the continuous lines to draw
   First element of these lines show the number of points on that line
*/

static char sig_dat[] = { 1,			/* length */
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
			NULL,		/* wire_dat */
			xnr_dat,	/* xnor_dat */
			xor_dat,
			not_dat,
			nor_dat,
			or_dat,
			nand_dat,
			and_dat,
			sig_dat,
			NULL		/* null terminator */
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
	unsigned char in_dev[2];	/* Two inputs. Designate which devices are wired up */
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

/* function table for editDevice */
typedef void (*edDevptr)();

void _andStup( device )
Device* device;
{
	device->type=AND_GATE;
	device->shape=and_dat;
}

void _orStup( device )
Device* device;
{
	device->type=OR_GATE;
	device->shape=or_dat;
}

void _sigStup( device )
Device* device;
{
	device->type=SIGNAL;
	device->shape=sig_dat;
}

void _xorStup( device )
Device* device;
{
	device->type=XOR_GATE;
	device->shape=xor_dat;
}

void _nndStup( device )
Device* device;
{
	device->type=NND_GATE;
	device->shape=nand_dat;
}

void _norStup( device )
Device* device;
{
	device->type=NOR_GATE;
	device->shape=nor_dat;
}

void _notStup( device )
Device* device;
{
	device->type=NOT_GATE;
	device->shape=not_dat;
}

void _xnrStup( device )
Device* device;
{
	device->shape=xnr_dat;
	device->type=XNR_GATE;
}

edDevptr edDvTbl[] =
{
	_andStup,
	_orStup,
	_sigStup,
	_xorStup,
	_nndStup,
	_norStup,
	_notStup,
	_xnrStup,
	NULL 
};

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
			term->puts( term, "\010 \010" );
			i--; 
			continue;
		}
		/* print char and increment buffer */
		term->putchar( term, buf[i++] = c );
	}
	/* eol */
	buf[i] = 0;

	/* format check */
	if ( f == 'd' )
		*(int*)a = atoi(buf);
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
	term->puts( term, "\033[0J" );
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

/* Edit Device
edit the currently selected Device
the Device is passed in via a pointer to that element in the Device structure
*/
void editDv( term, device )
Geoff* term;
Device* device;
{
	char input;
	float tmp_val;
	char i;
	char *inArray = "aosxrndz";
	input = 0;
	
	/* choose device type */
	clrLwrSn( term );
	term->puts( term, "Type: [A]nd  [O]r  [S]ignal  [X]or  No[R]\r\n[N]ot  Nan[D]  [W]ire  [Z]Xnor  [>]Next  [Q]uit");
	/* grab input */
	input = term->getch( term );
	input |= 32;				/* turn to lowercase */

	/* check if quit or continue */
	if ( input == 'q' )
		return;
	
	if ( input == '>' )
	{
		goto edDvLblc;
	}

	/* iterate/compare through list */
	for ( i = 0; i < 9; ++i )
	{
		if ( input == inArray[ i ] )
			break;
	}
	/* jump to function in array on index */
	if ( ! ( i == 8 ) )
		edDvTbl[i]( device );
	else
		return;

edDvLblc:
	/* choose value */
	clrLwrSn( term );
	term->puts( term, "Enter a value [0] or [1]: " );
	_scanf( term, 'd', &device->value);
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

/* Connect Devices
selects two Devices and creates a Cnxtion using the freshly realloc'ed array
*/
void connDv( term, dev, cnx )
Geoff* term;
Device* dev;
Cnxtion* cnx;
{
	char loc_buf[8];
	int src_dev, trg_dev, inpt;
	num_cnx--;				/* decrement counter from realloc */
	
	/* simple menu to select source, target, and port to connect to */
	clrLwrSn( term );
	term->puts( term, "Connect Devices\r\nSelect Source Device (0-" );
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", num_dev - 1 );
	term->puts( term, loc_buf );
	term->puts( term, "): \r\n" );
	_scanf( term, 'd', &src_dev);
	clrLwrSn( term );
	term->puts( term, "Select Target Device (0-" );
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", num_dev - 1 );
	term->puts( term, loc_buf );
	term->puts( term, "): \r\n" );
	_scanf( term, 'd', &trg_dev);
	clrLwrSn( term );
	term->puts( term, "Select Target Device Input [1] or [2]:" );
	_scanf( term, 'd', &inpt);

	cnx[num_cnx].input = inpt;
	dev[trg_dev].in_dev[inpt - 1] = src_dev;
	cnx[num_cnx].src_dev = src_dev;
	cnx[num_cnx].trg_dev = trg_dev;

	num_cnx++;				/* reincrement number of cnxtions */
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
	term->puts( term, txtstr + ( devices[i].type * 10 ) );
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
	term->puts( term, "Device: ");
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", sel_dev );
	term->puts(term, loc_buf);
	term->puts( term, "\t" );
	rndrDvS( term, devices, sel_dev );
	term->puts(term, "\tX: ");
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", devices[sel_dev].x );
	term->puts(term, loc_buf);
	term->puts(term, "\tY: ");
	sprintf( loc_buf, "%d", devices[sel_dev].y );
	term->puts(term, loc_buf);
	term->puts(term, "\tValue: ");
	if ( devices[ sel_dev ].value )
		term->puts(term, "HIGH");
	else
		term->puts(term, "LOW");
	term->putchar( term, '\t' );
	if ( cursmode == DEV_MODE )
		term->putchar( term, 'D' );
	else if ( cursmode == CNX_MODE )
		term->putchar( term, 'C' );

	/* Devices - left side of screen */
	loc_y = 1;
	/* loc_x = 57; */
	loc_x = 63;
	term->gotoxy( term, loc_x - 2, sel_dev + 1 );
	term->puts( term, ">>");
	term->gotoxy( term, loc_x, loc_y );
	for (i = 0; i < num_dev; ++i )
	{
		memset( loc_buf, 0, sizeof(loc_buf) );
		sprintf( loc_buf, "%d", i );
		term->puts( term, loc_buf );
		term->puts( term, "\t" );

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

int main()
{
	/* local vars */
	Geoff gterm;
	Device* devices;
	Device* _dev_tmp;
	Cnxtion* cnxtions;
	Cnxtion* _cnx_tmp;

	int i;
	char k_in;

	/* definitions */
	initTerm( &gterm, 18 );
	cursmode = DEV_MODE;
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
		gterm.puts( &gterm, "Memory allocation failed!");
		return 1;
	}
	setupDev( &devices[i], SIGNAL, 20, 15 );

	do
	{
		int k;
		int cdx1, cdy1, cdx2, cdy2;
		gterm.clear( &gterm );
		
		/* if number of Devices > 0, draw Devices */
		for (k=0; k<num_dev; ++k)
		{
			drawDv( &gterm, &devices[k] );
		}
		/* if number of Devices > 0, simulate then draw Device info */
		if ( num_dev )
		{
			simDv( devices );
			prntDev( &gterm, devices );
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
			
			gterm.drawLine( &gterm,
					cdx1,
					cdy1,
					cdx2,	
					cdy2	
					);
		}

		/* draw cursor at Device if in DEV_MODE */
		if ( cursmode == DEV_MODE )
		{
			if ( num_dev )
				drawCurs( &gterm, devices[sel_dev].x, devices[sel_dev].y );
		}
		/* draw cursor at Cnxtion if in CNX_MODE */
		else if ( cursmode == CNX_MODE )
		{
			/*
			if ( num_cnx )
				drawCurs( &gterm, (cnxtions[sel_cnx].src_x + cnxtions[sel_cnx].trg_x) / 2,
						  (cnxtions[sel_cnx].src_y + cnxtions[sel_cnx].trg_y) / 2);
			*/
		}

		/* get keypress */
		k_in = gterm.getch( &gterm );
		switch ( k_in )
		{
			/* if no devices, do nothing at all */
			if ( num_dev == 0 )
				break;
			/* force a simulation render */
			case 'R':
				break;
			/* jump to Device instead of scrolling */
			case 'J':
				clrLwrSn( &gterm );
				gterm.puts( &gterm, "Jump to device: " );
				sel_dev = -1;
				while ( sel_dev < 0 && sel_dev < num_dev )
				{
					_scanf( &gterm, 'd', &sel_dev);
				}
				break;
			/* change mode between Device and Cnxtion */
			case 'M':
				if ( cursmode == DEV_MODE )
					cursmode = CNX_MODE;
				else if ( cursmode == CNX_MODE )
					cursmode = DEV_MODE;
				break;
			/* move Devices around the screen s well as moving them alot */
			case 'd':
				devices[sel_dev].x += 10;
				break;
			case 'D':
				devices[sel_dev].x += 50;
				break;
			case 'a':
				devices[sel_dev].x -= 10;
				break;
			case 'A':
				devices[sel_dev].x -= 50;
				break;
			case 'w':
				devices[sel_dev].y -= 10;
				break;
			case 'W':
				devices[sel_dev].y -= 50;
				break;
			case 's':
				devices[sel_dev].y += 10;
				break;
			case 'S':
				devices[sel_dev].y += 50;
				break;
			/* change Device's scale */
			/*
			case '+':
				devices[sel_dev].scale += 0.2;
				break;
			case '-':
				devices[sel_dev].scale -= 0.2;
				break;
			*/
			/* scroll through selected Devices or Cnxtions */
			case '>':
				if ( cursmode == DEV_MODE )
					sel_dev = (sel_dev + 1) >= num_dev ? 0 : sel_dev + 1;
				else if ( cursmode == CNX_MODE )
					sel_cnx = (sel_cnx + 1) >= num_cnx ? 0 : sel_cnx + 1;
				break;
			case '<':
				if ( cursmode == DEV_MODE )
					sel_dev = (sel_dev - 1) < 0 ? num_dev - 1 : sel_dev - 1;
				else if ( cursmode == CNX_MODE )
					sel_cnx = (sel_cnx - 1) < 0 ? num_cnx - 1 : sel_cnx - 1;
				break;
			/* edit selected Device */
			case 'e':
				editDv( &gterm, &devices[sel_dev] );
				break;
			/* insert Device or Cnxtion depending on cursmode */
			case 'i':
				if ( cursmode == DEV_MODE )
				{
					num_dev++;
					_dev_tmp=(Device*)realloc(devices,num_dev*sizeof(Device));
					if (_dev_tmp==0)
					{
						gterm.puts( &gterm, "Memory reallocation failed!");
						free(devices);
						return 1;
					}
					devices=_dev_tmp;
					setupDev( &devices[num_dev-1], SIGNAL, 100, 100 );
					sel_dev=num_dev-1;
					break;
				}
				else if ( cursmode == CNX_MODE )
				{
					num_cnx++;
					_cnx_tmp=(Cnxtion*)realloc(cnxtions,num_cnx*sizeof(Cnxtion));
					if (_cnx_tmp==0)
					{
						gterm.puts( &gterm, "Memory reallocation failed!");
						free(cnxtions);
						return 1;
					}
					cnxtions=_cnx_tmp;
					connDv( &gterm, devices, cnxtions );
					break;
				}
			/* delete Device or Cnxtion depending on cursmode */
			case 'x':
				if ( cursmode == DEV_MODE )
				{
					if (num_dev>0)
					{
						char i;
						for (i=sel_dev; i<num_dev; ++i)
						{
							devices[i]=devices[i+1];
						}
						num_dev--;
					}
					if ( num_dev == 0 )
					{
						free(devices);
						devices = NULL;
						sel_dev = 0;
					}
					else
					{
						_dev_tmp=(Device*)realloc(devices,num_dev*sizeof(Device));
						if (_dev_tmp == NULL)
						{
							gterm.puts( &gterm, "Memory reallocation failed!");
							free(devices);
							return 1;
						}
						devices=_dev_tmp;
						sel_dev=num_dev-1;
					}
					break;
				}
				else if ( cursmode == CNX_MODE )
				{
					if (num_cnx>0)
					{
						char i;
						for (i=sel_cnx; i<num_cnx; ++i)
						{
							cnxtions[i]=cnxtions[i+1];
						}
						num_cnx--;
					}
					if ( num_cnx == 0 )
					{
						free(cnxtions);
						cnxtions = NULL;
						sel_cnx = 0;
					}
					else
					{
						_cnx_tmp=(Cnxtion*)realloc(cnxtions,num_cnx*sizeof(Cnxtion));
						if (_cnx_tmp == NULL)
						{
							gterm.puts( &gterm, "Memory reallocation failed!");
							free(cnxtions);
							return 1;
						}
						cnxtions=_cnx_tmp;
						sel_cnx=num_cnx-1;
					}
					break;
				}
			default:
				break;
		}
	} while ( k_in != 'q' );

	gterm.clear( &gterm );
	
	free(devices);
	free(_dev_tmp);
	free(cnxtions);
	free(_cnx_tmp);

	return 0;
}