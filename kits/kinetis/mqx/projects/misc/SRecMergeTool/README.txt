This folder contains all the necessary files to import an Eclipse
project to build the SRecMerge tool.

It is a tiny tool that merges the FNET bootloader and an
application's SRecords into one, writing application's entrypoint
in the correct place of bootloader's memmory. This way, the resulting
.S19 can be flashed and the bootloader will launch the application.

It's primary intention is to launch an iDigi application that
asks the user for MAC, Vendor ID, Cloud server and network configuration
so it can connect to iDigi to perform a firmware update.