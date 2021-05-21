import json
import logging

gCurrentConfig = None

def refreshConfig(config_file):
    global gCurrentConfig

    if (None == config_file):
        gCurrentConfig = None
        return

    with open(config_file, "r") as f:
        gCurrentConfig = json.load(f)

    logging.info("Refreshed with Config from file %s", config_file)
    logging.info(gCurrentConfig)

def getConfigDeep(node_path_list):
    global gCurrentConfig

    value = None
    component_idx = 0
    max = len(node_path_list)
    current_node = gCurrentConfig

    while (component_idx < max) and (None != current_node):
        nodeName = node_path_list[component_idx]

        if None == current_node[nodeName]:
            return value
        else:
            current_node = current_node[nodeName]

        component_idx += 1

    return current_node

def getConfig(node_name):
    return getConfigDeep([node_name])

def compareConfig(value, root_obj, property, type="int"):
    config_value = getConfigDeep([root_obj, property])

    if (None == config_value):
        return (config_value == value)

    if (type == "int"):
        config_value = int(config_value)

    return (config_value == value)


def compareConfigDeep(value, root_obj, node_name_list, type="int"):
    full_node_list = [root_obj]
    full_node_list.extend(node_name_list)
    config_value = getConfigDeep(full_node_list)

    if (None == config_value):
        return (config_value == value)

    if (type == "int"):
        config_value = int(config_value)

    return (config_value == value)
