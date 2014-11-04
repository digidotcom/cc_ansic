# -*- coding: iso-8859-1 -*-
import cc_testcase
import requests.exceptions

import string           # Provides a number of useful constants and classes for strings
import random           # Implements pseudo-random number generators for various distributions
import re


def generateRandomString(length):
    charList = [random.choice(string.ascii_letters + string.digits) for n in xrange(length)]
    randomString = "".join(charList)
    return randomString


class DatapointDvtTestCase(cc_testcase.TestCase):


    def test_01_datapoint_send_one_datastream_with_one_datapoint_type_integer(self):
        """ Sends a put request to upload zero bytes data with the overwrite option.
        Verifies the file is generated with an empty content.
        """


        for i in range(1,5):
            # Initialize vars
            target = "test_datapoint_send_datastream_with_datapoints"
            payload = "1;1;%s;Integer;" % i # num of loops, num of DataStreams, num of DataPoints, type of values

            # Initiate action on device
            self.sendDeviceRequestToInitiateAction(target, payload)

            for j in range(0,1):
                # Read the datapoints
                datapointList = self.readDataPointsFromConsole()


                # Verify the DataPoints in Device Cloud
                if ( not self.verifyDataPointsInDeviceCloud(datapointList) ):
                    self.fail("Error with the DataPoints")


            # Wait until the group of DataPoints are sent
            result, line, readBuffer = self.deviceHandler.readUntilPattern ( pattern="Finished the Thread for", timeout=60)

            if ( not result ):
                self.sendErrorAndCleanDataStream("Console feedback was not readed.....");
            else:
                self.log.info("Console feedback for the end of the thread execution readed!!")



    def verifyDataPointsInDeviceCloud(self, datapointList):

        numberDataPointsUploaded = len(datapointList)
        retryCounter = 0
        message = ""

        # Try to get the DataPoints with a retry system due to sometimes device cloud returns an error
        while ( retryCounter < 3 ):
            verificationResult = True
            retryCounter+=1;

            # Get the last DataPoints uploaded from Device Cloud
            result,datapointListFromDeviceCloud,requestResponse = self.cloudHandler.getDataPoints(self.device_id, "incremental", size=numberDataPointsUploaded, order="descending")

            if ( not result ):
                message = "Error getting the DataPoints from device cloud: %s" % requestResponse.content
                self.log.error(message)
                time.sleep(1)
                continue # Go to the next loop

            datapointListFromDeviceCloud.reverse()

            self.log.info("Verify if DataPoints uploaded in match with the readed from Device Cloud...")

            for index in range(0,numberDataPointsUploaded-1):
                # Get DataPoint readed from console and DataPoint from Device Cloud
                eachDatapointFromDeviceCloud = datapointListFromDeviceCloud[index]
                eachDatapointUploaded = datapointList[index]

                # Verify the Data
                if ( eachDatapointFromDeviceCloud.data != eachDatapointUploaded["data"] ):
                    message = "DataPoint data does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" % (eachDatapointFromDeviceCloud.data, eachDatapointUploaded["data"], eachDatapointUploaded, eachDatapointFromDeviceCloud)
                    self.log.warning(message)
                    verificationResult = False
                    break

                # Verify the Quality
                if ( eachDatapointFromDeviceCloud.quality != eachDatapointUploaded["quality"] ):
                    message = "DataPoint quality does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" % (eachDatapointFromDeviceCloud.quality, eachDatapointUploaded["quality"], eachDatapointUploaded, eachDatapointFromDeviceCloud)
                    self.log.warning(message)
                    verificationResult = False
                    break

                # Verify the location
                if ( eachDatapointFromDeviceCloud.location != eachDatapointUploaded["location"] ):
                    message = "DataPoint location does not match!!! readed: '%s' != expected: '%s'\n Original DataPoint: %s\n Readed DataPoint: %s\n" % (eachDatapointFromDeviceCloud.location, eachDatapointUploaded["location"], eachDatapointUploaded, eachDatapointFromDeviceCloud)
                    self.log.warning(message)
                    verificationResult = False
                    break


            # All DataPoints were verified
            if ( verificationResult ):
                message = "%s Datapoints verified successfully....OK" % (numberDataPointsUploaded)
                break
            else:
                self.log.info("Retry system to verify the data from Device Cloud...(%s)" % retryCounter)
                time.sleep(1)


        # Show the test result
        if ( verificationResult ):
            self.log.info(message)
            return True
        else:
            self.log.error(message)
            return False



    def readDataPointsFromConsole(self):

            # Wait until the group of DataPoints are sent
            result, line, readBuffer = self.deviceHandler.readUntilPattern ( pattern="END LOOP ----------------------------------------", timeout=60)

            if ( not result ):
                self.sendErrorAndCleanDataStream("Console feedback for the end of the loop readed.....");

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

            return datapointList




    def sendDeviceRequestToInitiateAction(self, target, payload, expectedResponse="Launch successful"):

        # Send Device Request to the target to initiate action
        status, response = self.cloudHandler.sendDeviceRequest(self.device_id, target, payload)

        if(status):
            # Status 200. Checking the received response
            responseText = response.resource["sci_reply"]["data_service"]["device"]["requests"]["device_request"]["#text"]
            if(responseText == expectedResponse):
                self.log.info("Received the expected response\n")
            else:
                self.fail("Received response from device: \"%s\" is not the expected\n" % responseText)
        else:
            self.log.error("Response content from device: %s" % response.content)
            self.fail("Incorrect response code: %d" % response.status_code)


    def sendErrorAndCleanDataStream(self, message):

        result,response = self.cloudHandler.removeDataStream(self.device_id, "incremental")
        if( not result):
            self.log.warning("Error removing the DataStream: %s" % response.content)
        else:
            self.log.info("DataStream was successfully removed!")

        self.fail(message)
