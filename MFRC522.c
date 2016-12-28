#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"
#include "uart.h"
#include "interrupt.h"

// Common interface includes
#include "uart_if.h"
#include "pinmux.h"

//aditional includes for project integration //Shankar
#include "simplelink.h"
#include "gpio_if.h"
#include "MFRC522.h"
#include "lcd.h"
#include "timer.h"
#include "timer_if.h"

#define APPLICATION_VERSION     "1.1.1"
//*****************************************************************************
//
// Application Master/Slave mode selector macro
//
// MASTER_MODE = 1 : Application in master mode
// MASTER_MODE = 0 : Application in slave mode
//
//*****************************************************************************

//#define SPI_IF_BIT_RATE  100000
#define SPI_IF_BIT_RATE  160000
#define TR_BUFF_SIZE     512

#define OUTPUT 				1
#define INPUT 				0
#define True 				1
#define False 				0
#define	HIGH				1
#define LOW					0

#define _resetPowerDownPin 	9
//#define DEBUG
#define	DELAYRW 0xFF

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned char g_ucTxBuff[TR_BUFF_SIZE];
static unsigned char g_ucRxBuff[TR_BUFF_SIZE];
volatile int gameStartFlag = 0;
static volatile unsigned long g_ulRefBase;  // ref for timer for ticket dispense pulse added by brij
static volatile unsigned long g_ulBase;   // ref for timer for coin mech pulse


short Coinflag = 0;      // flag for coin mechanism pulse

//static int readcounter = 0;
//static unsigned long ulPort = 0;	//Set the port value here. For now we can leave this constant
unsigned char nuidPICC[4];

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************
//*****************************************************************************
//                 GLOBAL VARIABLES -- MFRC start
//*****************************************************************************
enum PCD_Register {
		// Page 0: Command and status
		//						  0x00			// reserved for future use
		CommandReg				= 0x01 << 1,	// starts and stops command execution
		ComIEnReg				= 0x02 << 1,	// enable and disable interrupt request control bits
		DivIEnReg				= 0x03 << 1,	// enable and disable interrupt request control bits
		ComIrqReg				= 0x04 << 1,	// interrupt request bits
		DivIrqReg				= 0x05 << 1,	// interrupt request bits
		ErrorReg				= 0x06 << 1,	// error bits showing the error status of the last command executed 
		Status1Reg				= 0x07 << 1,	// communication status bits
		Status2Reg				= 0x08 << 1,	// receiver and transmitter status bits
		FIFODataReg				= 0x09 << 1,	// input and output of 64 byte FIFO buffer
		FIFOLevelReg			= 0x0A << 1,	// number of bytes stored in the FIFO buffer
		WaterLevelReg			= 0x0B << 1,	// level for FIFO underflow and overflow warning
		ControlReg				= 0x0C << 1,	// miscellaneous control registers
		BitFramingReg			= 0x0D << 1,	// adjustments for bit-oriented frames
		CollReg					= 0x0E << 1,	// bit position of the first bit-collision detected on the RF interface
		//						  0x0F			// reserved for future use
		
		// Page 1: Command
		// 						  0x10			// reserved for future use
		ModeReg					= 0x11 << 1,	// defines general modes for transmitting and receiving 
		TxModeReg				= 0x12 << 1,	// defines transmission data rate and framing
		RxModeReg				= 0x13 << 1,	// defines reception data rate and framing
		TxControlReg			= 0x14 << 1,	// controls the logical behavior of the antenna driver pins TX1 and TX2
		TxASKReg				= 0x15 << 1,	// controls the setting of the transmission modulation
		TxSelReg				= 0x16 << 1,	// selects the internal sources for the antenna driver
		RxSelReg				= 0x17 << 1,	// selects internal receiver settings
		RxThresholdReg			= 0x18 << 1,	// selects thresholds for the bit decoder
		DemodReg				= 0x19 << 1,	// defines demodulator settings
		// 						  0x1A			// reserved for future use
		// 						  0x1B			// reserved for future use
		MfTxReg					= 0x1C << 1,	// controls some MIFARE communication transmit parameters
		MfRxReg					= 0x1D << 1,	// controls some MIFARE communication receive parameters
		// 						  0x1E			// reserved for future use
		SerialSpeedReg			= 0x1F << 1,	// selects the speed of the serial UART interface
		
		// Page 2: Configuration
		// 						  0x20			// reserved for future use
		CRCResultRegH			= 0x21 << 1,	// shows the MSB and LSB values of the CRC calculation
		CRCResultRegL			= 0x22 << 1,
		// 						  0x23			// reserved for future use
		ModWidthReg				= 0x24 << 1,	// controls the ModWidth setting?
		// 						  0x25			// reserved for future use
		RFCfgReg				= 0x26 << 1,	// configures the receiver gain
		GsNReg					= 0x27 << 1,	// selects the conductance of the antenna driver pins TX1 and TX2 for modulation 
		CWGsPReg				= 0x28 << 1,	// defines the conductance of the p-driver output during periods of no modulation
		ModGsPReg				= 0x29 << 1,	// defines the conductance of the p-driver output during periods of modulation
		TModeReg				= 0x2A << 1,	// defines settings for the internal timer
		TPrescalerReg			= 0x2B << 1,	// the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
		TReloadRegH				= 0x2C << 1,	// defines the 16-bit timer reload value
		TReloadRegL				= 0x2D << 1,
		TCounterValueRegH		= 0x2E << 1,	// shows the 16-bit timer value
		TCounterValueRegL		= 0x2F << 1,
		
