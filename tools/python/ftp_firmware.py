#!/usr/bin/python
#
# ***************************************************************************
# Copyright (c) 2013 Digi International Inc.,
# All rights not expressly granted are reserved.
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
#
# ***************************************************************************
# query_firmware.py
# Send query_firmware_targets SCI operation to get a list of 
# firmware targets on the device. 
# -------------------------------------------------
# Usage: query_firmware.py <Username> <Password> <Device ID> [<Device Cloud URL>]
# -------------------------------------------------
from ftplib import FTP
import sys

def Usage():
    print 'Usage: ftp_firmware.py <Host address> <Username> <Password> <image name>'
    print '    Example Usage:' 
    print '        python ./ftp_firmware.py 10.20.30.40 myuser mypassword rtr.dwn'
   
def DownloadFirmware(host, username, password, image):
    fb = open(image, 'rb')
    ftp = FTP(host, username, password)
    ftp.storbinary('STOR image', fb)
    ftp.quit()

def main(argv):
    #process arguments
    count = len(argv);
    if count != 4:
        Usage()
    else:
        DownloadFirmware(argv[0], argv[1], argv[2], argv[3])

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

