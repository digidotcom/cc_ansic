import time
import ic_testcase

from ..utils import DeviceConnectionMonitor, device_is_connected



DISCONNECT_REQUEST = \
"""<sci_request version="1.0">
    <disconnect>
        <targets>
            <device id="%s"/>
        </targets>
    </disconnect>
</sci_request>"""

class DisconnectTestCase(ic_testcase.TestCase):

    def test_disconnect(self):

        """ Sends disconnect request to given device and verifies that
        the device disconnects and reconnects to an iDigi server.
        """

        self.log.info("***** Beginning Connect and Disconnect Test *****")
        monitor = DeviceConnectionMonitor(self.push_client, self.dev_id)

        try:
            monitor.start()
            if device_is_connected(self) == False:
                self.log.info("Waiting for device connected before starting the test")
                monitor.wait_for_connect(30)
            
            self.log.info("Device is connected. Start testing.")
            self.log.info("Sending Connection Control Disconnect to %s." % self.device_id)

            # Create disconnect request
            disconnect_request = DISCONNECT_REQUEST % (self.device_id)

            # Send SCI disconnect request
            self.session.post('http://%s/ws/sci' % self.hostname,
                data=disconnect_request).content


            self.log.info("Waiting for iDigi to disconnect device.")
            monitor.wait_for_disconnect(30)
            self.log.info("Monitor: Device is disconnected.")

            self.log.info("Waiting for Device to reconnect.")
            monitor.wait_for_connect(30)
            self.log.info("Monitor: Device is connected.")

            #this set the test result as failed
            #self.fail("Reason for the test fail")
        finally:
            monitor.stop()

if __name__ == '__main__':
    unittest.main()