		// Page 3: Test Registers
		// 						  0x30			// reserved for future use
		TestSel1Reg				= 0x31 << 1,	// general test signal configuration
		TestSel2Reg				= 0x32 << 1,	// general test signal configuration
		TestPinEnReg			= 0x33 << 1,	// enables pin output driver on pins D1 to D7
		TestPinValueReg			= 0x34 << 1,	// defines the values for D1 to D7 when it is used as an I/O bus
		TestBusReg				= 0x35 << 1,	// shows the status of the internal test bus
		AutoTestReg				= 0x36 << 1,	// controls the digital self-test
		VersionReg				= 0x37 << 1,	// shows the software version
		AnalogTestReg			= 0x38 << 1,	// controls the pins AUX1 and AUX2
		TestDAC1Reg				= 0x39 << 1,	// defines the test value for TestDAC1
		TestDAC2Reg				= 0x3A << 1,	// defines the test value for TestDAC2
		TestADCReg				= 0x3B << 1		// shows the value of ADC I and Q channels
		// 						  0x3C			// reserved for production tests
		// 						  0x3D			// reserved for production tests
		// 						  0x3E			// reserved for production tests
		// 						  0x3F			// reserved for production tests
	};
	
	// MFRC522 commands. Described in chapter 10 of the datasheet.
	enum PCD_Command {
		PCD_Idle				= 0x00,		// no action, cancels current command execution
		PCD_Mem					= 0x01,		// stores 25 bytes into the internal buffer
		PCD_GenerateRandomID	= 0x02,		// generates a 10-byte random ID number
		PCD_CalcCRC				= 0x03,		// activates the CRC coprocessor or performs a self-test
		PCD_Transmit			= 0x04,		// transmits data from the FIFO buffer
		PCD_NoCmdChange			= 0x07,		// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
		PCD_Receive				= 0x08,		// activates the receiver circuits
		PCD_Transceive 			= 0x0C,		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
		PCD_MFAuthent 			= 0x0E,		// performs the MIFARE standard authentication as a reader
		PCD_SoftReset			= 0x0F		// resets the MFRC522
	};
	
	// MFRC522 RxGain[2:0] masks, defines the receiver's signal voltage gain factor (on the PCD).
	// Described in 9.3.3.6 / table 98 of the datasheet at http://www.nxp.com/documents/data_sheet/MFRC522.pdf
	enum PCD_RxGain {
		RxGain_18dB				= 0x00 << 4,	// 000b - 18 dB, minimum
		RxGain_23dB				= 0x01 << 4,	// 001b - 23 dB
		RxGain_18dB_2			= 0x02 << 4,	// 010b - 18 dB, it seems 010b is a duplicate for 000b
		RxGain_23dB_2			= 0x03 << 4,	// 011b - 23 dB, it seems 011b is a duplicate for 001b
		RxGain_33dB				= 0x04 << 4,	// 100b - 33 dB, average, and typical default
		RxGain_38dB				= 0x05 << 4,	// 101b - 38 dB
		RxGain_43dB				= 0x06 << 4,	// 110b - 43 dB
		RxGain_48dB				= 0x07 << 4,	// 111b - 48 dB, maximum
		RxGain_min				= 0x00 << 4,	// 000b - 18 dB, minimum, convenience for RxGain_18dB
		RxGain_avg				= 0x04 << 4,	// 100b - 33 dB, average, convenience for RxGain_33dB
		RxGain_max				= 0x07 << 4		// 111b - 48 dB, maximum, convenience for RxGain_48dB
	};
	
	// Commands sent to the PICC.
	enum PICC_Command {
		// The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
		PICC_CMD_REQA			= 0x26,		// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
		PICC_CMD_WUPA			= 0x52,		// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
		PICC_CMD_CT				= 0x88,		// Cascade Tag. Not really a command, but used during anti collision.
		PICC_CMD_SEL_CL1		= 0x93,		// Anti collision/Select, Cascade Level 1
		PICC_CMD_SEL_CL2		= 0x95,		// Anti collision/Select, Cascade Level 2
		PICC_CMD_SEL_CL3		= 0x97,		// Anti collision/Select, Cascade Level 3
		PICC_CMD_HLTA			= 0x50,		// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
		// The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
		// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
		// The read/write commands can also be used for MIFARE Ultralight.
		PICC_CMD_MF_AUTH_KEY_A	= 0x60,		// Perform authentication with Key A
		PICC_CMD_MF_AUTH_KEY_B	= 0x61,		// Perform authentication with Key B
		PICC_CMD_MF_READ		= 0x30,		// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
		PICC_CMD_MF_WRITE		= 0xA0,		// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
		PICC_CMD_MF_DECREMENT	= 0xC0,		// Decrements the contents of a block and stores the result in the internal data register.
		PICC_CMD_MF_INCREMENT	= 0xC1,		// Increments the contents of a block and stores the result in the internal data register.
		PICC_CMD_MF_RESTORE		= 0xC2,		// Reads the contents of a block into the internal data register.
		PICC_CMD_MF_TRANSFER	= 0xB0,		// Writes the contents of the internal data register to a block.
		// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
		// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
		PICC_CMD_UL_WRITE		= 0xA2		// Writes one 4 byte page to the PICC.
	};
	
	// MIFARE constants that does not fit anywhere else
	enum MIFARE_Misc {
		MF_ACK					= 0xA,		// The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
		MF_KEY_SIZE				= 6			// A Mifare Crypto1 key is 6 bytes.
	};
	
	// PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
	// last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
	enum PICC_Type  {
		PICC_TYPE_UNKNOWN		,
		PICC_TYPE_ISO_14443_4	,	// PICC compliant with ISO/IEC 14443-4 
		PICC_TYPE_ISO_18092		, 	// PICC compliant with ISO/IEC 18092 (NFC)
		PICC_TYPE_MIFARE_MINI	,	// MIFARE Classic protocol, 320 bytes
		PICC_TYPE_MIFARE_1K		,	// MIFARE Classic protocol, 1KB
		PICC_TYPE_MIFARE_4K		,	// MIFARE Classic protocol, 4KB
		PICC_TYPE_MIFARE_UL		,	// MIFARE Ultralight or Ultralight C
		PICC_TYPE_MIFARE_PLUS	,	// MIFARE Plus
		PICC_TYPE_TNP3XXX		,	// Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
		PICC_TYPE_NOT_COMPLETE	= 0xff	// SAK indicates UID is not complete.
	};
	
	// Return codes from the functions in this class. Remember to update GetStatusCodeName() if you add more.
	// last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
	enum StatusCode  {
		STATUS_OK				,	// Success
		STATUS_ERROR			,	// Error in communication
		STATUS_COLLISION		,	// Collission detected
		STATUS_TIMEOUT			,	// Timeout in communication.
		STATUS_NO_ROOM			,	// A buffer is not big enough.
		STATUS_INTERNAL_ERROR	,	// Internal error in the code. Should not happen ;-)
		STATUS_INVALID			,	// Invalid argument.
		STATUS_CRC_WRONG		,	// The CRC_A does not match
		STATUS_MIFARE_NACK		= 0xff	// A MIFARE PICC responded with NAK.
	};
	
	// A struct used for passing the UID of a PICC.
	struct UIDStruct{
		unsigned char		size;			// Number of bytes in the UID. 4, 7 or 10.
		unsigned char		uidByte[10];
		unsigned char		sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
	};
	
	// A struct used for passing a MIFARE Crypto1 key
	typedef struct {
		unsigned char		keyByte[MF_KEY_SIZE];
	} MIFARE_Key;
	
	// Member variables
	Uid uid;								// Used by PICC_ReadCardSerial().
	
