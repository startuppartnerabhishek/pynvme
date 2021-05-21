import logging

import configStore as CfgStore
import cParser as Parser

class Field:
    def __init__(self, name, nodeInCfg, nodeInSpec):
        self.structNodeInCfg = nodeInCfg
        self.structNodeInSpec = nodeInSpec
        self.pathToSpecField = nodeInSpec.split('.')
        self.name = name

    def compare(self, rootObj, compareWith):
        pass

    def getSpecField(self, specStructAsDict):
        nodes = 0
        max = len(self.pathToSpecField)
        currDict = specStructAsDict

        assert currDict != None, "input dictionary cannot be None"

        while (nodes < max):
            nodeName = self.pathToSpecField[nodes]

            nodes += 1
            currDict = currDict[nodeName]

            if (nodes < max):
                assert currDict, nodeName + " not found in path to " + self.name
            else:
                return currDict

        assert False, "Unexpected code path"


class FieldString(Field):
    def compare(self, rootObj, compareWith):
        specValue = self.getSpecField(compareWith)

        # convert the config value (string) to an array of ascii codes, before comarison
        cfgValue = list( map( ord, (CfgStore.getConfigDeep([rootObj, self.structNodeInCfg]))))

        if None == cfgValue:
            # value not found in store, don't fail the test
            return

        shortenedCompvalue = specValue[0:len(cfgValue)]

        if shortenedCompvalue != cfgValue:
            logging.error("Object %s, field %s, path %s, spec-field %s type STR", rootObj, self.name, self.structNodeInCfg, self.structNodeInSpec)
            logging.error("Expected (and expected as byte-array)")
            logging.error(CfgStore.getConfigDeep([rootObj, self.structNodeInCfg]))
            logging.error(cfgValue)
            logging.error("Got")
            logging.error(compareWith[self.structNodeInSpec])
            assert False


class FieldInt(Field):
    def compare(self, rootObj, compareWith):
        specValue = self.getSpecField(compareWith)
        cfgValue = int(CfgStore.getConfigDeep([rootObj, self.structNodeInCfg]))

        if None == cfgValue:
            # value not found in store, don't fail the test
            return

        if cfgValue != specValue:
            logging.error("Object %s, field %s, path %s, spec-field %s type STR", rootObj, self.name, self.structNodeInCfg, self.structNodeInSpec)
            logging.error("Expected")
            logging.error(CfgStore.getConfigDeep([rootObj, self.structNodeInCfg]))
            logging.error("Got")
            logging.error(specValue)
            assert False

class StructValidator:
    def __init__(self, fieldList, specStructName):
        self.allFields = fieldList
        self.specStruct = specStructName

    def compare(self, rootObj, compareWithRawBuf):
        structDict = Parser.struct_parse_as_dict(compareWithRawBuf[0:Parser.struct_size(self.specStruct)], self.specStruct)

        for field in self.allFields:
            field.compare(rootObj, structDict)

# instantiate controllers
gControllerValidator = StructValidator([
    FieldInt("Vendor Id", "vid", "vid"),
    FieldString("Serial Number", "serial", "sn")
], "spdk_nvme_ctrlr_data")

# external API
def validateIdControllerResponse(rootObj, rawBuf):
    gControllerValidator.compare(rootObj, rawBuf)