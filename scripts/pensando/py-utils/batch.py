import sys
import logging

gBatchCfgKeyRoot = "batch_config"
gBatchCfgKeyConfigs = "configs"
gBatchCfgKeyDefault = "default_config"
gBatchCfgKeyBatchMap = "batches"
gBatchCfgFinalCleanup = "test_final_cleanup"
gBatchCfgInitialSetup = "test_initial_setup"

gBatchNamePrefix = "batch"


class BatchControl():
    def __init__(self, jsonConf):
        self.__clean_slate()

        self.__configure(jsonConf)

    def __clean_slate(self):
        self.__currentBatch = -1
        self.__configs = []
        self.__defaultConfigIdx = 0
        self.__batchToConfigIdMap = {}
        self.__finalCleanup = None
        self.__initialSetup = None

    def __configure(self, jsonConf):

        self.__clean_slate()

        rootConfig = jsonConf[gBatchCfgKeyRoot]
        
        if gBatchCfgKeyRoot in jsonConf:
            assert rootConfig[gBatchCfgKeyConfigs], "Could not find configs"
            self.__configs = rootConfig[gBatchCfgKeyConfigs]
            assert len(self.__configs) > 0, "Need at least one config"

            if gBatchCfgKeyDefault in rootConfig:
                self.__defaultConfigIdx = int(rootConfig[gBatchCfgKeyDefault])
                assert self.__defaultConfigIdx < len(self.__configs), "default config out-of-range"

            if gBatchCfgKeyBatchMap in rootConfig:
                self.__batchToConfigIdMap = rootConfig[gBatchCfgKeyBatchMap]

            if gBatchCfgFinalCleanup in rootConfig:
                self.__finalCleanup = rootConfig[gBatchCfgFinalCleanup]

            if gBatchCfgInitialSetup in rootConfig:
                self.__initialSetup = rootConfig[gBatchCfgInitialSetup]

    def print(self):
        print(self); sys.stdout.flush();

    def getCurrentBatchNumber(self):
        return self.__currentBatch

    def getCurrentBatch(self, bAdvance=False):
        batchName = gBatchNamePrefix + str(self.__currentBatch)
        config = None

        if (self.__currentBatch < 0):
            return None

        if batchName not in self.__batchToConfigIdMap:
            logging.info("No explicit mapping, using default %u", int(self.__defaultConfigIdx))
            config = self.__configs[int(self.__defaultConfigIdx)]
        else:
            logging.info("Found explicit mapping, using batch %u", int(self.__batchToConfigIdMap[batchName]))
            config = self.__configs[int(self.__batchToConfigIdMap[batchName])]

        if (bAdvance):
            self.__currentBatch += 1

        return config

    def moveToNextBatch(self):
        self.__currentBatch += 1
        return self.getCurrentBatch(False)

    def getFinalCleanup(self):
        return self.__finalCleanup

    def getInitialSetup(self):
        return self.__initialSetup