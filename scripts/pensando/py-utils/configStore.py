import json
import logging

gCurrentConfig = None
gConfigFileListSeparator = ';'

def refreshConfig(config_files):
    global gCurrentConfig
    global gConfigFileListSeparator
    i = 0

    gCurrentConfig = None

    if (None == config_files):
        return

    cfg_file_list = config_files.split(gConfigFileListSeparator)
    fileCount = len(cfg_file_list)

    # print("%s - %u file(s)", cfg_file_list, fileCount)

    if (fileCount <= 0):
        return

    gCurrentConfig = {}

    # top level keys can be common, but internal keys should not conflict
    while i < fileCount:
        config_file = cfg_file_list[i]
        i += 1

        with open(config_file, "r") as f:
            aConfig = json.load(f)
            # print("Existing config")
            # print(gCurrentConfig)

            # print("Got new config from file %s", config_file)
            # print(aConfig)

            for k in aConfig:
                if k in gCurrentConfig:
                    gCurrentConfig[k].update(aConfig[k])
                else:
                    gCurrentConfig[k] = aConfig[k]

            # print("Updated config")
            # print(gCurrentConfig)

    logging.info("Refreshed with Config from file(s) %s", cfg_file_list)
    logging.info(gCurrentConfig)

def getConfigDeep(node_path_list):
    global gCurrentConfig

    value = None
    component_idx = 0
    max = len(node_path_list)
    current_node = gCurrentConfig

    while (component_idx < max) and (None != current_node):
        nodeName = node_path_list[component_idx]

        if nodeName not in current_node:
            return value
        else:
            current_node = current_node[nodeName]

        component_idx += 1

    return current_node

def setConfigDeep(node_path_list, new_value):
    global gCurrentConfig

    component_idx = 0
    max = len(node_path_list)
    current_node = gCurrentConfig

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
    return gCurrentConfig