//*****************************************************************************
//                 GLOBAL VARIABLES -- MFRC END
//*****************************************************************************

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
int MFRC522loop()
{
	unsigned char i;
	//unsigned int ii;
	//UART Functions are as below use these and replace Arduino UART
	//Report("\n\rSend      %s",g_ucTxBuff);

/*************************************************************************************************
 * Only for Testing Purpose, remove this junk once you are done testing //Shankar
 ************************************************************************************************/
	//for(ii = 0; ii < 0x07FFFFFF; ii++)
	//{
	//	i = i;
	//}
	//CallHttpPost();
/*************************************************************************************************
 * Only for Testing Purpose, remove this junk once you are done testing //Shankar
 ************************************************************************************************/

	  // Look for new cards
  if ( !PICC_IsNewCardPresent())
  {
	  return (-1);
  }

  // Verify if the NUID has been readed
  if ( !PICC_ReadCardSerial())
    return (-1);

  //Serial.print(F("PICC type: "));
  enum PICC_Type piccType = PICC_GetType(uid.sak);
  //Serial.println(PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != PICC_TYPE_MIFARE_MINI &&  
    piccType != PICC_TYPE_MIFARE_1K &&
    piccType != PICC_TYPE_MIFARE_4K) 
	{
		Report("Your tag is not of type MIFARE Classic.\n");
		return (-1);
	}
  else
  {
	  LCDClear();
	  LCDSelectLine(0);
	  print("Success UID is:");
	  LCDSelectLine(1);
		Report("A card has been detected.UID is: ");
		// Store NUID into nuidPICC array
		for (i = 0; i < 4; i++)
		{
			Report("%x ", uid.uidByte[i]);
			number((unsigned int)uid.uidByte[i]);
		}
  }

  // Halt PICC
	PICC_HaltA();

  // Stop encryption on PCD
	PCD_StopCrypto1();

	return (1);
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
/*******************************************************************************************************************************************
The MFRC32 code starts here   --Shankar
********************************************************************************************************************************************/
void pinMode(unsigned char Pin, unsigned char direction)
{
	//GPIODirModeSet(ulPort, Pin, direction);		//Nothing much to do here since this is sone in pinmux
	//GPIO_IF_LedConfigure(LED1);

}
unsigned char digitalRead(unsigned char ipPin)
{
	//return(GPIOPinRead(ulPort, ipPin));
	//return(GPIO_IF_LedStatus(MCU_RED_LED_GPIO));
	return(MAP_GPIOPinRead(GPIOA3_BASE, GPIO_PIN_4));
}
void digitalWrite(unsigned char opPin, unsigned char Level)
{
	if(Level)
		//GPIO_IF_LedOn(MCU_RED_LED_GPIO);
		MAP_GPIOPinWrite(GPIOA3_BASE, GPIO_PIN_4, 1);
	else
		MAP_GPIOPinWrite(GPIOA3_BASE, GPIO_PIN_4, 0);
		//GPIO_IF_LedOff(MCU_RED_LED_GPIO);

}

void delay(unsigned int delaytime)
{
}

void MFRC522_init()   //unsigned char IresetPowerDownPin  < Arduino pin connected to MFRC522's reset and power down input (Pin 6, NRSTPD, active low)
{
	Message("Initialization.....\n");
	MAP_SPIReset(GSPI_BASE);
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8));
    MAP_SPIEnable(GSPI_BASE);

    PCD_Init();
    lcd_init();
} 

void PCD_WriteRegister_alternate(  unsigned char reg,   ///< The register to write to. One of the PCD_Register enums.
                  unsigned char value    ///< The value to write.
                ) 
{
	unsigned char memcpy_Buff;
	memcpy_Buff = (reg & 0x7E);
	memcpy(g_ucTxBuff,&memcpy_Buff,sizeof(reg)); // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
	MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,1, SPI_CS_ENABLE|SPI_CS_DISABLE);
	memcpy_Buff = (value);
	memcpy(g_ucTxBuff,&memcpy_Buff,sizeof(value));
	MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,1, SPI_CS_ENABLE|SPI_CS_DISABLE);
} 

void PCD_WriteRegister(  unsigned char reg,   ///< The register to write to. One of the PCD_Register enums.
                  unsigned char value    ///< The value to write.
                ) 
{
    unsigned long ulUserData;
    unsigned long ulDummy;

#ifdef DELAYRW
    int i;
    for(i = 0; i < DELAYRW; i++);
#endif
	MAP_SPICSEnable(GSPI_BASE);
	ulUserData = reg & 0x7E;
	MAP_SPIDataPut(GSPI_BASE,ulUserData);
    MAP_SPIDataGet(GSPI_BASE,&ulDummy);
	ulUserData = value;
	MAP_SPIDataPut(GSPI_BASE,ulUserData);
    MAP_SPIDataGet(GSPI_BASE,&ulDummy);
	MAP_SPICSDisable(GSPI_BASE);	
} 

void PCD_WriteBytes_alterenate(  unsigned char reg,   ///< The register to write to. One of the PCD_Register enums.
                  unsigned char count,   ///< The number of bytes to write to the register
                  unsigned char *values  ///< The values to write. Byte array.
                ) 
{
	unsigned char memcpy_Buff;
	memcpy_Buff = (reg & 0x7E);
	memcpy(g_ucTxBuff,&memcpy_Buff,sizeof(reg)); // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
	MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,1, SPI_CS_ENABLE|SPI_CS_DISABLE);     
	memcpy(g_ucTxBuff,(values),sizeof(values));
	MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,sizeof(values), SPI_CS_ENABLE|SPI_CS_DISABLE);	
} //

