import time
import cc_testcase
import re

class ConnectionTestCase(cc_testcase.TestCase):

    def test_send_datapoints_loop(self):

        """ Sends a bunch of DataPoints for one DataStream several times in a loop while,
        also the DataPoints are verified in Device Cloud server.
        """

        self.log.info("***** Beginning DataPoints loop Test *****")


        for loop in range(0,50):

            # Wait until the group of DataPoints are sent
            result, line, readBuffer = self.deviceHandler.readUntilPattern ( pattern="END LOOP ----------------------------------------", timeout=60)

            if ( not result ):
                self.sendErrorAndCleanDataStream("Console feedback was not readed.....");

            # DataPoint structure where save the different elements
            datapointList = []


            # Find all datapoints from the console
            for index in readBuffer.findAll("^\[DataPoint\]"):
                # Initialize vars
                pointLocation = None
                pointQuality = None
                pointDescription = None
                pointData = None

                # Read the line for each DataPoint
                line = readBuffer[index]

                # Get the location
                listGroups = re.search("Location: '(.+?)'", line)
                if listGroups:
                    pointLocation = listGroups.group(1)

                # Get the quality
                listGroups = re.search("Quality: '(.+?)'", line)
                if listGroups:
                    pointQuality = listGroups.group(1)

                # Get the Description
                listGroups = re.search("Description: '(.+?)'", line)
                if listGroups:
                    pointDescription = listGroups.group(1)

                # Get the Data
                listGroups = re.search("Data: '(.+?)'", line)
                if listGroups:
                    pointData = listGroups.group(1)


                # Create new DataPoint
                dataPoint = {}
                dataPoint["location"] = pointLocation.strip()
                dataPoint["quality"] = pointQuality.strip()
                dataPoint["description"] = pointDescription.strip()
                dataPoint["data"] = pointData.strip()

                # Add DataPoint to the list
                datapointList.append(dataPoint)


            # Wait to Device Cloud to save the DataPoints
            self.log.info("Waiting 3 seconds to allow Device Cloud update the DataPoints...")
            time.sleep(3)


            numberDataPointsUploaded = len(datapointList)

            # Get the last DataPoints uploaded from Device Cloud
            result,datapointListFromDeviceCloud,requestResponse = self.cloudHandler.getDataPoints(self.device_id, "incremental", size=numberDataPointsUploaded, order="descending")

            datapointListFromDeviceCloud.reverse()

            self.log.info("Verify if DataPoints uploaded in loop %s match with the readed from Device Cloud..." % loop)

            for index in range(0,numberDataPointsUploaded-1):

                eachDatapointFromDeviceCloud = datapointListFromDeviceCloud[index]
                eachDatapointUploaded = datapointList[index]

                # Verify the Data
                if ( eachDatapointFromDeviceCloud.data != eachDatapointUploaded["data"] ):
                    self.sendErrorAndCleanDataStream("DataPoint data does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" %
                        (eachDatapointFromDeviceCloud.data, eachDatapointUploaded["data"], eachDatapointUploaded, eachDatapointFromDeviceCloud) )

                # Verify the Quality
                if ( eachDatapointFromDeviceCloud.quality != eachDatapointUploaded["quality"] ):
                    self.sendErrorAndCleanDataStream("DataPoint quality does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" %
                        (eachDatapointFromDeviceCloud.quality, eachDatapointUploaded["quality"], eachDatapointUploaded, eachDatapointFromDeviceCloud) )


                # Verify the location
                if ( eachDatapointFromDeviceCloud.location != eachDatapointUploaded["location"] ):
                    self.sendErrorAndCleanDataStream("DataPoint location does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" %
                        (eachDatapointFromDeviceCloud.location, eachDatapointUploaded["location"], eachDatapointUploaded, eachDatapointFromDeviceCloud) )



            self.log.info("%s Datapoints verified successfully....OK" % numberDataPointsUploaded)


    def sendErrorAndCleanDataStream(self, message):

        result,response = self.cloudHandler.removeDataStream(self.device_id, "incremental")
        if( not result):
            self.log.warning("Error removing the DataStream: %s" % response.content)
        else:
            self.log.info("DataStream was successfully removed!")

        self.fail(message)