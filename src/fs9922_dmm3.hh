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
 * Parser for the serial data output of the FS9922_DMM3 chip
 * 
 * Specification:
 * http://www.ic-fortune.com/upload/Download/FS9922-DMM3-DS-11_EN.pdf
 */
#ifndef FS9922_DMM3_HH
#define FS9922_DMM3_HH

#include <math.h>
#include <string>

enum unit_prefix_t
{
    PREFIX_MEGA = 0x10,
    PREFIX_KILO = 0x20,
    PREFIX_MILLI = 0x40,
    PREFIX_MICRO = 0x80,
    PREFIX_NANO = 0x100
};

enum unit_t
{
    UNIT_FAHRENHEIT = 0x1,
    UNIT_DEGREE = 0x2,
    UNIT_FARAD = 0x4,
    UNIT_HERTZ = 0x8,
    UNIT_HFE = 0x10,
    UNIT_OHM = 0x20,
    UNIT_AMPERE = 0x40,
    UNIT_VOLT = 0x80,
    UNIT_DUTY = 0x100
};

enum minmax_t
{
    MINMAX_NONE = 0,
    MINMAX_MIN = 1,
    MINMAX_MAX = 2
};

enum power_t
{
    POWER_NONE = 0,
    POWER_DC = 1,
    POWER_AC = 2
};


/**
 * This class represents a single data frame and provides access methods for the
 * individual frame values
 */
class FS9922_DMM3
{
    public:
    
        // raw frame data
        const char* data;
        
        /**
         * Create frame
         * \param data raw frame data
         */
        FS9922_DMM3(const char* data);
        
        /**
         * Return value as floating point number in the unit `unit()` scaled
         * with `unit_prefix()`
         */
        float value();
        
        
        /**
         * Return unscaled value as floating point number in the unit `unit()`
         */
        float value_unscaled();
        
        
        /**
         * Return whether overflow occurs
         */
        bool overflow();
        
        /**
         * Return whether "hold" button is pressed
         */
        bool hold();
        
        /**
         * Return whether "relative value" button is pressed
         */
        bool relative();
        
        /**
         * Return whether bargraph is visible
         */
        bool bargraph();
        
        /**
         * Return whether autorange is active
         */
        bool autorange();
        
        /**
         * Return whether automatic power off is active
         */
        bool autopoweroff();
        
        /**
         * Return whether battery is low
         */
        bool lowbattery();
        
        /**
         * Return whether device is in "diode" measuring mode
         */
        bool diode();
        
        /**
         * Return whether device beeps
         */
        bool beep();
        
        /**
         * Return value of bargraph
         */
        int bargraph_value();
        
        /**
         * Return power measuring mode (AC or DC)
         */
        power_t power();
        
        /**
         * Return whether "min" or "max" button is pressed
         */
        minmax_t minmax();
        
        /**
         * Return unit prefix
         */
        unit_prefix_t unit_prefix();
        
        /**
         * Return unit
         */
        unit_t unit();
        
        
        /**
         * Return string representation of constantes
         */
        static std::string unit2str(unit_t t);
        static std::string unit_prefix2str(unit_prefix_t t);
        static std::string power2str(power_t t);
        static std::string minmax2str(minmax_t t);
};
#endif
