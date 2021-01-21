# Uni-T UT61B multimeter libusb driver

An analysis and description of the USB protocol can be found at https://lukasschwarz.de/ut61b.

## Installation
The driver bases on the libusb library which has to be installed. Compile the program with a simple

    make

This creates the commandline tool **ut61b_cli** in a build/ subfolder.

### udev rule
In order to grant the libusb library access to the usb device, the capturing program has to be run as root. Alternatively, an udev rule can be applied, which grants access at user level. An example rule is found in [utils/88-ut61b.rules](utils/88-ut61b.rules). Copy this file to /etc/udev/rules.d/ and reload the udev rules with

    udevadm control --reload-rules
    udevadm trigger

## Usage
 * connect multimeter with the serial-to-usb cable
 * power on multimeter
 * enable serial data transmission via long pressing of REL button
 * start capturing data via **ut61b_cli**

Running the commandline tool **ut61b_cli** shows the live data. Passing a log file argument via

    ut61b_cli -f <file>

saves all the data into the file. Capturing is stopped via the key-stroke ctrl+c or passing a maximum time or frame count via 

    ut61b_cli -n <frames> -t <time>

The simple script [utils/ut61b_gp](utils/ut61b_gp) runs gnuplot to show the live data graphically. Usage

    ut61b_gp <ut61b_cli> <file>
      <ut61b_cli> : path to ut61b_cli binary
      <file>      : path to logfile
