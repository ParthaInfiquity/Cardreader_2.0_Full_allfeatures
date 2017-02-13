//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************


//*****************************************************************************
//
// Application Name     -   File operations 
// Application Overview -   This example demonstate the file operations that 
//                          can be performed by the applications. The 
//                          application use the serial-flash as the storage 
//                          medium.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_File_Operations
// or
// docs\examples\CC32xx_File_Operations.pdf
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup filesystem_demo
//! @{
//
//*****************************************************************************
#include <stdlib.h>
#include <string.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"

//Common interface includes
#include "gpio_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "pinmux.h"


#define APPLICATION_NAME        "FILE OPERATIONS"
#define APPLICATION_VERSION     "1.1.1"

#define SL_MAX_FILE_SIZE        64L*1024L       /* 64KB file */
//#define BUF_SIZE                2048
#define BUF_SIZE                120
#define USER_FILE_NAME          "fs_demo.txt"

/* Application specific status/error codes */
typedef enum{
    // Choosing this number to avoid overlap w/ host-driver's error codes
    FILE_ALREADY_EXIST = -0x7D0,
    FILE_CLOSE_ERROR = FILE_ALREADY_EXIST - 1,
    FILE_NOT_MATCHED = FILE_CLOSE_ERROR - 1,
    FILE_OPEN_READ_FAILED = FILE_NOT_MATCHED - 1,
    FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED -1,
    FILE_READ_FAILED = FILE_OPEN_WRITE_FAILED - 1,
    FILE_WRITE_FAILED = FILE_READ_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned char gaucCmpBuf[BUF_SIZE];
unsigned char gaucOldMacDonald[128];

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
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************

//*****************************************************************************
//
//!  This funtion includes the following steps:
//!  -open a user file for writing
//!  -write "Old MacDonalds" child song 37 times to get just below a 64KB file
//!  -close the user file
//!
//!  /param[out] ulToken : file token
//!  /param[out] lFileHandle : file handle
//!
//!  /return  0:Success, -ve: failure
//
//*****************************************************************************
long WriteFileToDevice(unsigned long *ulToken, long *lFileHandle)
{
    long lRetVal = -1;
    //int iLoopCnt = 0;

    //
    //  create a user file
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                FS_MODE_OPEN_CREATE(4096, \
                          _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                        ulToken,
                        lFileHandle);
    if(lRetVal < 0)
    {
        //
        // File may already be created
        //
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);
    }
    else
    {
        //
        // close the user file
        //
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        if (SL_RET_CODE_OK != lRetVal)
        {
            ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
        }
    }
    
    //
    //  open a user file for writing
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_WRITE, 
                        ulToken,
                        lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_OPEN_WRITE_FAILED);
    }
    
    //
    // write "Old MacDonalds" child song as many times to get just below a 64KB file
    //
    //for (iLoopCnt = 0; iLoopCnt < (SL_MAX_FILE_SIZE / sizeof(gaucOldMacDonald)); iLoopCnt++)
    //{
        lRetVal = sl_FsWrite(*lFileHandle, 0, (unsigned char *)gaucOldMacDonald, sizeof(gaucOldMacDonald));
        if (lRetVal < 0)
        {
            lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
            ASSERT_ON_ERROR(FILE_WRITE_FAILED);
        }
    //}				//This doesn't make sense for real application.
    
    //
    // close the user file
    //
    lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
    }

    return SUCCESS;
}

//*****************************************************************************
//
//!  This funtion includes the following steps:
//!    -open the user file for reading
//!    -read the data and compare with the stored buffer
//!    -close the user file
//!
//!  /param[in] ulToken : file token
//!  /param[in] lFileHandle : file handle
//!
//!  /return 0: success, -ve:failure
//
//*****************************************************************************
long ReadFileFromDevice(unsigned long ulToken, long lFileHandle)
{
    long lRetVal = -1;
    //int iLoopCnt = 0;

    //
    // open a user file for reading
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_READ,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_OPEN_READ_FAILED);
    }

    //
    // read the data and compare with the stored buffer
    //
    //for (iLoopCnt = 0; iLoopCnt < (SL_MAX_FILE_SIZE / sizeof(gaucOldMacDonald)); iLoopCnt++)
    //{
        lRetVal = sl_FsRead(lFileHandle, 0, gaucCmpBuf, sizeof(gaucOldMacDonald));
        if ((lRetVal < 0) || (lRetVal != sizeof(gaucOldMacDonald)))
        {
            lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
            ASSERT_ON_ERROR(FILE_READ_FAILED);
        }

#if 0
        lRetVal = memcmp(gaucOldMacDonald,
                         gaucCmpBuf,
                         sizeof(gaucOldMacDonald));
        if (lRetVal != 0)
        {
        	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
            //ASSERT_ON_ERROR(FILE_NOT_MATCHED);
        	return -1;
        }
#endif
    //}				//Does not make sense for a real application

    //
    // close the user file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
    }

    return SUCCESS;
}

//*****************************************************************************
//
//! \brief  the aim of this example code is to demonstrate File-system
//!          capabilities of the device.
//!         For simplicity, the serial flash is used as the device under test.
//!
//! \param  None
//!
//! \return none
//!
//! \note   Green LED is turned solid in case of success
//!         Red LED is turned solid in case of failure
//
//*****************************************************************************
int FileIo(int wrtflg, char *SSID, char *PWD)
{
    //long lRetVal;
    //unsigned char policyVal;
    long lFileHandle;
    unsigned long ulToken;
	int index1 = 0;
	int index2 = 0;
	int index3 = 0;

#if 0
    //
    // Initializing the CC3200 networking layers
    //
    lRetVal = sl_Start(NULL, NULL, NULL);
    if(lRetVal < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        LOOP_FOREVER();
    }

    //
    // reset all network policies
    //
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                    SL_CONNECTION_POLICY(0,0,0,0,0),
                    &policyVal,
                    1 /*PolicyValLen*/);
    if(lRetVal < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        LOOP_FOREVER();
    }
#endif	
	if(wrtflg)
	{
		while(SSID[index1] != '\0')
		{
			gaucOldMacDonald[index3++] = SSID[index1++];
		}
		gaucOldMacDonald[index3++] = '\0';
		while(PWD[index2] != '\0')
		{
			gaucOldMacDonald[index3++] = PWD[index2++];			//Note, we still need to use ind3 since for gaucOldMacDonald its continuous
		}
		gaucOldMacDonald[index3++] = '\0';
		if(WriteFileToDevice(&ulToken, &lFileHandle) < 0)
		{
			return -1;        
		}
	}
	else
	{
		if(ReadFileFromDevice(ulToken, lFileHandle) < 0)
		{
			return -1;
		}
		index1 = 0;
		index2 = 0;
		index3 = 0;
		while(gaucCmpBuf[index3] != '\0')
		{
			SSID[index1++] = gaucCmpBuf[index3++];			
		}
		SSID[index1++] = '\0';
		index3++;
		while(gaucCmpBuf[index3] != '\0')
		{
			PWD[index2++] = gaucCmpBuf[index3++];			
		}
		PWD[index2++] = '\0';
	}

	return 1;

}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
