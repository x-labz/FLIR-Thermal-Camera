#include <asf.h>
#include "flir.h"
#include "LEPTON_I2C_Reg.h"
#include "LEPTON_AGC.h"
#include "LEPTON_SYS.h"
#include "i2c.h"


#define FLIR_LOG_SIZE 32

//uint16_t flir_log[FLIR_LOG_SIZE] ;

uint16_t flir_setup (void) {
	uint16_t success = 0, statusReads = 0, result, statusReg;
	
	//for (uint16_t i=0; i!=FLIR_LOG_SIZE; i++ ) { flir_log[i] = 0xAA; }
	
	do
	{	
		statusReads++;
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
	} while ( statusReg & 0x0006 != 0x0006 && statusReads < 100 );
	//flir_log[0] = statusReads;
	//flir_log[1] = result ;
	//flir_log[2] = statusReg;
	result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_AUX_TEMPERATURE_KELVIN, ( uint16_t* ) &result, 1 );
	result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_FPA_TEMPERATURE_KELVIN, ( uint16_t* ) &result, 1 );
	
	//uint64_t serial ;
	//LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_FLIR_SERIAL_NUMBER, ( uint16_t* ) &serial, 4 );
		
	//result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_CAM_STATUS, ( uint16_t* ) &flir_log[8], 4 );
	
	uint32_t agcEnableState = (uint32_t)0x00000001 ; //(uint32_t)0x00000001 ;
	result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_AGC_ENABLE_STATE, ( uint16_t* ) &agcEnableState, 2 );
	//result = LEP_I2C_GetAttribute( ( uint16_t )LEP_CID_AGC_ENABLE_STATE, ( uint16_t* ) &flir_log[14], 2 );	
	result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_AGC_CALC_ENABLE_STATE, ( uint16_t* ) &agcEnableState, 2 );	
	
	agcEnableState = (uint32_t)0x00000001 ;
	result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_SYS_TELEMETRY_ENABLE_STATE, ( uint16_t* ) &agcEnableState, 2 );
	uint32_t telemetryLoc = (uint32_t)0x00000000 ;
	result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_SYS_TELEMETRY_LOCATION, ( uint16_t* ) &telemetryLoc, 2 );
	
	//result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_TELEMETRY_ENABLE_STATE, ( uint16_t* ) &flir_log[20], 2 );
	//result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_TELEMETRY_LOCATION, ( uint16_t* ) &flir_log[23], 2 );
	
	uint32_t FCC_mode[10] ;
	result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ, ( uint16_t* ) FCC_mode, 20 );
	FCC_mode[0] = (uint32_t)0x00000000;
	result = LEP_I2C_SetAttribute( ( uint16_t)LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ, ( uint16_t* ) FCC_mode, 20 );
	result = 55 ;
	
	static uint16_t ROI[4];
	result = LEP_I2C_GetAttribute( ( uint16_t)LEP_CID_AGC_ROI, ( uint16_t* ) ROI, 4 );
	// -		ROI	[4]	uint16_t [4]
	//0	0	uint16_t
	//1	0	uint16_t
	//2	79	uint16_t
	//3	59	uint16_t
	
	do
	{
		statusReads++;
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
	} while ( statusReg & 0x0006 != 0x0006 && statusReads < 100 );
	result = statusReads;
	result = statusReg;	
}

//void generatePalette(uint16_t* palette_p) {
	//for (uint16_t i=0;i!=256; i++) {
		//*(palette_p+i) = color565(0,i>>2,0) ;
	//}
//}

