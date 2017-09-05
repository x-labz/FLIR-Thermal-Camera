#ifndef FLIR_H_
#define FLIR_H_

#define LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT              1000

static const uint16_t color_palette[256] = {0,4096,4097,6146,8195,8195,8196,10245,10246,12295,12295,12296,12297,14346,14346,14347,16396,16396,16397,16398,16399,18447,18448,18449,18449,20498,20499,20499,20500,20500,20501,22550,22550,22551,22551,22552,24600,24601,24601,24602,24602,24602,24603,26651,26652,26652,26652,26653,26653,28701,28702,28702,28702,28702,28702,28703,28703,30751,30751,30751,30751,30751,30783,30783,32831,32831,32831,32831,32831,32831,32831,32831,32831,34879,34878,34878,34878,34878,34878,34877,34909,34909,36956,36956,36956,36955,36955,36954,36954,36954,36953,39001,39032,39032,39031,39031,39030,39030,39029,39028,39028,41107,41107,41106,41105,41105,41104,41103,41103,41134,41133,43180,43180,43179,43178,43178,43209,43208,43207,43207,43206,43205,45284,45283,45283,45282,45281,45280,45312,45312,45312,45312,45312,47392,47392,47392,47392,47392,47424,47424,47424,47424,47456,47456,47456,49504,49504,49536,49536,49536,49536,49568,49568,49568,49600,49600,49600,51648,51680,51680,51680,51680,51712,51712,51712,51744,51744,51744,51776,51776,53824,53856,53856,53856,53888,53888,53888,53920,53920,53920,53952,53952,53984,56032,56032,56064,56064,56064,56096,56096,56128,56128,56160,56160,56160,56192,56192,58272,58272,58304,58304,58304,58336,58336,58368,58368,58400,58400,58432,58432,58464,60512,60544,60544,60576,60576,60608,60608,60640,60672,60672,60704,60704,60736,60736,60768,62816,62848,62880,62880,62912,62912,62944,62976,62976,63008,63040,63040,63072,63072,63104,63136,65184,65216,65248,65280,65280,65312,65344,65344,65376,65408,65408,65440,65472,65504} ;
static const uint16_t green_palette[256] = {0,0,0,0,32,32,32,32,64,64,64,64,96,96,96,96,128,128,128,128,160,160,160,160,192,192,192,192,224,224,224,224,256,256,256,256,288,288,288,288,320,320,320,320,352,352,352,352,384,384,384,384,416,416,416,416,448,448,448,448,480,480,480,480,512,512,512,512,544,544,544,544,576,576,576,576,608,608,608,608,640,640,640,640,672,672,672,672,704,704,704,704,736,736,736,736,768,768,768,768,800,800,800,800,832,832,832,832,864,864,864,864,896,896,896,896,928,928,928,928,960,960,960,960,992,992,992,992,1024,1024,1024,1024,1056,1056,1056,1056,1088,1088,1088,1088,1120,1120,1120,1120,1152,1152,1152,1152,1184,1184,1184,1184,1216,1216,1216,1216,1248,1248,1248,1248,1280,1280,1280,1280,1312,1312,1312,1312,1344,1344,1344,1344,1376,1376,1376,1376,1408,1408,1408,1408,1440,1440,1440,1440,1472,1472,1472,1472,1504,1504,1504,1504,1536,1536,1536,1536,1568,1568,1568,1568,1600,1600,1600,1600,1632,1632,1632,1632,1664,1664,1664,1664,1696,1696,1696,1696,1728,1728,1728,1728,1760,1760,1760,1760,1792,1792,1792,1792,1824,1824,1824,1824,1856,1856,1856,1856,1888,1888,1888,1888,1920,1920,1920,1920,1952,1952,1952,1952,1984,1984,1984,1984,2016,2016,2016,2016};

typedef struct  {
	uint8_t master;
	uint8_t r;
	uint8_t g;
	uint8_t b;
} brightness_type;

static const brightness_type brightness_levels[4] = {
	{ 0x0f, 0xff, 0xff, 0xff },
	{ 0x0f, 0x75, 0x60, 0x6a },
	{ 0x09, 0x86, 0x69, 0x7c },
	{ 0x05, 0x80, 0x6b, 0x7f }
};

typedef struct {
	uint16_t x_start;
	uint16_t y_start;
	uint16_t x_end;
	uint16_t y_end;
} ROI_type ;

