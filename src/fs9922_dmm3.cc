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

#include "fs9922_dmm3.hh"

enum byte7_t
{
    B7_BPN = 0x1,
    B7_HOLD = 0x2,
    B7_REL = 0x4,
    B7_AC = 0x8,
    B7_DC = 0x10,
    B7_AUTO = 0x20
};


enum byte8_t
{
    B8_NANO = 0x2,
    B8_BAT = 0x4,
    B8_APO = 0x8,
    B8_MIN = 0x10,
    B8_MAX = 0x20
};


enum byte9_t
{
    B9_PERCENT = 0x2,
    B9_DIODE = 0x4,
    B9_BEEP = 0x8,
    B9_MEGA = 0x10,
    B9_KILO = 0x20,
    B9_MILLI = 0x40,
    B9_MICRO = 0x80
};


/**
 * Create frame
 */
FS9922_DMM3::FS9922_DMM3(const char* data) : data(data) { }


/**
 * Return value
 */
float FS9922_DMM3::value()
{
    if(overflow()) {
        return INFINITY;
    }
    float v = atof(data);
    switch(data[6]) {
        case 0x31:
            return v*1e-3;
        case 0x32:
            return v*1e-2;
        case 0x34:
            return v*1e-1;
    }
    return v;
}


/**
 * Return unscaled value
 */
float FS9922_DMM3::value_unscaled()
{
    float v = value();
    switch(unit_prefix()) {
        case PREFIX_MEGA:
            return v*1e6;
        case PREFIX_KILO:
            return v*1e3;
        case PREFIX_MILLI:
            return v*1e-3;
        case PREFIX_MICRO:
            return v*1e-6;
        case PREFIX_NANO:
            return v*1e-9;
    }
    return v;
}


/**
 * Return whether overflow occurs
 */
bool FS9922_DMM3::overflow()
{
    return (data[1] == '?');
}


/**
 * Return whether "hold" button is pressed
 */
bool FS9922_DMM3::hold()
{
    return (data[7] & B7_HOLD);
}


/**
 * Return whether "relative value" button is pressed
 */
bool FS9922_DMM3::relative()
{
    return (data[7] & B7_REL);
}


/**
 * Return whether bargraph is visible
 */
bool FS9922_DMM3::bargraph()
{
    return (data[7] & B7_BPN);
}


/**
 * Return whether autorange is active
 */
bool FS9922_DMM3::autorange()
{
    return (data[7] & B7_AUTO);
}


/**
 * Return whether automatic power off is active
 */
bool FS9922_DMM3::autopoweroff()
{
    return (data[8] & B8_APO);
}


/**
 * Return whether battery is low
 */
bool FS9922_DMM3::lowbattery()
{
    return (data[8] & B8_BAT);
}


/**
 * Return whether device is in "diode" measuring mode
 */
bool FS9922_DMM3::diode()
{
    return (data[9] & B9_DIODE);
}


/**
 * Return whether device beeps
 */
bool FS9922_DMM3::beep()
{
    return (data[9] & B9_BEEP);
}


/**
 * Return value of bargraph
 */
int FS9922_DMM3::bargraph_value()
{
    return ((data[11] & 0x80) ? -1 : 1)*(data[11] & 0x7f);
}


/**
 * Return power measuring mode (AC or DC)
 */
power_t FS9922_DMM3::power()
{
    if(data[7] & B7_DC) {
        return POWER_DC;
    }
    if(data[7] & B7_AC) {
        return POWER_AC;
    }
    return POWER_NONE;
}


/**
 * Return whether "min" or "max" button is pressed
 */
minmax_t FS9922_DMM3::minmax()
{
    if(data[8] & B8_MAX) {
        return MINMAX_MAX;
    }
    if(data[8] & B8_MIN) {
        return MINMAX_MIN;
    }
    return MINMAX_NONE;
}


/**
 * Return unit prefix
 */
unit_prefix_t FS9922_DMM3::unit_prefix()
{
    if(data[8] & B8_NANO) {
        return PREFIX_NANO;
    }
    return (unit_prefix_t)((uint8_t)data[9]);
}


/**
 * Return unit
 */
unit_t FS9922_DMM3::unit()
{
    if(data[9] & B9_PERCENT) {
        return UNIT_DUTY;
    }
    return (unit_t)((uint8_t)data[10]);
}


/**
 * Return string representation of unit
 */
std::string FS9922_DMM3::unit2str(unit_t t)
{
    switch(t) {
        case UNIT_FAHRENHEIT:
            return "°F";
        case UNIT_DEGREE:
            return "°C";
        case UNIT_FARAD:
            return "F";
        case UNIT_HERTZ:
            return "Hz";
        case UNIT_HFE:
            return "hFE";
        case UNIT_OHM:
            return "Ω";
        case UNIT_AMPERE:
            return "A";
        case UNIT_VOLT:
            return "V";
        case UNIT_DUTY:
            return "%";
        default:
            return "";
    }
}


/**
 * Return string representation of unit prefix
 */
std::string FS9922_DMM3::unit_prefix2str(unit_prefix_t t)
{
    switch(t) {
        case PREFIX_MEGA:
            return "M";
        case PREFIX_KILO:
            return "k";
        case PREFIX_MILLI:
            return "m";
        case PREFIX_MICRO:
            return "µ";
        case PREFIX_NANO:
            return "n";
        default:
            return "";
    }
}



/**
 * Return string representation of power mode
 */
std::string FS9922_DMM3::power2str(power_t t)
{
    switch(t) {
        case POWER_AC:
            return "AC";
        case POWER_DC:
            return "DC";
        default:
            return "";
    }
}


/**
 * Return string representation of min/max mode
 */
std::string FS9922_DMM3::minmax2str(minmax_t t)
{
    switch(t) {
        case MINMAX_MAX:
            return "MAX";
        case MINMAX_MIN:
            return "MIN";
        default:
            return "";
    }
}
