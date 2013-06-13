from nose.tools import *
import xml.dom.minidom          # http://docs.python.org/2/library/xml.dom.html
import xml.sax.saxutils         # http://wiki.python.org/moin/EscapingXml
import time
import ic_testcase
import logging
from ic_plugin import ICPlugin

log = logging.getLogger('ic_testcase')
log.setLevel(logging.INFO)
if len(log.handlers) == 0:
    handler = logging.StreamHandler()
    handler.setLevel(logging.INFO)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    log.addHandler(handler)

logging.getLogger('requests').setLevel(logging.WARNING)


##################################################################
#Templates for XML to send.
##################################################################
RCI_BASE_TEMPLATE = """<sci_request version="1.0">
  <send_message cache="%s">
    <targets>
      <device id="%s"/>
    </targets>
    <rci_request version="1.1">
      %s
    </rci_request>
  </send_message>
</sci_request>"""

QUERY_DESCRIPTOR_SETTING = """<query_descriptor><query_setting/></query_descriptor>"""

QUERY_DESCRIPTOR_STATE = """<query_descriptor><query_state/></query_descriptor>"""

SET_SETTING = """<set_setting><%s index="%s"><%s>%s</%s></%s></set_setting>"""

QUERY_SETTING = """<query_setting><%s/></query_setting>"""

SET_STATE = """<set_state><%s index="%s"><%s>%s</%s></%s></set_state>"""

QUERY_STATE = """<query_state><%s/></query_state>"""



def send_rci(request):
    try:
        req_data = xml.dom.minidom.parseString(request).toxml()
    except Exception, e:
        error = "Invalid XML passed in: %s"%request
        assert 0==1, error
    log.info("Sending SCI Request: \n%s" % request)
    response = ICPlugin.session.post('https://%s/ws/sci' % ICPlugin.hostname,
                    data=request,
                    verify=False)

    assert_equal(200, response.status_code, "Non 200 Status Code: %d.  " \
        "Response: %s" % (response.status_code, response.content))
    try:
        res_data = xml.dom.minidom.parseString(response.content)
        log.info("Received SCI Response: \n%s" \
            % res_data.toprettyxml(indent=' '))
        return res_data
    except Exception, e:
        error = "Response was not valid XML: %s" % response.content
        assert 0==1, error






