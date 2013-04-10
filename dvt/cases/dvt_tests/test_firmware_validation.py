import os
import time
import ic_testcase
import xml.dom.minidom

from ..utils import update_firmware

firmware_file = 'dvt/cases/test_files/firmware.bin'
target_file = 'firmware.bin'

class FirmwareValidTestCase(ic_testcase.TestCase):
    """ Performs a series of tests against several different firmware targets
    and verifies that the correct error response is returned.
    """

    def test_download_validation(self):
        self.log.info("Beginning Test - test_download_validation")
        # must be 12 to start writing a file
        target = 12

        # send request to update firmware
        self.log.info("Sending request to update firmware.")
        response = update_firmware(self.session, self.hostname, self.device_id,
            firmware_file, "%d" % target)

        # print response
        self.log.info("response: \n%s" % response)

        expected_file = open(firmware_file, "r")
        expected_content = expected_file.read()
        expected_file.close()

        read_file = open(target_file, "r")
        read_content = read_file.read()
        read_file.close()

        os.remove(target_file)

        self.log.info("Verifying the image data.")
        self.assertEqual(expected_content, read_content)

if __name__ == '__main__':

    unittest.main()
