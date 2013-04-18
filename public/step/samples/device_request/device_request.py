#!/usr/bin/python
#
# ***************************************************************************
# Copyright (c) 2011, 2012 Digi International Inc.,
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
# Send SCI device_request to server. 
# -------------------------------------------------
# Usage: device_request.py <username> <password> <device_id> [<cloud_url>]
# -------------------------------------------------
import httplib
import base64
import sys

def Usage():
    print 'Usage: device_request.py <username> <password> <device_id> [<cloud_url>]\n'
   
def PostMessage(username, password, device_id, cloud_url):
    # create HTTP basic authentication string, this consists of
    # "username:password" base64 encoded
    auth = base64.encodestring("%s:%s"%(username,password))[:-1]
    
    # device request message to send to server
    message = """<sci_request version="1.0">
        <data_service>
            <targets>
                <device id="%s"/>
            </targets>
            <requests>
            <device_request target_name="myTarget">My device request data</device_request>
            </requests>
        </data_service>
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
    response_body = webservice.getfile().read()
    
    # print the output to standard out
    if statuscode == 200:
        print '\nResponse:'
        print response_body
    else:
        print '\nError: %d %s' %(statuscode, statusmessage)
    
    webservice.close()


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
        PostMessage(argv[0], argv[1], argv[2], cloud_url)

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
    
