/* #include "cpm.h" */
#include "geofftrm.h
/* If not using the include sources,
please link against my cpm and geofftrm libraries 
*/

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define	MAX_CNX	32
#define	SIOSTAT	16
#define	SIODATA	17
#define	SCANBUF	15

#define	NULL	0

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

extern double atof();
/* extern int atoi(); */

int num_dev;		/* number of devices */
int num_cnx;		/* number of connections */
int sel_dev;		/* selected device */
int sel_cnx;		/* selected connection */
char cursmode;		/* selection/cursor mode */

/* shape contains a 1D array describing the shape
   First element is number of line segments
   Successive element pairs are line segment
   coordinates
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

typedef struct sDevice
{
	char type;
	int x, y;
	int* shape;
	double value;
	float scale;
	int in_dev[2];
	char output;
} Device;

typedef struct sCnxtion
{
	int src_x, src_y;
	int trg_x, trg_y;
	int src_dev;
	int trg_dev;
	char input;
	char value;
} Cnxtion;

/*
char getch()
{
	while (!(in(SIOSTAT) & 1));
	return in(SIODATA);
}
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
		/* c = getch(); */
		c = term->getch( term );
		if (c == '\r' || c == '\n')
		{
			i++;
			break;
		}
		else if (c == 0x08 && i > 0)
		{
			term->puts( term, "\010 \010" );
			/*
			term->putchar( term, 0x08 );
			term->putchar( term, ' ' );
			term->putchar( term, 0x08 );
			*/
			i--; 
			continue;
		}
		/*putchar(buf[i++] = c);*/
		term->putchar( term, buf[i++] = c );
	}
	buf[i] = 0;
	term->puts( term, "");

	if ( f == 'd' )
		*(int*)a = atoi(buf);
	else if ( f == 'f' )
		*(double*)a = atof(buf);
}

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
		/*puts("Type: [A]nd  [O]r  [S]ignal  [X]or\n[N]AND  [W]ire  [>]Next  [Q]uit");*/
		term->gotoxy( term, 0, 21 );
		term->puts( term, "\033[0J" );
		term->puts( term, "Type: [A]nd  [O]r  [S]ignal  [X]or  No[R]\r\n[N]ot  Nan[D]  [W]ire  [>]Next  [Q]uit");
		/* input = getch(); */
		input = term->getch( term );
		input |= 32;
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
	/*puts("Enter a value:");*/
	term->gotoxy( term, 0, 21 );
	term->puts( term, "\033[0J" );
	term->puts( term, "Enter a value: " );
	_scanf( term, 'f', &device->value);
}

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

void connDv( term, dev, cnx )
Geoff* term;
Device* dev;
Cnxtion* cnx;
{
	char loc_buf[8];
	int src_dev, trg_dev, inpt;
	int src_x, src_y, trg_x, trg_y;
	num_cnx--;
	/* printf("Connect Devices\nSelect Source Device (0-%d): \n", num_dev - 1); */
	
	term->gotoxy( term, 0, 21 );
	term->puts( term, "\033[0J" );
	term->puts( term, "Connect Devices\r\nSelect Source Device (0-" );
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", num_dev - 1 );
	term->puts( term, loc_buf );
	term->puts( term, "): \r\n" );
	_scanf( term, 'd', &src_dev);
	/*printf("Select Target Device (0-%d): \n", num_dev - 1);*/
	
	term->gotoxy( term, 0, 21 );
	term->puts( term, "\033[0J" );
	term->puts( term, "Select Target Device (0-" );
	memset( loc_buf, 0, sizeof(loc_buf) );
	sprintf( loc_buf, "%d", num_dev - 1 );
	term->puts( term, loc_buf );
	term->puts( term, "): \r\n" );
	_scanf( term, 'd', &trg_dev);
	/*printf("Select Target Device Input [1] or [2]:");*/
	
	term->gotoxy( term, 0, 21 );
	term->puts( term, "\033[0J" );
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

	num_cnx++;
}

