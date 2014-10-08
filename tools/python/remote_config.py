#!/usr/bin/python
#
# ***************************************************************************
# Copyright (c) 2014 Digi International Inc.,
# All rights not expressly granted are reserved.
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
#
# ***************************************************************************
# device_request.py
# Send SCI device_request to Device Cloud. 
# -------------------------------------------------
# Usage: device_request.py <Username> <Password> <Device ID> [<Device Cloud URL>]
# -------------------------------------------------
import httplib
import base64
import sys
import difflib
import time

def Usage():
    print 'Usage: remote_config.py <Username> <Password> <Device ID> [<Device Cloud URL>]'
    print '    Sends an SCI query_setting request to <Device ID> to get the entire setting groups.'
    print '    The sample then sends second SCI set_setting with the collected settings.'
    print '    Finally, it sends a query_setting again to make sure the values are set correctly.'
    print '    where:' 
    print '        <Username> is the Device Cloud account Username to which your device is'
    print '                   connected.'
    print '        <Password> is the account password'
    print '        <Device ID> is the device to receive the Device Request.' 
    print '        [<Device Cloud URL>] is an optional Device Cloud URL.  The default URL is' 
    print '                   login.etherios.com.' 
    print '' 
    print '    Note:'
    print '        <Device ID> format can either be:'
    print '            Long: 00000000-00000000-00049DFF-FFAABBCC.' 
    print '            or short: 00049DFF-FFAABBCC\n'
    print '    Example Usage:' 
    print '        python ./remote_config.py myaccount mypassword 00049DFF-FFAABBCC'
    print '            Sends an SCI query_setting request to 00000000-00000000-00049DFF-FFAABBCC '
    print '            (in user account myaccount) through login.etherios.com.\n'
    print '        python ./remote_config.py myaccount mypassword 00049DFF-FFAABBCC login.etherios.co.uk'
    print '            Sends an SCI query_setting request to 00000000-00000000-00049DFF-FFAABBCC '
    print '            (in user account myaccount) through login.etherios.co.uk.\n'
   
def PostQuerySetting(username, password, device_id, cloud_url):
    # create HTTP basic authentication string, this consists of
    # "username:password" base64 encoded
    auth = base64.encodestring("%s:%s"%(username,password))[:-1]
    
    # device request message to send to Device Cloud
    message = """<sci_request version="1.0">
        <send_message cache="false">
            <targets>
                <device id="%s"/>
            </targets>
            <rci_request version="1.1">
                <query_setting/>
            </rci_request>
        </send_message>
    </sci_request>
    """%(device_id)
    
    # to what URL to send the request with a given HTTP method
    webservice = httplib.HTTP(cloud_url,80)
    webservice.putrequest("POST", "/ws/sci")
    
    # add the authorization string into the HTTP header
    webservice.putheader("Authorization", "Basic %s"%auth)
    webservice.putheader("Content-type", "text/xml; charset=\"UTF-8\"")
    webservice.putheader("Content-length", "%d" % len(message))
    webservice.endheaders()
    webservice.send(message)
    
    # get the response
    statuscode, statusmessage, header = webservice.getreply()    
    
    # print the output to standard out
    if statuscode == 200:
        response_body = webservice.getfile().read()
        print '\nquery-setting response:Success'
    else:
        print '\nquery-setting: %d %s' %(statuscode, statusmessage)
        response_body = None
    
    webservice.close()
    
    return response_body

def PostSetSetting(username, password, device_id, settings, cloud_url):
    # create HTTP basic authentication string, this consists of
    # "username:password" base64 encoded
    auth = base64.encodestring("%s:%s"%(username,password))[:-1]
    
    # device request message to send to Device Cloud
    message = """<sci_request version="1.0">
        <send_message cache="false">
            <targets>
                <device id="%s"/>
            </targets>
            <rci_request version="1.1">
                <set_setting>
                  "%s"
                </set_setting>
            </rci_request>
        </send_message>
    </sci_request>
    """%(device_id, settings)
    
    # to what URL to send the request with a given HTTP method
    webservice = httplib.HTTP(cloud_url,80)
    webservice.putrequest("POST", "/ws/sci")
    
    # add the authorization string into the HTTP header
    webservice.putheader("Authorization", "Basic %s"%auth)
    webservice.putheader("Content-type", "text/xml; charset=\"UTF-8\"")
    webservice.putheader("Content-length", "%d" % len(message))
    webservice.endheaders()
    webservice.send(message)
    
    # get the response
    statuscode, statusmessage, header = webservice.getreply()    
    
    # print the output to standard out
    if statuscode == 200:
        response_body = webservice.getfile().read()
        print '\nResponse:Success'
    else:
        response_body = None
        print '\nError: %d %s' %(statuscode, statusmessage)
    
    webservice.close()
    
    return response_body

def main(argv):
    #process arguments
    count = len(argv);
    if (count < 3) or (count > 4):
        Usage()
    else:
        if count > 3:
            cloud_url = argv[3]
        else:
            cloud_url = "login.etherios.com"
        response = PostQuerySetting(argv[0], argv[1], argv[2], cloud_url)
        if response != None:
            response1 = PostSetSetting(argv[0], argv[1], argv[2], response, cloud_url)
            if response1 != None:
                time.sleep(5);
                response2 = PostQuerySetting(argv[0], argv[1], argv[2], cloud_url)
                if response2 != None:
                    if response == response2:
                        print "Success: Response matched"
                    else:
                        print "There is a mismatch:"
                        diff = difflib.context_diff(response, response2)
                        delta = ''.join(diff)
                        print delta

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