void PCD_WriteBytes(  unsigned char reg,   ///< The register to write to. One of the PCD_Register enums.
                  unsigned char count,   ///< The number of bytes to write to the register
                  unsigned char *values  ///< The values to write. Byte array.
                )
{
	  unsigned char index;
	  unsigned long ulUserData;
	  unsigned long ulDummy;

#ifdef DELAYRW
	  int i;
	  for(i = 0; i < DELAYRW; i++);
#endif
	  MAP_SPICSEnable(GSPI_BASE);
	  ulUserData = (reg & 0x7E);
	  MAP_SPIDataPut(GSPI_BASE,ulUserData);       // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
	  MAP_SPIDataGet(GSPI_BASE,&ulDummy);
	  for (index = 0; index < count; index++)
	  {
		ulUserData = values[index];
	    MAP_SPIDataPut(GSPI_BASE,ulUserData);
	    MAP_SPIDataGet(GSPI_BASE,&ulDummy);
#ifdef DELAYRW
		  {int i;
		  for(i = 0; i < DELAYRW; i++);}
#endif
	  }
	  MAP_SPICSDisable(GSPI_BASE);   // Release slave again Stop using the SPI bus
}

unsigned char PCD_ReadRegister( unsigned char reg)  ///< The register to read from. One of the PCD_Register enums.                 
{
    unsigned long ulUserData;
    unsigned long ulDummy;
	unsigned char value;

#ifdef DELAYRW
	int i;
	for(i = 0; i < DELAYRW; i++);
#endif
  	MAP_SPICSEnable(GSPI_BASE);
	ulUserData = (0x80 | (reg & 0x7E));
	MAP_SPIDataPut(GSPI_BASE,ulUserData);
    MAP_SPIDataGet(GSPI_BASE, &ulDummy);
    //value = (unsigned char)ulDummy;
    //Report("%d ", value);
    ulUserData = 0;
	MAP_SPIDataPut(GSPI_BASE,ulUserData);
    MAP_SPIDataGet(GSPI_BASE, &ulDummy);
    value = (unsigned char)ulDummy;
#ifdef DEBUG
    Report("%d ", value);
#endif
	MAP_SPICSDisable(GSPI_BASE);
	return value;
} 

void PCD_ReadBytes( unsigned char reg,   ///< The register to read from. One of the PCD_Register enums.
                unsigned char count,   ///< The number of bytes to read
                unsigned char *values, ///< Byte array to store the values in.
                unsigned char rxAlign  ///< Only bit positions rxAlign..7 in values[0] are updated.
                )
{
	unsigned char value;
	unsigned long ulDummy;

#ifdef DELAYRW
	int i;
	for(i = 0; i < DELAYRW; i++);
#endif
	if (count == 0) 
	{
		return;
	}
	//Serial.print(F("Reading "));  Serial.print(count); Serial.println(F(" bytes from register."));
	unsigned char address = 0x80 | (reg & 0x7E);   	// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
	unsigned char index = 0;             			// Index in values array.
	MAP_SPICSEnable(GSPI_BASE);
	count--;               						 	// One read is performed outside of the loop
	MAP_SPIDataPut(GSPI_BASE,address);          	// Tell MFRC522 which address we want to read
	MAP_SPIDataGet(GSPI_BASE,&ulDummy);
	while (index < count) 
	{
		unsigned char i;
		if (index == 0 && rxAlign) {    // Only update bit positions rxAlign..7 in values[0]
		  // Create bit mask for bit positions rxAlign..7
		  unsigned char mask = 0;
		  for (i = rxAlign; i <= 7; i++)
		  {
			mask |= (1 << i);
		  }
		  // Read value and tell that we want to read the same address again.
		  MAP_SPIDataPut(GSPI_BASE, address);
		  MAP_SPIDataGet(GSPI_BASE, &ulDummy);
		  value = (unsigned char)ulDummy;
		  // Apply mask to both current value of values[0] and the new data in value.
		  values[0] = (values[index] & ~mask) | (value & mask);
		}
		else 
		{ // Normal case
		  MAP_SPIDataPut(GSPI_BASE, address);
		  MAP_SPIDataGet(GSPI_BASE, &ulDummy);  // Read value and tell that we want to read the same address again.
		  values[index] = (unsigned char)ulDummy;
		}
		index++;
#ifdef DELAYRW
		{int i;
		for(i = 0; i < DELAYRW; i++);}
#endif
	}
	MAP_SPIDataPut(GSPI_BASE, 0);
	MAP_SPIDataGet(GSPI_BASE, &ulDummy);  // Read value and tell that we want to read the same address again.
	values[index] = (unsigned char)ulDummy;
	MAP_SPICSDisable(GSPI_BASE); // Stop using the SPI bus
}

void PCD_SetRegisterBitMask( unsigned char reg, unsigned char mask) ///< The register to update. One of the PCD_Register enums and The bits to set.>
{ 
	unsigned char tmp;
	tmp = PCD_ReadRegister(reg);
	PCD_WriteRegister(reg, tmp | mask);     // set bit mask
} 

void PCD_ClearRegisterBitMask( unsigned char reg, ///< The register to update. One of the PCD_Register enums.
                    unsigned char mask ///< The bits to clear.
                    )
{
	unsigned char tmp;
	tmp = PCD_ReadRegister(reg);
	PCD_WriteRegister(reg, tmp & (~mask));    // clear bit mask
	//Message("Am Inside  ");
}

void PCD_Reset()
{
	PCD_WriteRegister(CommandReg, PCD_SoftReset); // Issue the SoftReset command.
	// The datasheet does not mention how long the SoftRest command takes to complete.
	// But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
	// Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74μs. Let us be generous: 50ms.
	// Wait for the PowerDown bit in CommandReg to be cleared
	while (PCD_ReadRegister(CommandReg) & (1<<4))
	{
    // PCD still restarting - unlikely after waiting 50ms, but better safe than sorry.
	}
}

void PCD_Init() 
{
	// Set the resetPowerDownPin as digital output, do not reset or power down.
	pinMode(_resetPowerDownPin, OUTPUT);
  
	if (digitalRead(_resetPowerDownPin) == LOW) 
	{ //The MFRC522 chip is in power down mode.
		digitalWrite(_resetPowerDownPin, HIGH);   // Exit power down mode. This triggers a hard reset.
		// Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74μs. Let us be generous: 50ms.
		delay(50);
	}
	else 
	{ // Perform a soft reset
		PCD_Reset();
	}
  
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	PCD_WriteRegister(TModeReg, 0x80);      // TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	PCD_WriteRegister(TPrescalerReg, 0xA9);   // TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25μs.
	PCD_WriteRegister(TReloadRegH, 0x03);   // Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	PCD_WriteRegister(TReloadRegL, 0xE8);
	
	PCD_WriteRegister(TxASKReg, 0x40);    // Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	PCD_WriteRegister(ModeReg, 0x3D);   // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	PCD_AntennaOn();            // Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
}


