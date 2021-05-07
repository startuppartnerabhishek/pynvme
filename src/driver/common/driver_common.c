/* driver.h interface implemenations that are common to SIM and PCIE-ASIC environments */

#include "driver.h"

#include "../../../spdk/lib/nvme/nvme_internal.h"

uint64_t* g_driver_config_ptr = NULL;
bool g_driver_crc32_memory_enough = false;

////module: timeval
///////////////////////////////

static struct timespec tv_diff;


void timeval_init(void)
{
  struct timespec ts;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  clock_gettime(CLOCK_MONOTONIC, &ts);

  tv_diff.tv_sec = tv.tv_sec-ts.tv_sec-1;
  tv_diff.tv_nsec = (1<<30)+tv.tv_usec*1000-ts.tv_nsec;
}


uint32_t timeval_to_us(struct timeval* t)
{
  return t->tv_sec*US_PER_S + t->tv_usec;
}


void timeval_gettimeofday(struct timeval *tv)
{
  struct timespec ts;

  assert(tv != NULL);

  // gettimeofday is affected by NTP and etc, so use clock_gettime
  clock_gettime(CLOCK_MONOTONIC, &ts);
  tv->tv_sec = ts.tv_sec+tv_diff.tv_sec;
  tv->tv_usec = (ts.tv_nsec+tv_diff.tv_nsec)>>10;
  if (tv->tv_usec > US_PER_S)
  {
    tv->tv_sec += tv->tv_usec/US_PER_S;
    tv->tv_usec = tv->tv_usec%US_PER_S;
  }
}


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

int driver_init_common(void)
{
  // init timer
  timeval_init();

  return 0;
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

int qpair_get_id(qpair_t* q)
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

void buffer_pattern_init(void *buf, size_t bytes, uint32_t ptype, uint32_t pvalue)
{
  uint32_t pattern = 0;

  if (ptype == 0)
  {
    // if pvalue is not zero, set data buffer all-one
    if (pvalue != 0)
    {
      pattern = 0xffffffff;
    }
  }
  else if (ptype == 32)
  {
    pattern = pvalue;
  }
  else if (ptype == 0xbeef)
  {
    // for random buffer, size the buffer all-zero first
    pattern = 0;
  }

  // set the buffer by 32-bit pattern
  //spdk_dma_zmalloc has set the buffer all-zero already
  if (pattern != 0)
  {
    uint32_t* ptr = buf;

    // left remaining unaligned bytes unset
    for (uint32_t i=0; i<bytes/sizeof(pattern); i++)
    {
      ptr[i] = pattern;
    }
  }

  // fill random data according to the percentage
  if (ptype == 0xbeef)
  {
    uint32_t count = 0;
    int fd = open("/dev/urandom", O_RDONLY);

    assert(pvalue <= 100);  // here needs a percentage <= 100
    count = (size_t)(bytes*pvalue/100);
    count = MIN(count, bytes);
    read(fd, buf, count);
    close(fd);
  }
   
}

int nvme_cpl_is_error(const struct spdk_nvme_cpl* cpl)
{
  return spdk_nvme_cpl_is_error(cpl);
}
