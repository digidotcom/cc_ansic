# -*- coding: utf-8 -*-
import time
import ic_testcase
import datetime
import re
import os
import base64
import xml.dom.minidom
from time import sleep

from random import randint
from xml.dom.minidom import getDOMImplementation
impl = getDOMImplementation()

from ..utils import getText
from ..utils import DeviceConnectionMonitor

MAX_TEST_FILE_SIZE = 30
dummyFsize = 4
expectError = True
noCleanup = False
doTruncate = True

rebootRequest = \
    """<sci_request version="1.0">
        <reboot>
            <targets>
                <device id="%s"/>
            </targets>
        </reboot>
    </sci_request>"""

noFileRequest = \
    """<sci_request version="1.0">
      <file_system>
        <targets>
          <device id="%s"/>
        </targets>
        <commands>
            <get_file path="ThisFileDoesNotExist" offset = "0" length = "10"/>
        </commands>
      </file_system>
      </sci_request>"""

fileGetRequest = \
    """<sci_request version="1.0">
      <file_system>
        <targets>
          <device id="%s"/>
        </targets>
        <commands>
            <get_file path="%s" offset = "%d" length = "%d"/>
        </commands>
      </file_system>
      </sci_request>"""

filePutRequest = \
        """<sci_request version="1.0">
          <file_system>
            <targets>
              <device id="%s"/>
            </targets>
            <commands>
               <put_file path="%s" offset = "%d" truncate = "%r">
                  <data>%s</data>
               </put_file>
            </commands>
          </file_system>
          </sci_request>"""

fileRmRequest = \
    """<sci_request version="1.0">
      <file_system>
        <targets>
          <device id="%s"/>
        </targets>
        <commands>
          <rm path="%s"/>
        </commands>
      </file_system>
    </sci_request>"""

filelsRequest = \
    """<sci_request version="1.0">
      <file_system>
        <targets>
          <device id="%s"/>
        </targets>
        <commands>
          <ls path="%s" hash="%s"/>
        </commands>
      </file_system>
    </sci_request>"""

class FileSystemDirTestCase(ic_testcase.TestCase):

    out_size = 0
    outData = ""
    #monitor = None

    def setUp(self):
        ic_testcase.TestCase.setUp(self)

        FileData = ""
        for i in xrange(MAX_TEST_FILE_SIZE):
            FileData += chr(randint(0, 255))
        self.out_size = len(FileData)
        self.outData = base64.encodestring(FileData)
    
    @classmethod
    def tearDownClass(cls):
        ic_testcase.TestCase.tearDownClass()

    def test_os01_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_malloc_abort1", "./test.py")

    def test_os02_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_malloc_abort2", "./test.py")

    def test_os04_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_free_abort1", "./test.py")

    def test_os05_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_free_abort2", "./test.py")

    def test_os06_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_sysuptime_abort", "./test.py")

    def test_os07_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_ls_abort("dvt_os_yeld_abort", "./test.py")

    def test_os08_abort(self):
        """ Tests abort conditions in os callbacks """
        self.check_reboot_abort("dvt_os_reboot_abort")

    def test_os09_busy(self):
        """ Tests busy return in os callbacks """
        self.check_reboot("dvt_os_reboot_busy")

    def test_os10_busy(self):
        """ Tests busy return in os callbacks """
        self.check_ls("dvt_os_malloc_null2", "./test.py")
        self.check_ls("dvt_os_malloc_busy2", "./test.py")
        self.log.info("Test success!")