enum StatusCode PCD_CalculateCRC(  unsigned char *data,   ///< In: Pointer to the data to transfer to the FIFO for CRC calculation.
                        unsigned char length,  ///< In: The number of bytes to transfer.
                        unsigned char *result  ///< Out: Pointer to result buffer. Result is written to result[0..1], low byte first.
           )
{
	PCD_WriteRegister(CommandReg, PCD_Idle);    // Stop any active command.
	PCD_WriteRegister(DivIrqReg, 0x04);       // Clear the CRCIRq interrupt request bit
	PCD_SetRegisterBitMask(FIFOLevelReg, 0x80);   // FlushBuffer = 1, FIFO initialization
	PCD_WriteBytes(FIFODataReg, length, data); // Write data to the FIFO
	PCD_WriteRegister(CommandReg, PCD_CalcCRC);    // Start the calculation

	// Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73μs.
	// TODO check/modify for other architectures than Arduino Uno 16bit
	unsigned short i = 5000;
	unsigned char n;
	while (1)
	{
		n = PCD_ReadRegister(DivIrqReg);  // DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
		if (n & 0x04) {           // CRCIRq bit set - calculation done
		  break;
		}
		if (--i == 0) {           // The emergency break. We will eventually terminate on this one after 89ms. Communication with the MFRC522 might be down.
		  return STATUS_TIMEOUT;
		}
	}
	PCD_WriteRegister(CommandReg, PCD_Idle);    // Stop calculating CRC for new content in the FIFO.

	// Transfer the result from the registers to the result buffer
	result[0] = PCD_ReadRegister(CRCResultRegL);
	result[1] = PCD_ReadRegister(CRCResultRegH);
	return STATUS_OK;
}

void PCD_StopCrypto1()
{
  // Clear MFCrypto1On bit
  PCD_ClearRegisterBitMask(Status2Reg, 0x08); // Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved MFCrypto1On ModemState[2:0]
} // End PCD_StopCrypto1()

enum StatusCode PICC_HaltA()
{
	enum StatusCode result;
	unsigned char buffer[4];

	// Build command buffer
	buffer[0] = PICC_CMD_HLTA;
	buffer[1] = 0;
	// Calculate CRC_A
	result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
	if (result != STATUS_OK)
	{
		return result;
	}
	// Send the command.
	// The standard says:
	//    If the PICC responds with any modulation during a period of 1 ms after the end of the frame containing the
	//    HLTA command, this response shall be interpreted as 'not acknowledge'.
	// We interpret that this way: Only STATUS_TIMEOUT is a success.
	result = PCD_TransceiveData(buffer, sizeof(buffer), NULL, 0, NULL, 0, 0);
	if (result == STATUS_TIMEOUT)
	{
		return STATUS_OK;
	}
	if (result == STATUS_OK)
	{ // That is ironically NOT ok in this case ;-)
		return STATUS_ERROR;
	}
	return result;
} // End PICC_HaltA()

void PCD_AntennaOn()
{
	unsigned char value = PCD_ReadRegister(TxControlReg);
	if ((value & 0x03) != 0x03)
	{
		PCD_WriteRegister(TxControlReg, value | 0x03);
	}
}


void PCD_AntennaOff() 
{
	PCD_ClearRegisterBitMask(TxControlReg, 0x03);
}



enum StatusCode PCD_CommunicateWithPICC( unsigned char command,   ///< The command to execute. One of the PCD_Command enums.
                            unsigned char waitIRq,   ///< The bits in the ComIrqReg register that signals successful completion of the command.
                            unsigned char *sendData,   ///< Pointer to the data to transfer to the FIFO.
                            unsigned char sendLen,   ///< Number of bytes to transfer to the FIFO.
                            unsigned char *backData,   ///< NULL or pointer to buffer if data should be read back after executing the command.
                            unsigned char *backLen,    ///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
                            unsigned char *validBits,  ///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits.
                            unsigned char rxAlign,   ///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
                            unsigned char checkCRC   ///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
                   ) 
{
	unsigned char n, _validBits;
	unsigned short int i;
  
	// Prepare values for BitFramingReg
	unsigned char txLastBits = validBits ? *validBits : 0;
	unsigned char bitFraming = (rxAlign << 4) + txLastBits;    // RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
  
	PCD_WriteRegister(CommandReg, PCD_Idle);      // Stop any active command.
	PCD_WriteRegister(ComIrqReg, 0x7F);         // Clear all seven interrupt request bits
	PCD_SetRegisterBitMask(FIFOLevelReg, 0x80);     // FlushBuffer = 1, FIFO initialization
	PCD_WriteBytes(FIFODataReg, sendLen, sendData);  // Write sendData to the FIFO
	PCD_WriteRegister(BitFramingReg, bitFraming);   // Bit adjustments
	PCD_WriteRegister(CommandReg, command);       // Execute the command
	if (command == PCD_Transceive) 
	{
		PCD_SetRegisterBitMask(BitFramingReg, 0x80);  // StartSend=1, transmission of data starts
	}	
  
	// Wait for the command to complete.
	// In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
	// Each iteration of the do-while-loop takes 17.86μs.
	// TODO check/modify for other architectures than Arduino Uno 16bit
	i = 2000;
	while (1) 
	{
		n = PCD_ReadRegister(ComIrqReg);  // ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
		if (n & waitIRq) {          // One of the interrupts that signal success has been set.
			break;
		}
		if (n & 0x01) {           // Timer interrupt - nothing received in 25ms
			return STATUS_TIMEOUT;
		}
		if (--i == 0) {           // The emergency break. If all other conditions fail we will eventually terminate on this one after 35.7ms. Communication with the MFRC522 might be down.
			return STATUS_TIMEOUT;
		}
	}	
  
	// Stop now if any errors except collisions were detected.
	unsigned char errorRegValue = PCD_ReadRegister(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
	if (errorRegValue & 0x13) 
	{  // BufferOvfl ParityErr ProtocolErr
		return STATUS_ERROR;
	} 

	// If the caller wants data back, get it from the MFRC522.
	if (backData && backLen) 
	{
		n = PCD_ReadRegister(FIFOLevelReg);     // Number of bytes in the FIFO
		if (n > *backLen) 
		{
			return STATUS_NO_ROOM;
		}
		*backLen = n;                     // Number of bytes returned
		PCD_ReadBytes(FIFODataReg, n, backData, rxAlign);  // Get received data from FIFO
		_validBits = PCD_ReadRegister(ControlReg) & 0x07;   // RxLastBits[2:0] indicates the number of valid bits in the last received unsigned char. If this value is 000b, the whole unsigned char is valid.
		if (validBits) 
		{
			*validBits = _validBits;
		}
	}
  
	// Tell about collisions
	if (errorRegValue & 0x08) 
	{   // CollErr
		return STATUS_COLLISION;
	}
  
	// Perform CRC_A validation if requested.
	if (backData && backLen && checkCRC) 
	{
		// In this case a MIFARE Classic NAK is not OK.
		if (*backLen == 1 && _validBits == 4) 
		{
			return STATUS_MIFARE_NACK;
		}
		// We need at least the CRC_A value and all 8 bits of the last byte must be received.
		if (*backLen < 2 || _validBits != 0) 
		{
			return STATUS_CRC_WRONG;
		}
		// Verify CRC_A - do our own calculation and store the control in controlBuffer.
		unsigned char controlBuffer[2];
		enum StatusCode status = PCD_CalculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
		if (status != STATUS_OK) 
		{
			return status;
		}
		if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) 
		{
			return STATUS_CRC_WRONG;
		}
	}
  
  return STATUS_OK;
} 