static const ROI_type ROI_settings[4] = {
	
		{  0,  0,79,59 },
		{ 10,  7,69,52 },
		{ 20, 14,59,45 },
		{ 30, 21,49,38 }
		
	} ;
	
	typedef enum Result
	{
		LEP_OK                            = 0,     /* Camera ok */
		LEP_COMM_OK                       = LEP_OK, /* Camera comm ok (same as LEP_OK) */

		LEP_ERROR                         = -1,    /* Camera general error */
		LEP_NOT_READY                     = -2,    /* Camera not ready error */
		LEP_RANGE_ERROR                   = -3,    /* Camera range error */
		LEP_CHECKSUM_ERROR                = -4,    /* Camera checksum error */
		LEP_BAD_ARG_POINTER_ERROR         = -5,    /* Camera Bad argument  error */
		LEP_DATA_SIZE_ERROR               = -6,    /* Camera byte count error */
		LEP_UNDEFINED_FUNCTION_ERROR      = -7,    /* Camera undefined function error */
		LEP_FUNCTION_NOT_SUPPORTED        = -8,    /* Camera function not yet supported error */

		/* OTP access errors */
		LEP_OTP_WRITE_ERROR               = -15,   /*!< Camera OTP write error */
		LEP_OTP_READ_ERROR				    = -16,   /* double bit error detected (uncorrectible) */

		LEP_OTP_NOT_PROGRAMMED_ERROR      = -18,   /* Flag read as non-zero */

		/* I2C Errors */
		LEP_ERROR_I2C_BUS_NOT_READY       = -20,   /* I2C Bus Error - Bus Not Avaialble */
		LEP_ERROR_I2C_BUFFER_OVERFLOW     = -22,   /* I2C Bus Error - Buffer Overflow */
		LEP_ERROR_I2C_ARBITRATION_LOST    = -23,   /* I2C Bus Error - Bus Arbitration Lost */
		LEP_ERROR_I2C_BUS_ERROR           = -24,   /* I2C Bus Error - General Bus Error */
		LEP_ERROR_I2C_NACK_RECEIVED       = -25,   /* I2C Bus Error - NACK Received */
		LEP_ERROR_I2C_FAIL                = -26,   /* I2C Bus Error - General Failure */
		
		/* Processing Errors */
		LEP_DIV_ZERO_ERROR                = -80,   /* Attempted div by zero */

		/* Comm Errors */
		LEP_COMM_PORT_NOT_OPEN            = -101,  /* Comm port not open */
		LEP_COMM_INVALID_PORT_ERROR       = -102,  /* Comm port no such port error */
		LEP_COMM_RANGE_ERROR              = -103,  /* Comm port range error */
		LEP_ERROR_CREATING_COMM           = -104,  /* Error creating comm */
		LEP_ERROR_STARTING_COMM           = -105,  /* Error starting comm */
		LEP_ERROR_CLOSING_COMM            = -106,  /* Error closing comm */
		LEP_COMM_CHECKSUM_ERROR           = -107,  /* Comm checksum error */
		LEP_COMM_NO_DEV                   = -108,  /* No comm device */
		LEP_TIMEOUT_ERROR                 = -109,  /* Comm timeout error */
		LEP_COMM_ERROR_WRITING_COMM       = -110,  /* Error writing comm */
		LEP_COMM_ERROR_READING_COMM       = -111,  /* Error reading comm */
		LEP_COMM_COUNT_ERROR              = -112,  /* Comm byte count error */

		/* Other Errors */
		LEP_OPERATION_CANCELED            = -126,  /* Camera operation canceled */
		LEP_UNDEFINED_ERROR_CODE          = -127   /* Undefined error */

	} LEP_RESULT;

	uint16_t flir_setup (void);
	//void generatePalette(uint16_t*);
	uint16_t read16(uint16_t , uint16_t * , uint8_t );
	uint16_t write16(uint16_t , uint16_t * , uint8_t );
	LEP_RESULT LEP_I2C_GetAttribute(uint16_t , uint16_t* , uint16_t );
	LEP_RESULT LEP_I2C_SetAttribute(uint16_t , uint16_t* , uint16_t );
	LEP_RESULT LEP_I2C_RunCommand(uint16_t );

	#endif /* FLIR_H_ */