uint16_t read16(uint16_t reg, uint16_t * result, uint8_t len) {
	uint8_t write_buffer[16] ;
	uint8_t read_buffer[32] ;
	
	write_buffer[0] = reg >> 8;
	write_buffer[1] = reg & 0xff;
	i2c_packet.data = write_buffer ;
	i2c_packet.data_length = 2 ;
	uint8_t r1 = i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &i2c_packet) ;
	
	i2c_packet.data = read_buffer;
	i2c_packet.data_length = 2 * len;
	uint8_t r2 = i2c_master_read_packet_wait(&i2c_master_instance, &i2c_packet) ;
	
	for (int i=0; i!=len; i++) {
		*(result+i) = Swap16( *(uint16_t*)(read_buffer+i*2) );
	}
	//*result = ((uint16_t)read_buffer[0]<<8) | (uint16_t)read_buffer[1] ;
	
	return (r1 << 8) | r2 ;
}

uint16_t write16(uint16_t reg, uint16_t * data, uint8_t volatile len) {
	uint8_t write_buffer[32] ;
	
	write_buffer[0] = reg >> 8;
	write_buffer[1] = reg & 0xff;
	
	i2c_packet.data = write_buffer ;
	//i2c_packet.data_length = 2 ;
	//uint8_t r1 = i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &i2c_packet) ;
	
	for (int i=0; i!=len; i++) {
		//*(result+i) = Swap16( read_buffer[i] );
		*((uint16_t*)(write_buffer+2+i*2)) = Swap16(*(data+i)) ;
	}
	
	//i2c_packet.data = write_buffer ;
	i2c_packet.data_length = 2 + 2*len ;
	uint8_t r1 = i2c_master_write_packet_wait(&i2c_master_instance, &i2c_packet) ;

	return  r1 ;
}

LEP_RESULT LEP_I2C_GetAttribute(uint16_t commandID, uint16_t* attributePtr, uint16_t attributeWordLength)
{
	LEP_RESULT result;
	uint16_t statusReg;
	int16_t statusCode;
	uint32_t done;
	uint16_t crcExpected, crcActual;

	/* Implement the Lepton TWI READ Protocol
	*/
	/* First wait until the Camera is ready to receive a new
	** command by polling the STATUS REGISTER BUSY Bit until it
	** reports NOT BUSY.
	*/

	do
	{
		/* Read the Status REGISTER and peek at the BUSY Bit
		*/
		//result = LEP_I2C_MasterReadData( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_STATUS_REG,
		//&statusReg,
		//1 );
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;

		if(result != LEP_OK)
		{
			return(result);
		}
		done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

	}while( !done );

	/* Set the Lepton's DATA LENGTH REGISTER first to inform the
	** Lepton Camera how many 16-bit DATA words we want to read.
	*/
	//result = LEP_I2C_MasterWriteData( portDescPtr->portID,
	//portDescPtr->deviceAddress,
	//LEP_I2C_DATA_LENGTH_REG,
	//&attributeWordLength,
	//1);
	result = write16(LEP_I2C_DATA_LENGTH_REG,&attributeWordLength,1) ;
	
	if(result != LEP_OK)
	{
		return(result);
	}
	/* Now issue the GET Attribute Command
	*/
	//result = LEP_I2C_MasterWriteData( portDescPtr->portID,
	//portDescPtr->deviceAddress,
	//LEP_I2C_COMMAND_REG,
	//&commandID,
	//1);
	result = write16(LEP_I2C_COMMAND_REG,&commandID,1) ;

	if(result != LEP_OK)
	{
		return(result);
	}

	/* Now wait until the Camera has completed this command by
	** polling the statusReg REGISTER BUSY Bit until it reports NOT
	** BUSY.
	*/
	do
	{
		/* Read the statusReg REGISTER and peek at the BUSY Bit
		*/
		//result = LEP_I2C_MasterReadData( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_STATUS_REG,
		//&statusReg,
		//1 );
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;

		if(result != LEP_OK)
		{
			return(result);
		}
		done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

	}while( !done );
	

	/* Check statusReg word for Errors?
	*/
	statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
	if(statusCode)
	{
		return((LEP_RESULT)statusCode);
	}

	/* If NO Errors then READ the DATA from the DATA REGISTER(s)
	*/
	if( attributeWordLength <= 16 )
	{
		/* Read from the DATA Registers - always start from DATA 0
		** Little Endian
		*/
		//result = LEP_I2C_MasterReadData(portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_DATA_0_REG,
		//attributePtr,
		//attributeWordLength );
		result = read16(LEP_I2C_DATA_0_REG, attributePtr, attributeWordLength) ;
	}
	else if( attributeWordLength <= 1024 )
	{
		/* Read from the DATA Block Buffer
		*/
		//result = LEP_I2C_MasterReadData(portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_DATA_BUFFER_0,
		//attributePtr,
		//attributeWordLength );
		result = read16(LEP_I2C_DATA_BUFFER_0, attributePtr, attributeWordLength) ;
	}
	if(result == LEP_OK && attributeWordLength > 0)
	{
		/* Check CRC */
		//result = LEP_I2C_MasterReadData( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_DATA_CRC_REG,
		//&crcExpected,
		//1);
		//crcActual = (LEP_UINT16)CalcCRC16Words(attributeWordLength, (short*)attributePtr);
		//
		///* Check for 0 in the register in case the camera does not support CRC check
		//*/
		//if(crcExpected != 0 && crcExpected != crcActual)
		//{
		//return(LEP_CHECKSUM_ERROR);
		//}
		
	}
	return(result);
}

