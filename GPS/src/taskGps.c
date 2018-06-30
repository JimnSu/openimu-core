/** ***************************************************************************
 * @file taskGps.c handle GPS data, make sure the GPS handling function gets
 *  called on a regular basis
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *****************************************************************************/
/*******************************************************************************
Copyright 2018 ACEINNA, INC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "driverGPS.h"
#include "uart.h"
#include "gps.h"
#include "osapi.h"
#include "platformAPI.h"
#include "timer.h"    // for TimeNow()
#include "algorithm.h"

/** ****************************************************************************
 * @name TaskGps
 * @brief task callback with the main loop for handle GPS data, make sure the
 *        GPS handling function gets called on a regular basis;.
 * gCalibration.productConfiguration.bit.hasGps = 1; by setting:
 * <hasGps>true</hasGps> and <useGps>true</useGps> in name_IMU380.xml file
 * @param N/A
 * @retval N/A
 ******************************************************************************/
void TaskGps(void)
{
    static uint32_t updateHDOP, pollSirfCnt;

    // UART_COMM is 0
    if (getUnitCommunicationType() != UART_COMM) { // UART / SPI
        // TODO - still can use debug serial port
        // but for now - just idle loop.
        while(1) {
            OS_Delay( 1000);
        }
    }

    // start out with the DOP high
    gGpsDataPtr->HDOP = 50.0;

    /// initial internal GPS settings, uses settings to configure then switch
    //  to BINARY and faster baud
    // But so far only external GPS
 //   if ( IsInternalGPS() == true)  {
 //       SetConfigurationProtocolGPS(NMEA_TEXT);
 //       SetConfigurationBaudRateGps(BAUD_4800);
 //   }

    initGPSHandler();
    GPSHandler();

    while (1) {
        // could wait on a message from the data acquistion
        OS_Delay(5000);

        if (gAlgorithm.Behavior.bit.useGPS)   // useGPS   so far only external GPS
        {
            GPSHandler();
            uart_BIT(GPS_UART);  // 
            // or signal other tasks based on these params?

            // Set the GPS validity, based on the Horizontal Dilution of Precision
            if (gGpsDataPtr->gpsValid && gGpsDataPtr->HDOP > 15.0) {
                gGpsDataPtr->gpsValid = false;
            } else if (gGpsDataPtr->HDOP < 10.0) {
                gGpsDataPtr->gpsValid = true;
            }

            if (((TimeNow() / 1000) - updateHDOP) > 600) {
                gGpsDataPtr->HDOP = 50.0;
                updateHDOP = (TimeNow() / 1000);
                pollSiRFVersionMsg();
                pollSirfCnt++;
            }
        }
    }
}