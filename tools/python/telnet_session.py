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
import telnetlib
import sys

def Usage():
    print 'Usage: telnet_session.py <Host address> <Username> <Password>'
    print '    Example Usage:' 
    print '        python ./telnet_session.py 10.20.30.40 myuser mypassword'
   
def TelnetSession(host, user, password):
    tn = telnetlib.Telnet(host)
    tn.read_until('Username: ')
    tn.write(user + "\r\n")
    tn.read_until('Password: ')
    tn.write(password + "\r\n")
    
    tn.read_until('>', 1)
    tn.write('cloud 0 server test.etherios.com\r\n')
    tn.read_until('>', 1)
    tn.write('cloud 0 clientconn on\r\n')
    tn.read_until('>', 1)
    tn.write('def_route 0 ll_ent eth\r\n')
    tn.read_until('>', 1)
    tn.write('def_route 0 ll_add 0\r\n')
    tn.read_until('>', 1)
    tn.write('config 0 save\r\n')
    tn.read_until('>', 1)
    tn.write('reboot\r\n')

def main(argv):
    #process arguments
    count = len(argv);
    if count != 3:
        Usage()
    else:
        TelnetSession(argv[0], argv[1], argv[2])

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

