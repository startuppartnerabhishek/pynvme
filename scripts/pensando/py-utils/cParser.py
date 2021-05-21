import os
import pycstruct

# See https://pypi.org/project/pycstruct/

###############################


"""
import pycstruct

definitions = pycstruct.parse_file('simple_example.c')

with open('simple_example.dat', 'rb') as f:
    inbytes = f.read()

# Dictionary representation
result = definitions['person'].deserialize(inbytes)
print(str(result))

# Alternative, Instance representation
instance = definitions['person'].instance(inbytes)

"""

###############################

gThisModulePath = os.path.dirname(os.path.realpath(__file__))
gSpecHeader = gThisModulePath + "/nvme_parser_spec.h"

gParserDefinitions = pycstruct.parse_file(gSpecHeader);

def struct_parse(buffer, typeName):
    instance = gParserDefinitions[typeName].instance(buffer)
    return instance

def struct_list():
    results = []

    for k in gParserDefinitions:
        results.extend(k)

    return results

def struct_size(typeName):
    return gParserDefinitions[typeName].size()