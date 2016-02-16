## Introduction ##
Estick-jtag is a USB-JTAG adapter to program ARM-based microcontrollers
using OpenOCD. It allows real time debugging, in-system programming and
boundary-scan testing for embedded target devices. It uses eStick as the
USB device. The firmware is based on the opendous-jtag with some optimizations,
which is 3 times faster than the original version
of opendous-jtag.

The original idea of this project was to port OpenOCD-firmware of USBProg
to eStick. But after some code comparison, it turned out that opendous-jtag
has a better code that lead also to a faster flash/ram programming time.
And since opendous-jtag uses the microcontroller AT90USB162, which is also
used by eStick, the porting of the firmware from opendous-jtag to eStick
was without any problem.

Currently estick-jtag (firmware ver. 0.2) is much faster than USBProg
(12 times faster for ram programming), but it is still slower comparing
to the commercial device such as Segger J-Link (using ARM microcontroller).
The problem was here not really the usb frame rate, since estick-jtag was
also designed to send as much as possible data in a usb frame.

The bottleneck is the performance of the microcontroller to read
the data from the incoming buffer, send it to the target, read the result
from target and write it in outgoing buffer.
The whole processing time of 350 bytes (current max buffer size) of data between
device and the target is about 5 ms. It is still much slower than the time
used to send 350 byte of data from host to the device (the frame size of Full Speed USB is 1 ms). Of course, the transfer rate could be little bit faster
if it uses bigger buffer, but AT90USB162 has only 512 bytes of ram,
and it should be shared among stack, variables, incoming and outgoing buffer.

Nevertheless, the performance is quite good if you think that the price for
such a small device is less than $10 (source: <a href='http://code.google.com/p/micropendous/source/browse/trunk/Micropendous/Design/Micropendous1/Micropendous1_BillOfMaterials.txt'>micropendous</a>). In fact, as a student
at the University of Applied Sciences Technikum Wien, you can get eStick for
only 5 Euro :)


## News ##

  * Apr 11, 2010: The early version of OpenOCD 0.4.0 driver for eStick was commited, and work properly using the the same firmware.

  * Mar 08, 2010: this project was also ported to USB AVR Lab (<a href='http://code.google.com/p/usbvlab-jtag'>usbvlab-jtag</a>).

  * Feb 07, 2010: this project was ported back to USBProg (<a href='http://code.google.com/p/usbprog-jtag'>usbprog-jtag</a>).

  * Jan 17, 2010: opendous-jtag has synchronized it's source code with this project, so it's transfer rate is now similar with this project.

