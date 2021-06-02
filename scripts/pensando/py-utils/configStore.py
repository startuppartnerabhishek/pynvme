import json
import logging

__gCurrentConfig = None

__gNsvCfgCtrlrTag = "ctrlr_list"
__gNsvCfgCtrlrNsTag = "ns_list"
__gTestCfgObjectFileListTag = "objects"
__gTestCfgGlobalsFileListTag = "globals"
gCtrlrNsCountTag = "ns_count"

# example, access with rootObj = "nvme2", or "nvme1.ns2" etc.
gControllerPrefix = "nvme"
gNameSpacePrefix = "ns"
gConfigStoreGlobalsField = "globals_cfg"


# translate incoming config to a format easily usable for pynvme tests
def __translateConfigNsvTestCfg(aCfg, controllerCount):
    newConfig = {}
    ctrlrCount = controllerCount
    nsCount = int(0)

    if __gNsvCfgCtrlrTag not in aCfg:
        return newConfig, ctrlrCount

    for aCtrlr in aCfg[__gNsvCfgCtrlrTag]:
        ctrlrNodeName = gControllerPrefix + str(ctrlrCount)

        newConfig[ctrlrNodeName] = aCtrlr

        nsCount = int(0)

        if __gNsvCfgCtrlrNsTag in aCtrlr:
            for ns in aCtrlr[__gNsvCfgCtrlrNsTag]:
                nsNodeName = gNameSpacePrefix + str(nsCount)

                newConfig[ctrlrNodeName][nsNodeName] = ns

                nsCount += 1

        newConfig[ctrlrNodeName][gCtrlrNsCountTag] = nsCount

        ctrlrCount += 1

    return newConfig, ctrlrCount

def refreshObjectConfig(object_file_list):
    global __gCurrentConfig
    i = 0

    controllerCount = 0

    if (None == object_file_list):
        return

    cfg_file_list = object_file_list
    fileCount = len(cfg_file_list)

    # print("%s - %u file(s)", cfg_file_list, fileCount)

    if (fileCount <= 0):
        return

    # top level keys can be common, but internal keys should not conflict
    while i < fileCount:
        config_file = cfg_file_list[i]
        i += 1

        with open(config_file, "r") as f:
            aConfig = json.load(f)
            # print("Existing config")
            # print(__gCurrentConfig)

            # print("Got new config from file %s", config_file)
            # print(aConfig)

            translatedConfig, newControllers = __translateConfigNsvTestCfg(aConfig, controllerCount)

            controllerCount += newControllers

            # make a best effort to merge top-level config-store keys
            for k in translatedConfig:
                if k in __gCurrentConfig:
                    __gCurrentConfig[k].update(aConfig[k])
                else:
                    __gCurrentConfig[k] = translatedConfig[k]

            # print("Updated config")
            # print(__gCurrentConfig)

    # print("Refreshed with Object Config from file(s) %s, %u controllers", cfg_file_list, controllerCount)


def refreshGobalsConfig(globals_file_list):
    global __gCurrentConfig
    i = 0

    __gCurrentConfig[gConfigStoreGlobalsField] = {}

    if (None == globals_file_list):
        return

    cfg_file_list = globals_file_list
    fileCount = len(cfg_file_list)

    # print("%s - %u file(s)", cfg_file_list, fileCount)

    if (fileCount <= 0):
        return

    # top level keys can be common, but internal keys should not conflict
    while i < fileCount:
        globals_config_file = cfg_file_list[i]
        i += 1

        with open(globals_config_file, "r") as f:
            aGlobalConfig = json.load(f)

            # make a best effort to merge top-level config-store keys
            for k in aGlobalConfig:
                if k in __gCurrentConfig[gConfigStoreGlobalsField]:
                    __gCurrentConfig[gConfigStoreGlobalsField][k].update(aGlobalConfig[k])
                else:
                    __gCurrentConfig[gConfigStoreGlobalsField][k] = aGlobalConfig[k]

    # print("Refreshed with Global Config from file(s) %s", cfg_file_list)

def refreshConfig(test_config):
    global __gCurrentConfig
    global __gTestCfgObjectFileListTag
    global __gTestCfgGlobalsFileListTag

    # print("refreshConfig - Entered")
    # print(test_config)
    logging.info("refreshConfig - Entered")
    logging.info(test_config)

    __gCurrentConfig = {}

    if (__gTestCfgObjectFileListTag in test_config):
        refreshObjectConfig(test_config[__gTestCfgObjectFileListTag])

    if (__gTestCfgGlobalsFileListTag in test_config):
        refreshGobalsConfig(test_config[__gTestCfgGlobalsFileListTag])

    logging.info("Refreshed Config")
    logging.debug(__gCurrentConfig)
    # print("Refreshed Config")
    # print(__gCurrentConfig)

def getConfigDeep(node_path_list):
    global __gCurrentConfig

    value = None
    component_idx = 0
    max = len(node_path_list)
    current_node = __gCurrentConfig

    while (component_idx < max) and (None != current_node):
        nodeName = node_path_list[component_idx]

        if nodeName not in current_node:
            return value
        else:
            current_node = current_node[nodeName]

        component_idx += 1

    return current_node

def getGlobalConfigDeep(node_path_list):
    node_name_list_for_globals = [gConfigStoreGlobalsField]

    node_name_list_for_globals.extend(node_path_list)

    return getConfigDeep(node_name_list_for_globals)

def setConfigDeep(node_path_list, new_value):
    global __gCurrentConfig

    component_idx = 0
    max = len(node_path_list)
    current_node = __gCurrentConfig

    while (component_idx < max):
        nodeName = node_path_list[component_idx]

        if nodeName not in current_node:
            # assign a blank disctionary at this node
            current_node[nodeName] = {"runtime_created": 1}

        current_node = current_node[nodeName]

        component_idx += 1

    current_node = new_value

def getConfig(node_name):
    return getConfigDeep([node_name])

def compareConfig(value, root_obj, property, type="int"):
    config_value = getConfigDeep([root_obj, property])

    if (None == config_value):
        return (config_value == value)

    if (type == "int"):
        config_value = int(config_value, 0)

    return (config_value == value)


def compareConfigDeep(value, root_obj, node_name_list, type="int"):
    full_node_list = [root_obj]
    full_node_list.extend(node_name_list)
    config_value = getConfigDeep(full_node_list)

    if (None == config_value):
        return (config_value == value)

    if (type == "int"):
        config_value = int(config_value, 0)

    return (config_value == value)

def getAllConfig():
    return __gCurrentConfig