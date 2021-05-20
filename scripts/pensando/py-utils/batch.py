import sys

gBatchCfgKeyRoot = "batch_config"
gBatchCfgKeyConfigs = "configs"
gBatchCfgKeyDefault = "default_config"
gBatchCfgKeyBatchMap = "batches"

gBatchNamePrefix = "batch"


class BatchControl():
    def __init__(self, jsonConf):
        self.__currentBatch = 0
        self.__configs = []
        self.__defaultConfig = 0
        self.__batchToConfigIdMap = {}

        self.__configure(jsonConf)


    def __configure(self, jsonConf):

        rootConfig = jsonConf[gBatchCfgKeyRoot]
        
        if jsonConf[gBatchCfgKeyRoot]:
            assert rootConfig[gBatchCfgKeyConfigs], "Could not find configs"
            self.__configs = rootConfig[gBatchCfgKeyConfigs]
            assert len(self.__configs) > 0, "Need at least one config"

            if rootConfig[gBatchCfgKeyDefault]:
                self.__defaultConfig = int(rootConfig[gBatchCfgKeyDefault])
                assert self.__defaultConfig < len(self.__configs), "default config out-of-range"

            if rootConfig[gBatchCfgKeyBatchMap]:
                self.__batchToConfigIdMap = rootConfig[gBatchCfgKeyBatchMap]

    def print(self):
        print(self); sys.stdout.flush();

    def getCurrentBatchNumber(self):
        return self.__currentBatch

    def getCurrentBatch(self, bAdvance=False):
        batchName = gBatchNamePrefix + str(self.__currentBatch)
        config = None

        if None == self.__batchToConfigIdMap[batchName]:
            config = self.__configs[self.__defaultConfig]
        else:
            config = self.__configs[int(self.__batchToConfigIdMap[batchName])]

        if (bAdvance):
            self.__currentBatch += 1

        return config