enum StatusCode PCD_TransceiveData(  unsigned char *sendData,   ///< Pointer to the data to transfer to the FIFO.
                          unsigned char sendLen,   ///< Number of bytes to transfer to the FIFO.
                          unsigned char *backData,   ///< NULL or pointer to buffer if data should be read back after executing the command.
                          unsigned char *backLen,    ///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
                          unsigned char *validBits,  ///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits. Default NULL.
                          unsigned char rxAlign,   ///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
                          unsigned char checkCRC   ///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
                 ) 
{
	unsigned char waitIRq = 0x30;    // RxIRq and IdleIRq
	return PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
}

enum StatusCode PICC_RequestA( unsigned char *bufferATQA, ///< The buffer to store the ATQA (Answer to request) in
                      unsigned char *bufferSize  ///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
                    ) 
{
	return PICC_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, bufferSize);
} 

enum StatusCode PICC_REQA_or_WUPA( unsigned char command,     ///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
                        unsigned char *bufferATQA, ///< The buffer to store the ATQA (Answer to request) in
                        unsigned char *bufferSize  ///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
                      ) 
{
	unsigned char validBits;
	enum StatusCode status;
	if (bufferATQA == NULL || *bufferSize < 2) 
	{  // The ATQA response is 2 bytes long.
		return STATUS_NO_ROOM;
	}
	PCD_ClearRegisterBitMask(CollReg, 0x80);    // ValuesAfterColl=1 => Bits received after collision are cleared.
	validBits = 7;                  // For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
	status = PCD_TransceiveData(&command, 1, bufferATQA, bufferSize, &validBits, 0, 0);
	if (status != STATUS_OK) 
	{
		return status;
	}
	if (*bufferSize != 2 || validBits != 0) 
	{   // ATQA must be exactly 16 bits.
		return STATUS_ERROR;
	}
	return STATUS_OK;
} 