LEP_RESULT LEP_I2C_SetAttribute(uint16_t commandID, uint16_t* attributePtr, uint16_t attributeWordLength)
{
	LEP_RESULT result;
	uint16_t statusReg;
	int16_t statusCode;
	uint32_t done;
	uint16_t timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

	/* Implement the Lepton TWI WRITE Protocol
	*/
	/* First wait until the Camera is ready to receive a new
	** command by polling the STATUS REGISTER BUSY Bit until it
	** reports NOT BUSY.
	*/
	do
	{
		/* Read the Status REGISTER and peek at the BUSY Bit
		*/
		//result = LEP_I2C_MasterReadData( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_STATUS_REG,
		//&statusReg,
		//1 );
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
		
		if(result != LEP_OK)
		{
			return(result);
		}
		done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
		/* Add timeout check */
		if( timeoutCount-- == 0 )
		{
			/* Timed out waiting for command busy to go away
			*/
			return(LEP_TIMEOUT_ERROR);

		}
	} while( !done );

	if( result == LEP_OK )
	{
		/* Now WRITE the DATA to the DATA REGISTER(s)
		*/
		if( attributeWordLength <= 16 )
		{
			/* WRITE to the DATA Registers - always start from DATA 0
			*/
			//result = LEP_I2C_MasterWriteData(portDescPtr->portID,
			//portDescPtr->deviceAddress,
			//LEP_I2C_DATA_0_REG,
			//attributePtr,
			//attributeWordLength );
			result = write16(LEP_I2C_DATA_0_REG,attributePtr,attributeWordLength) ;
		}
		else if( attributeWordLength <= 1024 )
		{
			/* WRITE to the DATA Block Buffer
			*/
			//result = LEP_I2C_MasterWriteData(portDescPtr->portID,
			//portDescPtr->deviceAddress,
			//LEP_I2C_DATA_BUFFER_0,
			//attributePtr,
			//attributeWordLength );
			result = write16(LEP_I2C_DATA_BUFFER_0,attributePtr,attributeWordLength) ;
		}
		else
		result = LEP_RANGE_ERROR;
	}

	if( result == LEP_OK )
	{
		/* Set the Lepton's DATA LENGTH REGISTER first to inform the
		** Lepton Camera how many 16-bit DATA words we want to read.
		*/
		//result = LEP_I2C_MasterWriteData( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_DATA_LENGTH_REG,
		//&attributeWordLength,
		//1);
		result = write16(LEP_I2C_DATA_LENGTH_REG,&attributeWordLength,1) ;

		if( result == LEP_OK )
		{
			/* Now issue the SET Attribute Command
			*/
			//result = LEP_I2C_MasterWriteData( portDescPtr->portID,
			//portDescPtr->deviceAddress,
			//LEP_I2C_COMMAND_REG,
			//&commandID,
			//1);
			commandID |= LEP_SET_TYPE;
			result = write16(LEP_I2C_COMMAND_REG,&commandID,1) ;
			
			if( result == LEP_OK )
			{
				/* Now wait until the Camera has completed this command by
				** polling the statusReg REGISTER BUSY Bit until it reports NOT
				** BUSY.
				*/
				do
				{
					/* Read the statusReg REGISTER and peek at the BUSY Bit
					*/
					//result = LEP_I2C_MasterReadData( portDescPtr->portID,
					//portDescPtr->deviceAddress,
					//LEP_I2C_STATUS_REG,
					//&statusReg,
					//1 );
					result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
					if(result != LEP_OK)
					{
						return(result);
					}
					done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

				}while( !done );

				/* Check statusReg word for Errors?
				*/
				statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
				if(statusCode)
				{
					return((LEP_RESULT)statusCode);
				}

			}
		}
	}

	/* Check statusReg word for Errors?
	*/

	return(result);
}


