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

/* Aztec C seems to need functions from any library other than c.lib to be extern'd */
extern double atof();

int num_dev;		/* number of devices */
int num_cnx;		/* number of connections */
int sel_dev;		/* selected device */
int sel_cnx;		/* selected connection */
char cursmode;		/* selection/cursor mode */

/* shape contains a 1D array describing the shape
   First element is number of line segments
   Next three lines are the input and output lines ( 2 input and 1 output )
   Successive lines make up the rest of the shape
   Each Device is drawn locally to the provided
   x and y coordinates.
*/

static int sig_dat[] = { 1,
			0, 0, 20, 0
			};
	
static int and_dat[] = { 9,
			0, 5, 5, 5,
			0, 30, 5, 30,
			40, 17, 45, 17,
			5, 0, 30, 0,
			30, 0, 40, 10,
			40, 10, 40, 25,
			40, 25, 30, 35,
			30, 35, 5, 35,
			5, 35, 5, 0
			};
static int nand_dat[] = { 13,
			0, 5, 5, 5,
			0, 30, 5, 30,
			46, 17, 51, 17,
			5, 0, 30, 0,
			30, 0, 40, 10,
			40, 10, 40, 25,
			40, 17, 43, 15,
			43, 15, 46, 17,
			46, 17, 43, 19,
			43, 19, 40, 17,
			40, 25, 30, 35,
			30, 35, 5, 35,
			5, 35, 5, 0
			};

static int nor_dat[] = {15,
			0, 5, 9, 5,
			0, 30, 9, 30,
			46, 17, 51, 17,
			5, 0, 30, 0,
			30, 0, 40, 10,
			40, 10, 40, 25,
			40, 17, 43, 15,
			43, 15, 46, 17,
			46, 17, 43, 19,
			43, 19, 40, 17,
			40, 25, 30, 35,
			30, 35, 5, 35,
			5, 35, 15, 25,
			15, 25, 15, 10,
			15, 10, 5, 0
			};

static int or_dat[] = { 11,
			0, 5, 9, 5,
			0, 30, 9, 30,
			40, 17, 45, 17,
			5, 0, 30, 0,
			30, 0, 40, 10,
			40, 10, 40, 25,
			40, 25, 30, 35,
			30, 35, 5, 35,
			5, 35, 15, 25,
			15, 25, 15, 10,
			15, 10, 5, 0
			};

static int xor_dat[] = { 14,
			0, 5, 9, 5,
			0, 30, 9, 30,
			40, 17, 45, 17,
			8, 0, 30, 0,
			30, 0, 40, 10,
			40, 10, 40, 25,
			40, 25, 30, 35,
			30, 35, 8, 35,
			8, 35, 18, 25,
			18, 25, 18, 10,
			18, 10, 8, 0,
			5, 35, 15, 25,
			15, 25, 15, 10,
			15, 10, 5, 0
			};

static int not_dat[] = { 10,
			0, 17, 9, 17,
			9, 0, 9, 0,
			46, 17, 51, 17,
			40, 17, 43, 15,
			43, 15, 46, 17,
			46, 17, 43, 19,
			43, 19, 40, 17,
			9, 0, 40, 17,
			40, 17, 9, 35,
			9, 35, 9, 0
			};

/* Device struct
A Device in this sense is any elemnt you draw on the screen that is not
text, the cursor, or airwires
*/
typedef struct sDevice
{
	char type;			/* type of Device */
	int x, y;			/* coordinates to draw */
	int* shape;			/* pointer to which Device to draw */
	double value;			/* Voltage value of Device */
	float scale;			/* Scale to draw locally */
	int in_dev[2];			/* Two inputs. Designate which devices are wired up */
	char output;			/* Designates which output device is hooked up */
} Device;

/* Cnxtion struct
A Cnxtion is a connection between two Devices
*/
typedef struct sCnxtion
{
	int src_x, src_y;		/* source Device's xy coords */
	int trg_x, trg_y;		/* target Device's xy coords */
	int src_dev;			/* source Device index */
	int trg_dev;			/* target Device index */
	char input;			/* input Device index */
	char value;			/* Voltage value of Cnxtion */
} Cnxtion;

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
		/* term->putchar( term, buf[i++] = c ); */
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
	else if ( f == 'f' )
		*(double*)a = atof(buf);
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
   Need to rethink the structure and drawing algo to
   efficiently draw with rectangles and circles, fill or no fill
