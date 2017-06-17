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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <unistd.h>
#include "fs9922_dmm3.hh"
#include "wch_ch9325.hh"

static const std::string VERSION = "1.0.0";

// maximum frame number to capture until program stops (0 = inf)
int max_frame = 0;

// maximum time to capture until program stops (0 = inf)
int max_time = 0;

// path to data log file
std::string file;

// file handle for data logging
std::ofstream fh;

// start time of capturing
timeval t_start;

// device object
WCH_CH9325* dev = 0;

// number of already captured frames
int frame_no = 0;


/**
 * Callback which is called for each data frame
 * \param data data frame
 */
void handle_frame(const char* data, void*)
{
    FS9922_DMM3 frame(data);
    
    if(frame_no == 0) {
        // save start time
        gettimeofday(&t_start, 0);
    }
    
    if(!fh.is_open()) {
        // open log file for first time
        // write column header
        fh.open(file, std::ofstream::out);
        fh << "# time[s] value_unscaled value prefix unit power min/max hold ";
        fh << "rel auto apo bat diode beep\n";
    }
    frame_no++;
    
    // get time since start of data capturing
    timeval t_now;
    gettimeofday(&t_now, 0);
    double time = (t_now.tv_sec - t_start.tv_sec);
    time += (t_now.tv_usec - t_start.tv_usec)/1e7;
    
    // write data to file
    fh << time << " ";
    fh << frame.value_unscaled() << " ";
    fh << frame.value() << " ";
    fh << frame.unit_prefix2str(frame.unit_prefix()) << " ";
    fh << frame.unit2str(frame.unit()) << " ";
    fh << frame.power2str(frame.power()) << " ";
    fh << frame.minmax2str(frame.minmax()) << " ";
    fh << frame.hold() << " ";
    fh << frame.relative() << " ";
    fh << frame.autorange() << " ";
    fh << frame.autopoweroff() << " ";
    fh << frame.lowbattery() << " ";
    fh << frame.diode() << " ";
    fh << frame.beep() << " ";
    fh << "\n" << std::flush;
    
    // show data
    system("clear");
    std::cout << "Uni-T UT61B\n\n";
    std::cout << "time   : ";
    std::cout << std::fixed << std::setprecision(2) << time << " s";
    if(max_time != 0) {
        std::cout << " (max " << max_time << " s)";
    }
    std::cout << "\n";
    std::cout << "frame  : " << frame_no;
    if(max_frame != 0) {
        std::cout << " (max " << max_frame << ")";
    }
    std::cout << "\n";
    std::cout << "value  : " << frame.value();
    std::cout << " ";
    std::cout << frame.unit_prefix2str(
        frame.unit_prefix()
    );
    std::cout << frame.unit2str(
        frame.unit()
    );
    std::cout << "\n";
    std::cout << "status : ";
    if(frame.hold()) {
        std::cout << "HOLD ";
    }
    if(frame.relative()) {
        std::cout << "REL ";
    }
    if(frame.autorange()) {
        std::cout << "AUTO ";
    }
    if(frame.autopoweroff()) {
        std::cout << "APO ";
    }
    if(frame.lowbattery()) {
        std::cout << "BAT ";
    }
    if(frame.diode()) {
        std::cout << "DIODE ";
    }
    if(frame.beep()) {
        std::cout << "BEEP ";
    }
    std::cout << frame.power2str(frame.power()) << " ";
    std::cout << frame.minmax2str(frame.minmax()) << " ";
    std::cout << "\n\n";
    std::cout << "bargraph:\n";
    if(frame.bargraph()) {
        int v = frame.bargraph_value();
        int BAR_MAX = 40;
        int maxv = (std::abs(v) < BAR_MAX) ? abs(v) : BAR_MAX;
        std::cout << ((v < 0) ? "-" : "+") << " [ ";
        for(int i = 0; i < maxv; ++i) {
            std::cout << "|";
        }
        for(int i = maxv; i < BAR_MAX; ++i) {
            std::cout << " ";
        }
        std::cout << " ]\n";
        std::cout << "    1        10        20        30        40";
    }
    std::cout << "\n\n";
    std::cout << "raw value:\n";
    std::ios::fmtflags f(std::cout.flags()); // save std::cout format
    for(int i=0; i < 14; ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex;
        std::cout << ((int)(unsigned char) data[i]) << " ";
    }
    std::cout.flags(f); // restore std::cout format
    std::cout << "\n";
    
    // check if max time is reached
    if(max_time != 0 && time > max_time) {
        dev->stop();
    }
    
    // check if max frame is reached
    if(max_frame != 0 && frame_no > max_frame) {
        dev->stop();
    }
}


/**
 * Print program usage
 */
void usage()
{
    std::cout << "Command line tool for capturing data from the digital multimeter Uni-T UT61b\n";
    std::cout << "Copyright (C) 2014 Lukas Schwarz\n";
    std::cout << "\n";
    std::cout << "Usage: ut61b_cli [OPTION]\n";
    std::cout << "Options:\n";
    std::cout << "-h            show help\n";
    std::cout << "-v            show version\n";
    std::cout << "-f <file>     log data to file\n";
    std::cout << "-n <frames>   maximum count of data frames to capture\n";
    std::cout << "-t <time>     maximum time (in sec) to capture data\n";
    std::cout << "\n";
    std::cout << "This program is free software: you can redistribute it and/or modify\n";
    std::cout << "it under the terms of the GNU General Public License as published by\n";
    std::cout << "the Free Software Foundation, either version 3 of the License, or\n";
    std::cout << "(at your option) any later version.\n";
    std::cout << "\n";
    std::cout << "This program is distributed in the hope that it will be useful,\n";
    std::cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
    std::cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
    std::cout << "GNU General Public License for more details.\n";
    std::cout << "\n";
    std::cout << "You should have received a copy of the GNU General Public License\n";
    std::cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";
}


int main(int argc, char* argv[])
{
    // parse command line arguments
    int c;
    opterr = 0;
    while((c = getopt(argc, argv, "hvf:n:t:")) != -1) {
        switch(c) {
            case 'h':
                usage();
                return 0;
            case 'v':
                std::cout << VERSION << "\n";
                return 0;
            case 'f':
                file = optarg;
                break;
            case 'n':
                max_frame = atoi(optarg);
                break;
            case 't':
                max_time = atoi(optarg);
                break;
            case '?':
                if(optopt == 'f') {
                    std::cerr << "Option -f requires a file name\n";
                }
                else if(optopt == 'n') {
                    std::cerr << "Option -n requires a frame number\n";
                }
                else if(optopt == 't') {
                    std::cerr << "Option -t requires a time (in sec.)\n";
                }
                else {
                    std::cerr << "Invalid option '" << (char)optopt << "'\n";
                }
                std::cerr << "Type ut61b_cli -h for help\n";
                return 1;
            default:
                usage();
                return 1;
        }
    }
    
    // open device and start listening
    try{
        dev = new WCH_CH9325();
        dev->set_callback(handle_frame, 0);
        dev->listen();
        delete dev;
    } catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
