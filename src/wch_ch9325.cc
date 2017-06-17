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

#include "wch_ch9325.hh"

int WCH_CH9325::cnt = 0;
libusb_context* WCH_CH9325::ctx = 0;


/**
 * Open device
 */
WCH_CH9325::WCH_CH9325() : devh(0), do_listen(false)
{
    init();
    
    // loop through all usb devices
    libusb_device** devs;
    ssize_t dev_cnt = libusb_get_device_list(WCH_CH9325::ctx, &devs);
    for(ssize_t i = 0; i < dev_cnt; i++) {
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r != 0) {
            std::cerr << "Retrieving device descriptor failed: " << r << "\n";
            continue;
        }
        
        // check for correct vendor and product id
        if(desc.idVendor == 6790 && desc.idProduct == 57352) {
            
            // try opening device
            r = libusb_open(devs[i], &devh);
            if(r != 0) {
                std::cerr << "Opening device failed: " << r << "\n";
                continue;
            }
            if(libusb_kernel_driver_active(devh, 0) == 1) {
                if(libusb_detach_kernel_driver(devh, 0) != 0) {
                    std::cerr << "Detaching kernel driver failed" << "\n";
                    libusb_close(devh);
                    devh = 0;
                    continue;
                }
            }
            r = libusb_claim_interface(devh, 0);
            if(r != 0) {
                std::cerr << "Claiming interface failed: " << r << "\n";
                libusb_close(devh);
                devh = 0;
                continue;
            }
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    
    if(devh == 0) {
        throw std::runtime_error("No device found");
    }
    WCH_CH9325::cnt++;
}


/**
 * Close device
 */
WCH_CH9325::~WCH_CH9325()
{
    if(devh != 0) {
        libusb_release_interface(devh, 0);
        libusb_close(devh);
        WCH_CH9325::cnt--;
    }
    uninit();
}


/**
 * Listen for data packages
 */
void WCH_CH9325::listen()
{
    int transferred = 0;
    int pos = 0;
    unsigned char* data = new unsigned char[8];
    char* frame = new char[14];
    
    // send SET_REPORT request:
    
    // bytes 0 and 1 set baudrate = 2400
    data[0] = 0x60;
    data[1] = 0x09;
    
    // set bytes 2,3,4 to fixed values retrieved from sniffing the protocoll
    // meaning is unknown
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x03;
    
    int r = libusb_control_transfer(devh,
        0x21,   // bmRequestType = host-to-device, class specific, to interface
        0x09,   // bRequest = SET_REPORT
        0x0300, // wValue = feature report
        0x0,    // wIndex = interface number
        data,   // report data
        5,      // wLength length of report data
        100     // timeout in ms
    );
    
    if(r < 0) {
        std::stringstream ss;
        ss << "Sending SET_REPORT request failed: " << r;
        throw std::runtime_error(ss.str());
    }
    
    // retrieve data frames:
    do_listen = true;
    while(do_listen) {
        
        // wait for interrupt
        r = libusb_interrupt_transfer(devh,
            (2|LIBUSB_ENDPOINT_IN), // endpoint
            data,                   // data buffer
            8,                      // size of data buffer
            &transferred,           // tranferred data
            100                     // timeout in ms
        );
        
        // continue on timeout
        if(r == LIBUSB_ERROR_TIMEOUT) {
            continue;
        }
        
        // continue on errors
        if(r < 0) {
            std::cerr << "Interrupt transfer failed: " << r << "\n";
            continue;
        }
        
        // continue on invalid data length
        if(transferred != 8) {
            std::cerr << "Too less data transferred" << "\n";
            continue;
        }
        
        // frame contains data --> buffer data
        if(data[0] == 0xf1) {
            frame[pos] = data[1];
            pos++;
        }
        
        // data buffer is full
        if(pos == 14) {
            
            // check for valid frame
            if(frame[12] == 0x0d && frame[13] == 0x0a) {
                callback(frame, callback_arg);
                pos = 0;
            }
            // invalid or missaligned frame -> skip first byte
            else {
                memmove(frame, frame+1, 13);
                pos = 13;
            }
        }
    }
    delete[] data;
    delete[] frame;
}


/**
 * Stop listening
 */
void WCH_CH9325::stop()
{
    do_listen = false;
}


/**
 * Set callback function
 */
void WCH_CH9325::set_callback(void (*callback)(const char* data, void* arg),
    void* arg)
{
    this->callback = callback;
    this->callback_arg = arg;
}


/**
 * Initialisize libusb
 */
void WCH_CH9325::init()
{
    if(WCH_CH9325::ctx == 0) {
        int r = libusb_init(&WCH_CH9325::ctx);
        if(r != 0) {
            std::stringstream ss;
            ss << "Initialisizing libusb failed: " << r;
            throw std::runtime_error(ss.str());
        }
        libusb_set_debug(WCH_CH9325::ctx, 3);
    }
}


/**
 * Uninitialisize libusb
 */
void WCH_CH9325::uninit()
{
    if(WCH_CH9325::cnt == 0) {
        libusb_exit(WCH_CH9325::ctx);
        WCH_CH9325::ctx = 0;
    }
}