*/
void drawDv( term, device)
Geoff* term;
Device* device;
{
	int i;
	int numseg;
	numseg = device->shape[0] * 4;
	for ( i = 1; i < numseg; i+=4 )
	{
		float x, y, u, v;
		/* multiplied against scale to scale Device */
		x = device->x + (device->shape[i]   * device->scale);
		y = device->y + (device->shape[i+1] * device->scale);
		u = device->x + (device->shape[i+2] * device->scale);
		v = device->y + (device->shape[i+3] * device->scale);
		term->drawLine( term,
				(int)x,
				(int)y,
				(int)u,
				(int)v
				);
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
	input = 0;
	
	/* choose device type */
	do
	{
		clrLwrSn( term );
		term->puts( term, "Type: [A]nd  [O]r  [S]ignal  [X]or  No[R]\r\n[N]ot  Nan[D]  [W]ire  [>]Next  [Q]uit");
		input = term->getch( term );
		input |= 32;				/* turn to lowercase */
		if (input=='a')
		{
			device->type=AND_GATE;
			device->shape=and_dat;
			break;
		}
		else if (input=='o')
		{
			device->type=OR_GATE;
			device->shape=or_dat;
			break;
		}
		else if (input=='s')
		{
			device->type=SIGNAL;
			device->shape=sig_dat;
			break;
		}
		else if (input=='x')
		{
			device->type=XOR_GATE;
			device->shape=xor_dat;
			break;
		}
		else if (input=='d')
		{
			device->type=NND_GATE;
			device->shape=nand_dat;
			break;
		}
		else if (input=='r')
		{
			device->type=NOR_GATE;
			device->shape=nor_dat;
			break;
		}
		else if (input=='n')
		{
			device->type=NOT_GATE;
			device->shape=not_dat;
			break;
		}
		else if (input=='>')
		{
			break;
		}
		else if (input=='w')
			break;
		else
			continue;
	}
	while (input != 'q');
	if ( input == 'q' )
		return;
	/* choose value */
	clrLwrSn( term );
	term->puts( term, "Enter a value: " );
	_scanf( term, 'f', &device->value);
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
	device->value = 0.0;
	device->in_dev[0] = -1;
	device->in_dev[1] = -1;
	device->output	= -1;
	device->x = x;
	device->y = y;
	device->scale = 1.0;
	if ( t == SIGNAL )
		device->shape = sig_dat;
	else if ( t == AND_GATE )
		device->shape = and_dat;
	else if ( t == OR_GATE )
		device->shape = or_dat;
	else if ( t == XOR_GATE )
		device->shape = xor_dat;
	else if ( t == NND_GATE )
		device->shape = nand_dat;
	else if ( t == NOR_GATE )
		device->shape = nor_dat;
	else if ( t == NOT_GATE )
		device->shape = not_dat;
	
}

/* Update Cnxtion structure
used to update the Cnxtion structure's coordinates after moves and such
*/
void updateCn( devices, cnx)
Device* devices;
Cnxtion* cnx;
{
	int i;
	int src_dev;
	int trg_dev;
	char input;
	for (i = 0; i < num_cnx; i++)
	{
		src_dev = cnx[i].src_dev;
		trg_dev = cnx[i].trg_dev;
		input = cnx[i].input;

		/* Calculate source coordinates */
		if ( devices[src_dev].shape[0] == 1 )
		{
			cnx[i].src_y = devices[src_dev].y + 0;   /* Adjust as needed based on your device shape */
			cnx[i].src_x = devices[src_dev].x + 20; /* Adjust as needed based on your device shape */
		}
		else
		{
			cnx[i].src_x = devices[src_dev].x + devices[src_dev].shape[11];
			cnx[i].src_y = devices[src_dev].y + devices[src_dev].shape[12]; 
		}

		/* Calculate target coordinates based on input port */
		if (input == 1)
		{
			cnx[i].trg_x = devices[trg_dev].x + devices[trg_dev].shape[1]; 
			cnx[i].trg_y = devices[trg_dev].y + devices[trg_dev].shape[2];
		}
		else
		{
			cnx[i].trg_x = devices[trg_dev].x + devices[trg_dev].shape[5]; 
			cnx[i].trg_y = devices[trg_dev].y + devices[trg_dev].shape[6]; 
		}
        }
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
	int src_x, src_y, trg_x, trg_y;
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

	/* rethink this how */
	/* Calculate source coordinates */
	if ( dev[src_dev].shape[0] == 1 )
	{
		cnx[num_cnx].src_y = dev[src_dev].y + 0;   /* Adjust as needed based on your device shape */
		cnx[num_cnx].src_x = dev[src_dev].x + 20; /* Adjust as needed based on your device shape */
	}
	else
	{
		cnx[num_cnx].src_x = dev[src_dev].x + dev[src_dev].shape[11];
		cnx[num_cnx].src_y = dev[src_dev].y + dev[src_dev].shape[12]; 
	}

	if (inpt == 1) {
	    trg_x = dev[trg_dev].x + dev[trg_dev].shape[1];
	    trg_y = dev[trg_dev].y + dev[trg_dev].shape[2];
	} else {
	    trg_x = dev[trg_dev].x + dev[trg_dev].shape[5];
	    trg_y = dev[trg_dev].y + dev[trg_dev].shape[6];
	}

	cnx[num_cnx].src_x = src_x;
	cnx[num_cnx].src_y = src_y;
	cnx[num_cnx].trg_x = trg_x;
	cnx[num_cnx].trg_y = trg_y;
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
	sprintf( loc_buf, "%.02lf", devices[sel_dev].value );
	term->puts(term, loc_buf);
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
 turn this into a jump table
*/
void simDv( devices )
Device* devices;
{
	double inp1, inp2;
	unsigned char i, j;
	for ( j = 0; j < num_cnx; ++j )
	{
		for ( i = 0; i < num_dev; ++i )
		{
			inp1 = devices[devices[i].in_dev[0]].value;
			inp2 = devices[devices[i].in_dev[1]].value;
			switch ( devices[i].type )
			{
				case AND_GATE:
					devices[i].value = (inp1 > 0.5 && inp2 > 0.5 ) ? 5.0 : 0.0;
					break;
				case OR_GATE:
					devices[i].value = (inp1 > 0.5 || inp2 > 0.5 ) ? 5.0 : 0.0;
					break;
				case XOR_GATE:
					devices[i].value = (inp1 > 0.5 ^ inp2 > 0.5 ) ? 5.0 : 0.0;
					break;
				case NND_GATE:
					devices[i].value = (inp1 > 0.5 && inp2 > 0.5 ) ? 0.0 : 5.0;
					break;
				case NOR_GATE:
					devices[i].value = (inp1 > 0.5 || inp2 > 0.5 ) ? 0.0 : 5.0;
					break;
				case NOT_GATE:
					devices[i].value = (inp1 > 0.5) ? 0.0 : 5.0;
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
		
		/* if number of Cnxtions > 0, update Cnxtion values */
		if ( num_cnx )
			updateCn( devices, cnxtions );

		/* if number of Cnxtions > 0, draw Cnxtions */
		for (k = 0; k < num_cnx; ++k)
		{
			gterm.drawLine( &gterm,
					cnxtions[k].src_x, cnxtions[k].src_y,
					cnxtions[k].trg_x, cnxtions[k].trg_y
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
			if ( num_cnx )
				drawCurs( &gterm, (cnxtions[sel_cnx].src_x + cnxtions[sel_cnx].trg_x) / 2,
						  (cnxtions[sel_cnx].src_y + cnxtions[sel_cnx].trg_y) / 2);
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
			case '+':
				devices[sel_dev].scale += 0.2;
				break;
			case '-':
				devices[sel_dev].scale -= 0.2;
				break;
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