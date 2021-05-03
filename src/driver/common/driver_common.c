/* driver.h interface implemenations that are common to SIM and PCIE-ASIC environments */

#include "driver.h"

#include "../../../spdk/lib/nvme/nvme_internal.h"

uint64_t* g_driver_config_ptr = NULL;
bool g_driver_crc32_memory_enough = false;


uint64_t driver_config(uint64_t cfg_word)
{
  assert(g_driver_config_ptr != NULL);

  if (cfg_word & 1)
  {
    // enable verify, to check if it can be enabled
    if (g_driver_crc32_memory_enough != true)
    {
      cfg_word &= ~((uint64_t)1);
    }
  }

  return *g_driver_config_ptr = cfg_word;
}


uint64_t driver_config_read(void)
{
  return *g_driver_config_ptr;
}

void driver_srand(unsigned int seed)
{
  SPDK_DEBUGLOG(SPDK_LOG_NVME, "set random seed: 0x%x\n", seed);
  srandom(seed);
}

int pen_common_connectivity_check(char *src, char *dst, unsigned int count, int return_this)
{
    // #error Woo-hoo - I can see me compile

    if (count > 0)
    {
        memcpy(dst, src, count);
    }

    return return_this;
}


////module: commands name, SPDK
///////////////////////////////

static const char *
admin_opc_name(uint8_t opc)
{
  switch (opc) {
    case SPDK_NVME_OPC_DELETE_IO_SQ:
      return "Delete I/O Submission Queue";
    case SPDK_NVME_OPC_CREATE_IO_SQ:
      return "Create I/O Submission Queue";
    case SPDK_NVME_OPC_GET_LOG_PAGE:
      return "Get Log Page";
    case SPDK_NVME_OPC_DELETE_IO_CQ:
      return "Delete I/O Completion Queue";
    case SPDK_NVME_OPC_CREATE_IO_CQ:
      return "Create I/O Completion Queue";
    case SPDK_NVME_OPC_IDENTIFY:
      return "Identify";
    case SPDK_NVME_OPC_ABORT:
      return "Abort";
    case SPDK_NVME_OPC_SET_FEATURES:
      return "Set Features";
    case SPDK_NVME_OPC_GET_FEATURES:
      return "Get Features";
    case SPDK_NVME_OPC_ASYNC_EVENT_REQUEST:
      return "Asynchronous Event Request";
    case SPDK_NVME_OPC_NS_MANAGEMENT:
      return "Namespace Management";
    case SPDK_NVME_OPC_FIRMWARE_COMMIT:
      return "Firmware Commit";
    case SPDK_NVME_OPC_FIRMWARE_IMAGE_DOWNLOAD:
      return "Firmware Image Download";
    case SPDK_NVME_OPC_DEVICE_SELF_TEST:
      return "Device Self-test";
    case SPDK_NVME_OPC_NS_ATTACHMENT:
      return "Namespace Attachment";
    case SPDK_NVME_OPC_KEEP_ALIVE:
      return "Keep Alive";
    case SPDK_NVME_OPC_DIRECTIVE_SEND:
      return "Directive Send";
    case SPDK_NVME_OPC_DIRECTIVE_RECEIVE:
      return "Directive Receive";
    case SPDK_NVME_OPC_VIRTUALIZATION_MANAGEMENT:
      return "Virtualization Management";
    case SPDK_NVME_OPC_NVME_MI_SEND:
      return "NVMe-MI Send";
    case SPDK_NVME_OPC_NVME_MI_RECEIVE:
      return "NVMe-MI Receive";
    case SPDK_NVME_OPC_DOORBELL_BUFFER_CONFIG:
      return "Doorbell Buffer Config";
    case SPDK_NVME_OPC_FORMAT_NVM:
      return "Format NVM";
    case SPDK_NVME_OPC_SECURITY_SEND:
      return "Security Send";
    case SPDK_NVME_OPC_SECURITY_RECEIVE:
      return "Security Receive";
    case SPDK_NVME_OPC_SANITIZE:
      return "Sanitize";
    case SPDK_NVME_OPC_FABRIC:
      return "Fabrics Command";
    default:
      if (opc >= 0xC0) {
        return "Vendor specific";
      }
      return "Unknown";
  }
}

static const char *
io_opc_name(uint8_t opc)
{
  switch (opc) {
    case SPDK_NVME_OPC_FLUSH:
      return "Flush";
    case SPDK_NVME_OPC_WRITE:
      return "Write";
    case SPDK_NVME_OPC_READ:
      return "Read";
    case SPDK_NVME_OPC_WRITE_UNCORRECTABLE:
      return "Write Uncorrectable";
    case SPDK_NVME_OPC_COMPARE:
      return "Compare";
    case SPDK_NVME_OPC_WRITE_ZEROES:
      return "Write Zeroes";
    case SPDK_NVME_OPC_DATASET_MANAGEMENT:
      return "Dataset Management";
    case SPDK_NVME_OPC_RESERVATION_REGISTER:
      return "Reservation Register";
    case SPDK_NVME_OPC_RESERVATION_REPORT:
      return "Reservation Report";
    case SPDK_NVME_OPC_RESERVATION_ACQUIRE:
      return "Reservation Acquire";
    case SPDK_NVME_OPC_RESERVATION_RELEASE:
      return "Reservation Release";
    case SPDK_NVME_OPC_FABRIC:
      return "Fabrics Connect";
    case SPDK_NVME_OPC_ZONE_MANAGEMENT_SEND:
      return "Zone Management Send";
    case SPDK_NVME_OPC_ZONE_MANAGEMENT_RECEIVE:
      return "Zone Management Receive";
    case SPDK_NVME_OPC_ZONE_APPEND:
      return "Zone Management Append";
    default:
      if (opc >= 0x80) {
        return "Vendor specific";
      }
      return "Unknown command";
  }
}

const char* cmd_name(uint8_t opc, int set)
{
  if (set == 0)
  {
    return admin_opc_name(opc);
  }
  else if (set == 1)
  {
    return io_opc_name(opc);
  }
  else
  {
    return "Unknown command set";
  }
}

int qpair_get_id(struct spdk_nvme_qpair* q)
{
  // q NULL is admin queue
  return q ? q->id : 0;
}

bool ns_verify_enable(struct spdk_nvme_ns* ns, bool enable)
{
  crc_table_t* crc_table = (crc_table_t*)ns->crc_table;

  SPDK_INFOLOG(SPDK_LOG_NVME, "enable inline data verify: %d\n", enable);

  if (crc_table != NULL)
  {
    // crc is created, so verify is possible
    crc_table->enabled = enable;
    return true;
  }

  return false;
}