# LeakMon

## Arduino/Teensy Water Leakage Monitor


LeakMon is written for Teensy 4.1, with several FL-37 Rain Sensors (or
other versions like YL-83) attached to the ADC pins.  Software uses
DHCP to get an IP, and syncs RTC to NTP every few hours.  If RTC
battery is attached, the initial time is read from RTC as Teensy
boots.

Intended use is to place the Rain Sensors under e.g. refrigerator,
freezer and dishwasher, connect the Teensy to Ethernet, and run the
Python part of the software in a nearby server.  The aim is to use the
Python code to launch an alarm when any leakage is detected.

Whenever Teensy gets a UDP connection to port 8888, it replies by
sending a byte string containing unsigned little-endian integers as
 - 8bit int: data format version (now 0)
 - 16bit int: id of the installation
 - 32bit int: running sequence number since last boot
 - 32bit int: Unix timestamp of the data
 - N x 16bit int: the values of N sensors

In some future version the software may use e.g. SD card for logging
the values, and more expressive query language for reading it.  But
not yet.  Also a checksum for checking the data validity might be
nice.

N.B. The project just started.  Nothing really works yet.

## Not really instructions yet, but some notes

You should copy the file leakmon.conf.example as leakmon.conf and
edit.  The Python script is going to use it as the config file.