enum StatusCode PICC_Select( Uid *uid, unsigned char validBits)  //< Pointer to Uid struct. Normally output, but can also be used to supply a known UID.
                          //< The number of known UID bits supplied in *uid. Normally 0. If set you must also supply uid->size.                      
{
	unsigned char uidComplete;
	unsigned char selectDone;
	unsigned char useCascadeTag;
	unsigned char cascadeLevel = 1;
	enum StatusCode result;
	unsigned char count;
	unsigned char index;
	unsigned char uidIndex;          // The first index in uid->uidByte[] that is used in the current Cascade Level.
	signed char currentLevelKnownBits;   // The number of known UID bits in the current Cascade Level.
	unsigned char buffer[9];         // The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A
	unsigned char bufferUsed;        // The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
	unsigned char rxAlign;         // Used in BitFramingReg. Defines the bit position for the first bit received.
	unsigned char txLastBits;        // Used in BitFramingReg. The number of valid bits in the last transmitted byte. 
	unsigned char *responseBuffer;
	unsigned char responseLength;
  
	if (validBits > 80) 
	{
		return STATUS_INVALID;
	}
  
	// Prepare MFRC522
	PCD_ClearRegisterBitMask(CollReg, 0x80);    // ValuesAfterColl=1 => Bits received after collision are cleared.
  
	// Repeat Cascade Level loop until we have a complete UID.
	uidComplete = false;
	while (!uidComplete)
	{
		// Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
		switch (cascadeLevel) 
		{
			case 1:
				buffer[0] = PICC_CMD_SEL_CL1;
				uidIndex = 0;
				useCascadeTag = validBits && uid->size > 4; // When we know that the UID has more than 4 bytes
				break;
		  
			case 2:
				buffer[0] = PICC_CMD_SEL_CL2;
				uidIndex = 3;
				useCascadeTag = validBits && uid->size > 7; // When we know that the UID has more than 7 bytes
				break;
		  
			case 3:
				buffer[0] = PICC_CMD_SEL_CL3;
				uidIndex = 6;
				useCascadeTag = false;            // Never used in CL3.
				break;
		  
			default:
				return STATUS_INTERNAL_ERROR;

		}		
		// How many UID bits are known in this Cascade Level?
		currentLevelKnownBits = validBits - (8 * uidIndex);
		if (currentLevelKnownBits < 0) 
		{
		  currentLevelKnownBits = 0;
		}
		// Copy the known bits from uid->uidByte[] to buffer[]
		index = 2; // destination index in buffer[]
		if (useCascadeTag) 
		{
			buffer[index++] = PICC_CMD_CT;
		}
		unsigned char bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
		if (bytesToCopy) 
		{
			unsigned char maxBytes = useCascadeTag ? 3 : 4; // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
			if (bytesToCopy > maxBytes) 
			{
				bytesToCopy = maxBytes;
			}
			for (count = 0; count < bytesToCopy; count++)
			{
				buffer[index++] = uid->uidByte[uidIndex + count];
			}
		}
		// Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
		if (useCascadeTag) 
		{
		  currentLevelKnownBits += 8;
		}
		
		// Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
		selectDone = false;
		while (!selectDone) 
		{
			// Find out how many bits and bytes to send and receive.
			if (currentLevelKnownBits >= 32) 
			{ // All UID bits in this Cascade Level are known. This is a SELECT.
				//Serial.print(F("SELECT: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes
				// Calculate BCC - Block Check Character
				buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
				// Calculate CRC_A
				result = PCD_CalculateCRC(buffer, 7, &buffer[7]);
				if (result != STATUS_OK) 
				{
					return result;
				}
				txLastBits    = 0; // 0 => All 8 bits are valid.
				bufferUsed    = 9;
				// Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
				responseBuffer  = &buffer[6];
				responseLength  = 3;
			}
			else 
			{ // This is an ANTICOLLISION.
				//Serial.print(F("ANTICOLLISION: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				txLastBits    = currentLevelKnownBits % 8;
				count     = currentLevelKnownBits / 8;  // Number of whole bytes in the UID part.
				index     = 2 + count;          // Number of whole bytes: SEL + NVB + UIDs
				buffer[1]   = (index << 4) + txLastBits;  // NVB - Number of Valid Bits
				bufferUsed    = index + (txLastBits ? 1 : 0);
				// Store response in the unused part of buffer
				responseBuffer  = &buffer[index];
				responseLength  = sizeof(buffer) - index;
			}
		  
			// Set bit adjustments
			rxAlign = txLastBits;                     // Having a separate variable is overkill. But it makes the next line easier to read.
			PCD_WriteRegister(BitFramingReg, (rxAlign << 4) + txLastBits);  // RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
		  
			// Transmit the buffer and receive the response.
			result = PCD_TransceiveData(buffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign, 0);
			if (result == STATUS_COLLISION) 
			{ // More than one PICC in the field => collision.
				unsigned char valueOfCollReg = PCD_ReadRegister(CollReg); // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
				if (valueOfCollReg & 0x20) 
				{ // CollPosNotValid
					return STATUS_COLLISION; // Without a valid collision position we cannot continue
				}
				unsigned char collisionPos = valueOfCollReg & 0x1F; // Values 0-31, 0 means bit 32.
				if (collisionPos == 0)
				{
					collisionPos = 32;
				}
				if (collisionPos <= currentLevelKnownBits) 
				{ // No progress - should not happen 
					return STATUS_INTERNAL_ERROR;
				}
				// Choose the PICC with the bit set.
				currentLevelKnownBits = collisionPos;
				count     = (currentLevelKnownBits - 1) % 8; // The bit to modify
				index     = 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
				buffer[index] |= (1 << count);
			}
			else if (result != STATUS_OK) 
			{
				return result;
			}
			else 
			{ // STATUS_OK
				if (currentLevelKnownBits >= 32) 
				{ 	// This was a SELECT.
					selectDone = true; // No more anticollision 
					// We continue below outside the while.
				}
				else 
				{ // This was an ANTICOLLISION.
					// We now have all 32 bits of the UID in this Cascade Level
					currentLevelKnownBits = 32;
					// Run loop again to do the SELECT.
				}
			}
		} // End of while (!selectDone)
		
		// We do not check the CBB - it was constructed by us above.
		
		// Copy the found UID bytes from buffer[] to uid->uidByte[]
		index     = (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
		bytesToCopy   = (buffer[2] == PICC_CMD_CT) ? 3 : 4;
		for (count = 0; count < bytesToCopy; count++) 
		{
		  uid->uidByte[uidIndex + count] = buffer[index++];
		}
		
		// Check response SAK (Select Acknowledge)
		if (responseLength != 3 || txLastBits != 0) 
		{ // SAK must be exactly 24 bits (1 byte + CRC_A).
		  return STATUS_ERROR;
		}
		// Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
		result = PCD_CalculateCRC(responseBuffer, 1, &buffer[2]);
		if (result != STATUS_OK) {
		  return result;
		}
		if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2])) {
		  return STATUS_CRC_WRONG;
		}
		if (responseBuffer[0] & 0x04) { // Cascade bit set - UID not complete yes
		  cascadeLevel++;
		}
		else
		{
		  uidComplete = true;
		  uid->sak = responseBuffer[0];
		}
	} // End of while (!uidComplete)
  
	// Set correct uid->size
	uid->size = 3 * cascadeLevel + 1;
  
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
// Support functions
/////////////////////////////////////////////////////////////////////////////////////

enum PICC_Type PICC_GetType(unsigned char sak)   ///< The SAK byte returned from PICC_Select().
{
  // http://www.nxp.com/documents/application_note/AN10833.pdf
  // 3.2 Coding of Select Acknowledge (SAK)
  // ignore 8-bit (iso14443 starts with LSBit = bit 1)
  // fixes wrong type for manufacturer Infineon (http://nfc-tools.org/index.php?title=ISO14443A)
  sak &= 0x7F;
  switch (sak)
  {
    case 0x04:  return PICC_TYPE_NOT_COMPLETE;  // UID not complete
    case 0x09:  return PICC_TYPE_MIFARE_MINI;
    case 0x08:  return PICC_TYPE_MIFARE_1K;
    case 0x18:  return PICC_TYPE_MIFARE_4K;
    case 0x00:  return PICC_TYPE_MIFARE_UL;
    case 0x10:
    case 0x11:  return PICC_TYPE_MIFARE_PLUS;
    case 0x01:  return PICC_TYPE_TNP3XXX;
    case 0x20:  return PICC_TYPE_ISO_14443_4;
    case 0x40:  return PICC_TYPE_ISO_18092;
    default:  return PICC_TYPE_UNKNOWN;
  }
} // End PICC_GetType()

void PCD_DumpVersionToSerial() 
{
	// Get the MFRC522 firmware version
	unsigned char v = PCD_ReadRegister(VersionReg);
	Report("Firmware Version: 0x%x\n",v);
	// Lookup which version
	switch(v) 
	{
		case 0x88: Report(" = (clone)\n");  break;
		case 0x90: Report(" = v0.0\n");     break;
		case 0x91: Report((" = v1.0\n"));     break;
		case 0x92: Report((" = v2.0\n"));     break;
		default:   Report((" = (unknown)\n"));
	}
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF))
		Report(("WARNING: Communication failure, is the MFRC522 properly connected?"));
} 	// End PCD_DumpVersionToSerial()