#--------- 

    def verify_no_file(self, fpath):
        getRequest = fileGetRequest % (self.device_id, fpath, 0, dummyFsize)
        response = self.session.post('http://%s/ws/sci' % self.hostname, data=getRequest).content
        if (response.find('error id="1"') == -1):
            self.assertTrue(False, "Expected: No such file or directory, response[%s]" %(response))


    def verify_file_request_abort(self, request):
        abortExpected = True

        response = self.session.post('http://%s/ws/sci' % self.hostname, data=request).content
        self.log.info("Abort response %s" % response)

        aborted = False
        if ((response.find('error id="2001"') != -1) or (response.find('error id="2006"') != -1)):
            aborted = True

        self.assertTrue(aborted, "Abort expected, got response[%s]" %(response))

        response = ""
        sync_failed = True
        no_file_request = noFileRequest % (self.device_id)
        for x in xrange(1, 20):
            response = self.session.post('http://%s/ws/sci' % self.hostname, data=no_file_request).content
            if (response.find('error id="2001"') == -1):
                sync_failed = False
                break
            time.sleep(1)

        self.log.info("Sync after abort %d times" % x)
        if (response.find('error id="1"') == -1):
            self.log.info("Response to sync after abort %s" % response)
        
        self.assertTrue(sync_failed == False, "**********  DEVICE STILL NOT CONNECTED after 20 sec  **********, response[%s]" %(response))

    def verify_reboot_abort(self, request):
        abortExpected = True

        monitor = DeviceConnectionMonitor(self.push_client, self.dev_id)
        monitor.start()

        response = self.session.post('http://%s/ws/sci' % self.hostname, data=request).content
        self.log.info("Abort response %s" % response)

        aborted = False
        if ((response.find('submitted') != -1) or (response.find('error id="2001"') != -1) or (response.find('error id="2006"') != -1)):
            aborted = True
                        
        self.assertTrue(aborted, "Abort expected, got response[%s]" %(response))                                                                                                                                                                                                                                      
        if (response.find('submitted') != -1):
            monitor.wait_for_disconnect(30)
        monitor.wait_for_connect(30)
        monitor.stop()

        response = ""
        sync_failed = True
        no_file_request = noFileRequest % (self.device_id)
        for x in xrange(1, 20):
            response = self.session.post('http://%s/ws/sci' % self.hostname, data=no_file_request).content
            if (response.find('error id="2001"') == -1):
                sync_failed = False
                break
            time.sleep(1)

        self.log.info("Sync after abort %d times" % x)
        if (response.find('error id="1"') == -1):
            self.log.info("Response to sync after abort %s" % response)

        self.assertTrue(sync_failed == False, "**********  DEVICE STILL NOT CONNECTED after 20 sec  **********, response[%s]" %(response))




    def check_ls(self, test_name, ls_path, errorExpected = False, hashAlgo = "none"):
        lsRequest = filelsRequest % (self.device_id, ls_path, hashAlgo)

        self.log.info("test %s, ls request:\"%s\", and hash:%s" % (test_name, ls_path, hashAlgo))

        self.verify_no_file(test_name)
        response = self.session.post('http://%s/ws/sci' % self.hostname, data=lsRequest).content
        
        self.verify_device_response(response, errorExpected)
        return self.parse_ls(response)

    def check_ls_abort(self, test_name, ls_path, hashAlgo = "none"):
        lsRequest = filelsRequest % (self.device_id, ls_path, hashAlgo)

        self.log.info("test %s, ls request:\"%s\", and hash:%s" % (test_name, ls_path, hashAlgo))

        self.verify_no_file(test_name) 
        self.verify_file_request_abort(lsRequest)

    def check_get_abort(self, test_name):
        getRequest = fileGetRequest % (self.device_id, test_name, 0, 4)

        self.log.info("test %s, get request" % test_name)

        self.verify_file_request_abort(getRequest)


    def check_reboot_abort(self, test_name):
        reboot_request = rebootRequest % (self.device_id)

        self.log.info("test %s" % test_name)

        self.verify_no_file(test_name) 
        self.verify_reboot_abort(reboot_request)


    def check_reboot(self, test_name, errorExpected = False):
        reboot_request = rebootRequest % (self.device_id)

        self.log.info("test %s" % test_name)
 
        self.verify_no_file(test_name) 
        monitor = DeviceConnectionMonitor(self.push_client, self.dev_id)
        monitor.start()

        response = self.session.post('http://%s/ws/sci' % self.hostname, data=reboot_request).content
        self.log.info("Response %s" % response)
        self.assertTrue(response.find("submitted") != -1, "Response to Reboot %s" % response)

        monitor.wait_for_disconnect(30)
        monitor.wait_for_connect(30)
        monitor.stop()

    def verify_device_response(self, response, errorExpected):
        # validate response?

        if (response.find('<error') == -1):
            errorActual = False
            errorString = "Expected error, but got success"
        else:
            errorActual = True
            errorString = "Expected success, but got error"

        self.assertTrue(errorExpected == errorActual, "%s, response[%s]" %(errorString, response))

    def verify_device_response_text(self, response, text, actionExpected, text_yes, text_no):
        # validate response?

        if (response.find(text) == -1):
            actionActual = False
            errorString = text_no
        else:
            actionActual = True
            errorString = text_yes

        self.log.info("Response: %s" % response)

        self.assertTrue(actionExpected == actionActual, "%s, response[%s]" %(errorString, response))

    def get_obj_from_slice(self, segment):
        obj = []

        path_re = re.compile('path="([\.\w/-]+)"')
        last_re = re.compile('last_modified="([\.\w/-]+)"')
        size_re = re.compile('size="([\.\w/-]+)"')
        hash_re = re.compile('hash="([\.\w/-]+)"')

        pm = path_re.search(segment)
        obj.append((pm and pm.group(1)) or None)

        lm = last_re.search(segment)
        obj.append((lm and lm.group(1)) or None)

        sm = size_re.search(segment)
        obj.append((sm and sm.group(1)) or None)

        hm = hash_re.search(segment)
        obj.append((hm and hm.group(1)) or None)

        return obj

    def get_line_indexes(self, resp, starter):
        index = resp.find(starter)
        if index == -1:
            return None, None
        end_index = resp[index:].find('/>')
        self.assertTrue(end_index != -1, "Failed to parse out file from current response: %s" %resp)

        end_index += index

        return index, end_index

    def parse_ls(self, resp):
        file_list = []
        dir_list = []
        tmp_resp = resp
        while 1:
            index, end_index = self.get_line_indexes(tmp_resp, '<file ')
            if index is None:
                break

            segment = tmp_resp[index:end_index]
            obj = self.get_obj_from_slice(segment)
            file_list.append(obj)
            tmp_resp = tmp_resp[end_index:]

        tmp_resp = resp
        while 1:
            index, end_index = self.get_line_indexes(tmp_resp, '<dir')
            if index is None:
                break

            segment = tmp_resp[index:end_index]
            obj = self.get_obj_from_slice(segment)
            dir_list.append(obj)
            tmp_resp = tmp_resp[end_index:]

        self.log.info("Files: %d Dirs: %d parsed" %(len(file_list), len(dir_list)))
        return file_list, dir_list


if __name__ == '__main__':
    unittest.main() 