void rndrDvS( term, devices, i )
Geoff* term;
Device* devices;
int i;
{
	switch ( devices[i].type )
	{
		case SIGNAL:
			term->puts( term, "SIGNAL" );
			break;
		case AND_GATE:
			term->puts( term, "AND GATE" );
			break;
		case OR_GATE:
			term->puts( term, "OR GATE" );
			break;
		case XOR_GATE:
			term->puts( term, "XOR GATE" );
			break;
		case NND_GATE:
			term->puts( term, "NAND GATE" );
			break;
		case NOR_GATE:
			term->puts( term, "NOR GATE" );
			break;
		case NOT_GATE:
			term->puts( term, "NOT GATE" );
			break;
		default:
			term->puts( term, "UNKNOWN" );
			break;
	}
}

void prntDev( term, devices )
Geoff* term;
Device* devices;
{
	char loc_buf[16];
	unsigned char i;
	int loc_x, loc_y = 0;
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

	loc_y = 1;
	/* loc_x = 57; */
	loc_x = 63;
	term->gotoxy( term, loc_x - 2, sel_dev + 1 );
	term->puts( term, ">>");
	term->gotoxy( term, loc_x, loc_y );
	for (i = 0; i < num_dev; ++i )
	{
		/* term->puts( term, "Device: " ); */
		memset( loc_buf, 0, sizeof(loc_buf) );
		sprintf( loc_buf, "%d", i );
		term->puts( term, loc_buf );
		term->puts( term, "\t" );

		rndrDvS( term, devices, i );

		loc_y++;
		term->gotoxy( term, loc_x, loc_y );
	}
}

/* turn this into a jump table */
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
	initTerm( &gterm, 16 );
	cursmode = DEV_MODE;
	num_dev = 1;
	sel_dev = 0;
	sel_cnx = 0;
	num_cnx = 0;

	cnxtions = NULL;

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
		/* drawing stuff */
		gterm.clear( &gterm );
		if ( num_dev )
		{
			simDv( devices );
			prntDev( &gterm, devices );
			updateCn( devices, cnxtions );
		}
		for (k=0; k<num_dev; ++k)
		{
			drawDv( &gterm, &devices[k] );
		}

		if ( cursmode == DEV_MODE )
		{
			if ( num_dev )
				drawCurs( &gterm, devices[sel_dev].x, devices[sel_dev].y );
		}
		else if ( cursmode == CNX_MODE )
		{
			if ( num_cnx )
				drawCurs( &gterm, (cnxtions[sel_cnx].src_x + cnxtions[sel_cnx].trg_x) / 2,
						  (cnxtions[sel_cnx].src_y + cnxtions[sel_cnx].trg_y) / 2);
		}
		
		for (k = 0; k < num_cnx; ++k)
		{
			gterm.drawLine( &gterm,
					cnxtions[k].src_x, cnxtions[k].src_y,
					cnxtions[k].trg_x, cnxtions[k].trg_y
					);
		}

		/* k_in = getch(); */
		k_in = gterm.getch( &gterm );
		switch ( k_in )
		{
			if ( num_dev == 0 )
				break;
			case 'R':
				simDv( devices );
				break;
			case 'J':
				gterm.gotoxy( &gterm, 0, 21 );
				gterm.puts( &gterm, "\033[0J" );
				gterm.puts( &gterm, "Jump to device: " );
				sel_dev = -1;
				while ( sel_dev < 0 && sel_dev < num_dev )
				{
					_scanf( &gterm, 'd', &sel_dev);
				}
				break;
			case 'M':
				if ( cursmode == DEV_MODE )
					cursmode = CNX_MODE;
				else if ( cursmode == CNX_MODE )
					cursmode = DEV_MODE;
				break;
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
			case '+':
				devices[sel_dev].scale += 0.2;
				break;
			case '-':
				devices[sel_dev].scale -= 0.2;
				break;
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
			case 'e':
				/* will eventually port this to geoftrm */
				editDv( &gterm, &devices[sel_dev] );
				break;
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