void PICC_DumpDetailsToSerial(Uid *uid) ///< Pointer to Uid struct returned from a successful PICC_Select().
{
	unsigned char i;
	// UID
	Report("Card UID:\n");
	for (i = 0; i < uid->size; i++)
	{
		if(uid->uidByte[i] < 0x10)
			Report(" 0");
		else
			Report(" ");
		Report("0x%x\n", uid->uidByte[i]);
	} 
	Report("\n");  
	// SAK
	Report("Card SAK: \n");
	if(uid->sak < 0x10)
		Report("0");
	Report("0x%x\n", uid->sak);  
	// (suggested) PICC type
	enum PICC_Type piccType = PICC_GetType(uid->sak);
	Report("PICC type: ");
} // End PICC_DumpDetailsToSerial()


/////////////////////////////////////////////////////////////////////////////////////
// Convenience functions - does not add extra functionality
/////////////////////////////////////////////////////////////////////////////////////
unsigned char PICC_IsNewCardPresent()			//API 1 to be called first
 {
	unsigned char bufferATQA[2];
	unsigned char bufferSize = sizeof(bufferATQA);
	enum StatusCode result = PICC_RequestA(bufferATQA, &bufferSize);
	return (result == STATUS_OK || result == STATUS_COLLISION);
} // End PICC_IsNewCardPresent()


unsigned char PICC_ReadCardSerial() 		//API 1 to be called next most important API that determines the uid
{
  enum StatusCode result = PICC_Select(&uid, 0);
  return (result == STATUS_OK);
} // End 

void PICC_DumpToSerial(Uid *uid)  //< Pointer to Uid struct returned from a successful PICC_Select().
{								 //API 3 to be called if you wish to dump the uid and other details on the memory
	// Dump UID, SAK and Type
	PICC_DumpDetailsToSerial(uid);
	// Dump contents
	enum PICC_Type piccType = PICC_GetType(uid->sak);
	switch (piccType) 
	{
		case PICC_TYPE_MIFARE_MINI:
		case PICC_TYPE_MIFARE_1K:
		case PICC_TYPE_MIFARE_4K:
			Report("MINI, 1k or 4k found and dumped\n");
			break;		  
		case PICC_TYPE_MIFARE_UL:
			Report("UL found and dumped\n");
			break;		  
		case PICC_TYPE_ISO_14443_4:
		case PICC_TYPE_ISO_18092:
		case PICC_TYPE_MIFARE_PLUS:
		case PICC_TYPE_TNP3XXX:
			//Report("Dumping memory contents not implemented for that PICC type.\n");
			break;		  
		case PICC_TYPE_UNKNOWN:
		case PICC_TYPE_NOT_COMPLETE:
		default:
			//Report("Invalid PICC type.\n");
			break;	
	}  
	//Report("\n");
	PICC_HaltA(); // Already done if it was a MIFARE Classic PICC.
} // End PICC_DumpToSerial()


//*****************************************************************************
//
//  timer ISR for ticket dispense
void TimerRefIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulRefBase);
    //Message("IN ISR Reference\n");
    MAP_GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1, (MAP_GPIOPinRead(GPIOA1_BASE, GPIO_PIN_1) ^ GPIO_PIN_1));
}

// timer ISR for coin mechanism


void TimerBaseIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);
    gameStartFlag=1;
    Message("IN ISR Base\n");
}


//*****************************************************************************
//
//! Main function for spi demo application
//!
//! \param none
//!
//! \return None.
//
//*****************************************************************************
void main()
{
	int iRetVal = 0;
	unsigned char BtnVal = 0;     // for button input

    g_ulBase = TIMERA0_BASE;
    g_ulRefBase = TIMERA1_BASE;
    //
    // Initialize Board configurations
    //
    BoardInit();
    //
    // Muxing UART and SPI lines.    //
    PinMuxConfig();
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);
#ifndef NOUART
    //
    // Initialising the Terminal.
    //
    InitTerm();
    //
    // Clearing the Terminal.
    //
    ClearTerm();
    //
    // Display the Banner
    //
    Message("\n\n\n\r");
    Message("\t\t   ********************************************\n\r");
    Message("\t\t        MFRC522 (CC3200 in master mode)  \n\r");
    Message("\t\t   ********************************************\n\r");
    Message("\n\n\n\r");
#endif

    MAP_GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5, 0);				// Set logic low and complete the pulse

    // timer init for pwm
    Timer_IF_Init(PRCM_TIMERA1, g_ulRefBase,TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_IntSetup(g_ulRefBase, TIMER_A, TimerRefIntHandler);   // set timer interrupt

    // timer init for coin mechanism

    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_ONE_SHOT, TIMER_A, 0);
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerBaseIntHandler);

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MFRC522_init();
    iRetVal = APConnectInit();
    if(iRetVal < 0)
    {
    	Message("\n Failed to connect to AP all server communication will fail!!!!");
    }
    while(1)
    {

    	Coinflag = MFRC522loop();
		if(Coinflag == 1)
		{
			CallHttpPost();											// Actually we will have a get command here and we will do a query
			MAP_GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5, GPIO_PIN_5);  	// set logic high on coin pulse gpio
			Timer_IF_Start(g_ulBase, TIMER_A, 600);  				// start the timer for oneshot pulse
			while(!gameStartFlag);									// Wait until Pulse width time
			MAP_GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_5, 0);				// Set logic low and complete the pulse
  			Timer_IF_Start(g_ulRefBase,TIMER_A, 100); 				// Start timer for ticket dispense pulse
  			Coinflag = 0;											// Ensure its ready for new read
  			gameStartFlag = 0;										// Ensure its ready for new read

  			BtnVal = (MAP_GPIOPinRead(GPIOA2_BASE,GPIO_PIN_6) & (GPIO_PIN_6) )?1:0;
  			while(!BtnVal)											// AS of now this is current sensor, when button goes low you stop
  			{
  				BtnVal = (MAP_GPIOPinRead(GPIOA2_BASE,GPIO_PIN_6) & (GPIO_PIN_6) )?1:0;
  				//we can add the melody program here
  			}
  			Timer_IF_Stop(g_ulRefBase, TIMER_A);					// Button went low stop the ticketing and complete the loop
  			MAP_GPIOPinWrite(GPIOA1_BASE,GPIO_PIN_1, 0);  			//switch off the gpio of ticket dispense
  			MAP_GPIOPinWrite(GPIOA0_BASE,GPIO_PIN_7, 0);  			// turn off buzzer
  			//We need to have a server post command to send the tickets
		}
    }

}