class Test_brci_string(object):

    def ensure_connected(self):
        log.info("Ensuring Device %s is connected." \
            % ICPlugin.device_id)

        response = ICPlugin.rest_session.get_first('DeviceCore',
                        params={'condition' : "devConnectwareId='%s'" \
                        % ICPlugin.device_id})

        assert_equal(200, response.status_code)
        self.device_core = response.resource

        # If not connected, fail the TestCase.
        if not self.device_core.dpConnectionStatus == '1':
            assert_equal('1', self.device_core.dpConnectionStatus,
                "Device %s not connected." % ICPlugin.device_id)



    def getSettingValue(self, groupName, settingName, cache = "false"):
        # Create RCI query, ej:
        # <sci_request version="1.0">
        #       <send_message cache="no">
        #           <targets>
        #               <device id="00000000-00000000-BD34D8FF-FFD8E610"/>
        #           </targets>
        #           <rci_request version="1.1">
        #               <query_setting>
        #                   <device_info/>
        #               </query_setting>
        #           </rci_request>
        #       </send_message>
        # </sci_request>
        rciQuery = RCI_BASE_TEMPLATE % ( cache, # cache
                                         ICPlugin.device_id, # Device ID
                                         QUERY_SETTING % groupName) # RCI query


        # Send request to server and get the response
        # Response returned by server, ej:
        # <?xml version="1.0" ?>
        #   <sci_reply version="1.0">
        #     <send_message>
        #       <device id="00000000-00000000-BD34D8FF-FFD8E610">
        #         <rci_reply version="1.1">
        #           <query_setting>
        #             <device_info>
        #               <product>default</product>
        #               <model>default</model>
        #               <company>default</company>
        #               <desc>default</desc>
        #               <syspwd>default</syspwd>
        #             </device_info>
        #           </query_setting>
        #         </rci_reply>
        #       </device>
        #     </send_message>
        #   </sci_reply>
        xmlResponse = send_rci(rciQuery)



        # Get list of nodes for the setting
        nodeList = xmlResponse.getElementsByTagName(settingName) # return a NodeList object

        # Check if we only have a setting node
        if(nodeList.length != 1):
            log.error("Duplicated setting '%s', Response: %s" % (settingName, xmlResponse.toprettyxml(indent="  ")) )
            return None



        # Get Node for the setting, ej:
        # node.toxml():
        #  <product>default</product>
        nodeElement = nodeList[0] # return a Node object

        #Check if we have a valid element node
        if(nodeElement.nodeType != xml.dom.minidom.Node.ELEMENT_NODE):
            log.error("Wrong Element Node for setting '%s'" % (settingName) )
            return None



        # Get Text Node for the setting
        nodeText = nodeElement.firstChild # return a Text Node object

        ## Check if we have a valid Text node
        if(nodeText.nodeType != xml.dom.minidom.Node.TEXT_NODE):
            log.error("Wrong Text Node for setting '%s'" % (settingName) )
            return None


        # Return value for the setting
        return nodeText.data



    def setSettingValue(self, groupName, settingName, newValue, cache = "false"):
        # Escape new setting value to allow all kind of characters
        newValue = xml.sax.saxutils.escape(newValue)


        # Create RCI query, ej:
        # <sci_request version="1.0">
        #       <send_message cache="no">
        #           <targets>
        #               <device id="00000000-00000000-BD34D8FF-FFD8E610"/>
        #           </targets>
        #           <rci_request version="1.1">
        #               <query_setting>
        #                   <device_info/>
        #               </query_setting>
        #           </rci_request>
        #       </send_message>
        # </sci_request>
        rciQuery = RCI_BASE_TEMPLATE % ( cache, # cache
                                         ICPlugin.device_id, # Device ID
                                         SET_SETTING % (groupName, # Group
                                                        1, # Index
                                                        settingName, # setting
                                                        newValue, # new value to set
                                                        settingName, # setting
                                                        groupName, # Group
                                                        ) # RCI query
                                       )


        # Send request to server and get the response
        # Response returned by server, ej:
        # <?xml version="1.0" ?>
        #   <sci_reply version="1.0">
        #     <send_message>
        #       <device id="00000000-00000000-BD34D8FF-FFD8E610">
        #         <rci_reply version="1.1">
        #           <set_setting>
        #             <device_info>
        #               <product/>
        #             </device_info>
        #           </set_setting>
        #         </rci_reply>
        #       </device>
        #     </send_message>
        #   </sci_reply>
        xmlResponse = send_rci(rciQuery)



        # Get list of nodes for the setting
        nodeList = xmlResponse.getElementsByTagName(settingName) # return a NodeList object

        # Check if we only have a setting node
        if(nodeList.length != 1):
            if(nodeList.length >1):
                log.error("Duplicated setting '%s', Response: %s" % (settingName, xmlResponse.toprettyxml(indent="  ")) )
            elif(nodeList.length == 0):
                log.error("There is not a setting '%s', Response: %s" % (settingName, xmlResponse.toprettyxml(indent="  ")) )
            return False



        # Get Node for the setting, ej:
        # node.toxml():
        #  <product>default</product>
        nodeElement = nodeList[0] # return a Node object

        #Check if we have a valid element node
        if(nodeElement.nodeType != xml.dom.minidom.Node.ELEMENT_NODE):
            log.error("Wrong Element Node for setting '%s'" % (settingName) )
            return False


        # Get Text Node for the setting
        nodeText = nodeElement.firstChild # return a Text Node object

        ## Check if we have a valid Text node
        if(nodeText is not None):
            log.error("Wrong Text Node for setting '%s'" % (settingName) )
            return False

        return True



    def verifySettingValue(self, groupName, settingName, newValue, cache = "no"):
        # Initialize result
        result = False

        # Get original value
        originalValue = self.getSettingValue(groupName,settingName)
        log.info("Original value is '%s'" % (originalValue))
        # Check if we expect changes
        if(originalValue == newValue):
            log.warning("Original value '%s' match with the new value '%s' to be setup " % (originalValue,newValue) )

        # Set new value
        if( self.setSettingValue(groupName,settingName,newValue) ):
            # Get new value
            modifiedValue = self.getSettingValue(groupName,settingName)

            # Verify original and modified values
            if(newValue == modifiedValue):
                result = True
                log.info("New value '%s' match with obtained value '%s'" % (newValue,modifiedValue) )
            else:
                log.error("New value '%s' does not match with obtained value '%s'" % (newValue,modifiedValue) )

        return result



    def getChunks(self, string, number):
        # split a string each "number" of characters
        list = [string[i:i+number] for i in range(0,len(string),number)]

        return list



    def getListValidCharactersString(self):
        listValidCharacters = []
        for index in range(32,126):
            # We use this range of valid characters
            # see: http://www.ascii.cl/htmlcodes.htm
            listValidCharacters.append(chr(index))

        # Return a string
        return "".join(listValidCharacters)



    #**********************#
    #***** TEST_CASES *****#
    #**********************#
    def test_valid_characters_device_info_product(self):
        # Verify that Device is connected
        self.ensure_connected()

        # Generate all valid characters in lise
        lines = self.getChunks( self.getListValidCharactersString(), # return a string, ej: !"#$%&\'()*+,-.
                                64) # max length for each line

        # Set and verify setting with each line
        for eachline in lines:
            assert_equal(   True, # Expected value
                            self.verifySettingValue("device_info","product", eachline), # Returned value
                            "Verification for characters '%s' was unsuccessful." % eachline)



