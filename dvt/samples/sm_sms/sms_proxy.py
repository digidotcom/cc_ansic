#!/usr/bin/env python 

""" 
Intended to run on a Digi Gateway like ConnectPortX4,
configured with SMS enabled but with iDigi SMS disabled.
The application will listen on port 9999 for TCP connections from client.
SMSs messages received by the gateway will be forwarded through TCP to the client.
Messages received from the client through TCP will be sent as an SMS to the configured
phone_number.
To configure the phone-number send following message: 
    phone-number=xxxxxxxxxx
examples:
    for login.etherios.com: phone-number=447786201216
    for test.etherios.com:  phone-number=447786201217
""" 

import socket 
import digisms
import time
import string

#etherios telephone where to send SMSs
#requires initialitation using 'phone-number=xxxxxxx'
phone_number = ''

#listen on localhost
host = ''

#port where to listen
port = 9999 

backlog = 5 
size = 1024 

#for tests, skip sms sent.
mute = 0
 
def sms_callback(a):
  print """\
 
Received SMS from: %s
at: %s
====================
%s
====================
""" % (a.source_addr, a.timestamp, a.message)

  client.send(a.message) 
 

print ("mute: %d" % (mute))

sms = digisms.Callback(sms_callback)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
s.bind((host,port)) 
s.listen(backlog) 
exit1 = 0
while exit1 == 0: 
    client, address = s.accept() 
    print "hello ", address
    exit2 = 0
    while exit2 == 0: 
        data = client.recv(size) 
        if data == 'x': 
            exit1 = 1
            exit2 = 1
        elif data == 'm': 
            mute = not(mute)
            print ("mute: %d" % (mute))
        elif data == '': 
            # Client closed connection. Go to accept again.
            exit2 = 1
        elif data[:13] == 'phone-number=':
            phone_number = data[13:]
            print ("configuring phone-number=%s" %(phone_number))
        elif phone_number == '':
            print ("phone_number not set!!!!!!!. Use 'phone-number' command")
        elif data:
            if mute == 0:
                print ("Sending SMS to %s: %s" % (phone_number, data))
                digisms.send(phone_number, data)
            else:
                print ("SKIP Sending SMS to %s: %s" % (phone_number, data))
        else:
            print ("XXXX: %s" % (data))

    print "bye ", address
    client.close()
