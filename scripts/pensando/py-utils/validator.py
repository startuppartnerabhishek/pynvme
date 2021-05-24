import logging

import configStore as CfgStore
import cParser as Parser

class Field:
    def __init__(self, name, nodeInCfg, nodeInSpec):
        self.structNodeInCfg = nodeInCfg
        self.pathToCfgField = nodeInCfg.split(".")
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

    def __repr__(self):
        myself = "{ " + self.name + "(" + self.type + ") " + "spec @" + self.structNodeInSpec + ", cfg @" + self.structNodeInCfg + "}"
        return myself

class FieldString(Field):
    def __init__(self, name, nodeInCfg, nodeInSpec):
        super().__init__(name, nodeInCfg, nodeInSpec)
        self.type = "str"

    def compare(self, rootObj, compareWith):
        specValue = self.getSpecField(compareWith)
        fullPathAsList = rootObj.split('.') # rootObj can itself be a path, e.g. nvme0.nn0
        fullPathAsList.extend(self.pathToCfgField)

        # convert the config value (string) to an array of ascii codes, before comarison
        cfgValue = list( map( ord, (CfgStore.getConfigDeep(fullPathAsList))))

        if None == cfgValue:
            # value not found in store, don't fail the test
            return

        shortenedCompvalue = specValue[0:len(cfgValue)]

        if shortenedCompvalue != cfgValue:
            logging.error("Object %s, field %s, path %s, spec-field %s type STR", rootObj, self.name, self.structNodeInCfg, self.structNodeInSpec)
            logging.error("Expected (and expected as byte-array)")
            logging.error(CfgStore.getConfigDeep(fullPathAsList))
            logging.error(cfgValue)
            logging.error("Got (raw, compirson-range)")
            logging.error(specValue)
            logging.error(shortenedCompvalue)
            assert False


class FieldInt(Field):
    def __init__(self, name, nodeInCfg, nodeInSpec):
        super().__init__(name, nodeInCfg, nodeInSpec)
        self.type = "int"

    def compare(self, rootObj, compareWith):
        specValue = self.getSpecField(compareWith)
        fullPathAsList = rootObj.split('.') # rootObj can itself be a path, e.g. nvme0.nn0
        fullPathAsList.extend(self.pathToCfgField)

        cfgValue = int(CfgStore.getConfigDeep(fullPathAsList))

        if None == cfgValue:
            # value not found in store, don't fail the test
            return

        if cfgValue != specValue:
            logging.error("Object %s, field %s, path %s, spec-field %s type INT", rootObj, self.name, self.structNodeInCfg, self.structNodeInSpec)
            logging.error("Expected")
            logging.error(cfgValue)
            logging.error("Got")
            logging.error(specValue)
            assert False

class StructValidator:
    def __init__(self, fieldList, specStructName):
        self.allFields = fieldList
        self.specStruct = specStructName

    def compare(self, rootObj, compareWithRawBuf):
        structDict = Parser.struct_parse_as_dict(compareWithRawBuf[0:Parser.struct_size(self.specStruct)], self.specStruct)

        logging.info("%s validator with field list [%s]", self.specStruct, self.allFields)

        for field in self.allFields:
            field.compare(rootObj, structDict)

    def addFieldForComparison(self, newField):
        self.allFields.append(newField)

# instantiate controllers
gControllerValidator = StructValidator([
    FieldInt("Vendor Id", "vid", "vid"),
    FieldString("Serial Number", "serial", "sn")
], "spdk_nvme_ctrlr_data")

# external API
def controllerValidateResponse(rootObj, rawBuf):
    gControllerValidator.compare(rootObj, rawBuf)

def controllerAddValidatorField(friendly_name, cfg_field_name, spec_field_name, type="int"):
    new_field = None
    if (type == "int"):
        new_field = FieldInt(friendly_name, cfg_field_name, spec_field_name)
    else:
        new_field = FieldString(friendly_name, cfg_field_name, spec_field_name)

    gControllerValidator.addFieldForComparison(new_field)

# set any cfgField
def setCfgField(rootObj, pathToCfgField, newValue):
    pathAsList = rootObj.split(".")
    pathAsList.extend(pathToCfgField)

    CfgStore.setConfigDeep(pathAsList, newValue)