## eStick ##
![http://estick-jtag.googlecode.com/svn/wiki/images/eStick-01.png](http://estick-jtag.googlecode.com/svn/wiki/images/eStick-01.png)

eStick is a small USB device using Atmel Microcontroller AT90USB162.
It was developed by The Department of Embedded Systems at the University
of Applied Sciences Technikum Wien, and intended to give students a simple
but interesting way to get in contact with embedded system.

## eStick's pin ##
PORTB is configured for JTAG communications:

| **Pin** | **Function** |
|:--------|:-------------|
| 0       | TDI          |
| 1       | TMS          |
| 2       | TRST         |
| 3       | SRST         |
| 4       | TCK          |
| 5       | TDO          |

And following is the pin configuration of eStick:

<a href='http://estick-jtag.googlecode.com/svn/wiki/images/eStick-pin.png'>
<img src='http://estick-jtag.googlecode.com/svn/wiki/images/eStick-pin.png' width='500' />
</a>

## How to compile ##
The OpenOCD's source code
can be downloaded from the estick-jtag's repository, it was patched already with estick code. Following is the step to get/compile
openocd and make estick-jtag's firmware version 0.2:
```
svn export http://estick-jtag.googlecode.com/svn/tags/0.2.0/openocd/openocd-r1454.estick openocd-r1454.estick
cd openocd-r1454.estick
./bootstrap
./configure --enable-estick
make
```

```
svn export http://estick-jtag.googlecode.com/svn/tags/0.2.0/firmware firmware
cd firmware/src
make
```

## Test ##
LM3S2110 CAN Device Evaluation Board was used to test the transfer rate
of estick-jtag. This Board from Luminary Micro has 64KB of FLASH and 16KB of RAM.

### Run OpenOCD ###
```
$ openocd -f lm3s2110_estick.cfg

Info : eStick JTAG Interface ready
Info : JTAG tap: lm3s2110.cpu tap/device found: 0x3ba00477 (Manufacturer: 0x23b, Part: 0xba00, Version: 0x3)
Info : JTAG Tap/device matched
Warn : no tcl port specified, using default port 6666
```

### Loading to the RAM ###
```
$ arm-elf-gdb
GNU gdb (GDB) 7.0.1
Copyright (C) 2009 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "--host=i386-apple-darwin9.8.0 --target=arm-elf".
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.

(gdb) target remote localhost:3333
Remote debugging using localhost:3333
0xe59ff018 in ?? ()

(gdb) mon reset
JTAG tap: lm3s2110.cpu tap/device found: 0x3ba00477 (Manufacturer: 0x23b, Part: 0xba00, Version: 0x3)
JTAG Tap/device matched

(gdb) mon sleep 500

(gdb) mon poll

(gdb) monitor soft_reset_halt
requesting target halt and executing a soft reset
target state: halted
target halted due to breakpoint, current mode: Thread 
xPSR: 0x01000000 pc: 0x000000f0

(gdb) set mem inaccessible-by-default off

(gdb) monitor fast enable

(gdb) monitor verify_ircapture disable
verify Capture-IR is disabled


(gdb) mon mwb 0x20000000 0 64

(gdb) mon mdb 0x20000000 64
0x20000000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x20000020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 

(gdb) load test-ram
Loading section .text, size 0x48c lma 0x20000000
Loading section .eh_frame, size 0x4 lma 0x2000048c
Loading section .ARM.exidx, size 0x8 lma 0x20000490
Loading section .rodata, size 0x3840 lma 0x20000498
Loading section .data, size 0x100 lma 0x20003cd8
Start address 0x2000042d, load size 15832
Transfer rate: 12 KB/sec, 3166 bytes/write.

(gdb) mon mdb 0x20000000 64
0x00000000: 00 40 00 20 f1 00 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 
0x00000020: 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 
```

### Loading to the FLASH ###
```
$ telnet localhost 4444
Connected to localhost.
Escape character is '^]'.
Open On-Chip Debugger
> halt

> flash probe 0
flash 'stellaris' found at 0x00000000

> flash erase_sector 0 0 63
erased sectors 0 through 63 on flash bank 0 in 0.277004s

> mdb 0x0 64
0x00000000: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
0x00000020: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 

> flash write_image test-flash.bin 0 bin    
Algorithm flash write 2048 words to 0x0, 16246 remaining
Algorithm flash write 2048 words to 0x2000, 14198 remaining
Algorithm flash write 2048 words to 0x4000, 12150 remaining
Algorithm flash write 2048 words to 0x6000, 10102 remaining
Algorithm flash write 2048 words to 0x8000, 8054 remaining
Algorithm flash write 2048 words to 0xa000, 6006 remaining
Algorithm flash write 2048 words to 0xc000, 3958 remaining
Algorithm flash write 1910 words to 0xe000, 1910 remaining
wrote 64984 byte from file test-flash.bin in 7.204628s (8.808357 kb/s)

> mdb 0x0 64                                      
0x00000000: 00 40 00 20 f1 00 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 
0x00000020: 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 3d 04 00 00 
    
> verify_image test-flash.bin 0 bin
verified 64984 bytes in 0.717032s

```

### Transfer rate comparison ###


|  | **USBProg** | **opendous-jtag`*`** | **estick-jtag ver. 0.1`**`** | **estick-jtag ver. 0.2`***`** |
|:-|:------------|:---------------------|:-----------------------------|:------------------------------|
| FLASH | 0.6 KB/s    | 2.88 KB/s            | 2.6 KB/s                     | 8.8 KB/s                      |
| RAM | 1 KB/s      | ? KB/s               | 3 KB/s                       | 12 KB/s                       |

**`*`**:
The transfer rate of opendous-jtag are retrieved from opendous-jtag website.

**`**`**:
version 0.1 is the original ported version from opendous-jtag.

**`***`**:
version 0.2 is the optimized version.


The test was made with following information (except opendous-jtag):
  * Mac OS X 10.5.8, 2.33 GHz Intel Core 2 Duo, 2GB RAM
  * OpenOCD rev 1454
  * ARM GNU gdb (GDB) 7.0.1
  * AVR gcc version 4.3.2 (GCC)
  * Firmware was compiled with optimization: -Os
  * Target board: LM3S2110 CAN Device Evaluation Board
  * jtag\_speed 0
  * File size to flash: 64984 bytes
  * File size to ram: 15832 bytes