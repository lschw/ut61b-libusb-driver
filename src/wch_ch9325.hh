/*
 * Uni-T UT61B libusb driver
 * Copyright (C) 2014 Lukas Schwarz
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Driver for the WCH CH9325 chip used in the UTD04 serial-to-usb data cable of
 * the digital multimeter Uni-T UT61B
 * 
 * ---------
 * Protocol:
 * 
 * 1. initialisize data transfer with a SET_REPORT control transfer to set the
 * baudrate to 2400:
 * 
 * bmRequestType = 21
 * bRequest = 09
 * wValue = 00 03
 * wIndex = 00 00
 * wLength = 05 00
 * data = 60 09 00 00 03
 * 
 * -> see http://www.usb.org/developers/hidpage/HID1_11.pdf for more details
 * 
 * 2. device sends 8 byte data packages in interrupt transfer:
 * 
 *   either
 * 
 *   f0 00 00 00 00 00 00 00 : empty data package
 * 
 *   or
 * 
 *   f1 XX 00 00 00 00 00 00 : 1 byte of data XX of the 14 bytes frame of the
 *                             FS9922-DMM3 serial protocol
 */
#ifndef WCH_CH9325_HH
#define WCH_CH9325_HH

#include <libusb.h>
#include <sstream>
#include <iostream>
#include <string.h>


/**
 * This class represents a single serial-to-usb adapter. It is the "next"
 * available USB device.
 */
class WCH_CH9325
{
    public:
        WCH_CH9325();
        ~WCH_CH9325();
        
        /**
         * Set callback function, which is called for each retrieved valid
         * 14 byte data frame of the FS9922-DMM3 serial protocol
         * \param callback function called for each retrieved data frame
         * \param arg additional argument passed to the callback function
         */
        void set_callback(void (*callback)(const char*, void*), void* arg=0);
        
        
        /**
         * Start interrupt transfer and retrieve data. A callback is called for
         * each retrieved valid frame.
         */
        void listen();
        
        /**
         * Stop listen
         */
        void stop();
        
    private:
        
        /**
         * Initialisize libusb
         */
        static void init();
        
        /**
         * Uninitialisize libusb
         */
        static void uninit();
        
        // global device counter
        static int cnt;
        
        // global libusb context
        static libusb_context* ctx;
        
        // device handle
        libusb_device_handle* devh;
        
        // callback and optional argument
        void (*callback)(const char*, void*);
        void* callback_arg;
        
        // flag whether in listen mode
        bool do_listen;
        
};
#endif