LEP_RESULT LEP_I2C_RunCommand(uint16_t commandID)
{
	LEP_RESULT result;
	uint16_t statusReg;
	int16_t statusCode;
	uint32_t done;
	uint16_t timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

	/* Implement the Lepton TWI WRITE Protocol
	*/
	/* First wait until the Camera is ready to receive a new
	** command by polling the STATUS REGISTER BUSY Bit until it
	** reports NOT BUSY.
	*/
	do
	{
		/* Read the Status REGISTER and peek at the BUSY Bit
		*/
		//result = LEP_I2C_MasterReadRegister( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_STATUS_REG,
		//&statusReg);
		result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
		
		if(result != LEP_OK)
		{
			return(result);
		}
		done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
		/* Add timeout check */
		if( timeoutCount-- == 0 )
		{
			/* Timed out waiting for command busy to go away
			*/

		}
	}while( !done );

	if( result == LEP_OK )
	{
		/* Set the Lepton's DATA LENGTH REGISTER first to inform the
		** Lepton Camera no 16-bit DATA words being transferred.
		*/
		//result = LEP_I2C_MasterWriteRegister( portDescPtr->portID,
		//portDescPtr->deviceAddress,
		//LEP_I2C_DATA_LENGTH_REG,
		//(LEP_UINT16)0);
		uint16_t arg = 0;
		result = write16(LEP_I2C_DATA_LENGTH_REG,&arg,1) ;
		
		if( result == LEP_OK )
		{
			/* Now issue the Run Command
			*/
			//result = LEP_I2C_MasterWriteRegister( portDescPtr->portID,
			//portDescPtr->deviceAddress,
			//LEP_I2C_COMMAND_REG,
			//commandID);
			commandID |= LEP_RUN_TYPE ;
			result = write16(LEP_I2C_COMMAND_REG,&commandID,1) ;
			if( result == LEP_OK )
			{
				/* Now wait until the Camera has completed this command by
				** polling the statusReg REGISTER BUSY Bit until it reports NOT
				** BUSY.
				*/
				do
				{
					/* Read the statusReg REGISTER and peek at the BUSY Bit
					*/
					//result = LEP_I2C_MasterReadRegister( portDescPtr->portID,
					//portDescPtr->deviceAddress,
					//LEP_I2C_STATUS_REG,
					//&statusReg);
					result = read16(LEP_I2C_STATUS_REG, &statusReg, 1) ;
					
					if(result != LEP_OK)
					{
						return(result);
					}
					done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
					/* Timeout? */

				}while( !done );

				statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
				if(statusCode)
				{
					return((LEP_RESULT)statusCode);
				}
			}
		}
	}

	/* Check statusReg word for Errors?
	*/

	
	return(result);
}