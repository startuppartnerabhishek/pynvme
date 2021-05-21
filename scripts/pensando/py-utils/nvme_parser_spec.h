#ifndef __NVME_PARSER_SPEC_H__
#define __NVME_PARSER_SPEC_H__

#include <stdint.h>

/*********************************************************


 * A header file created for use by the spec-parser. Not used as
 * part of compilation.
 * Contents are pulled over from nvme_spec.h as needed.
 *
 * 
 * 
 * ***********************************************************/


#define SPDK_NVME_CTRLR_SN_LEN	20
#define SPDK_NVME_CTRLR_MN_LEN	40
#define SPDK_NVME_CTRLR_FR_LEN	8


enum spdk_nvme_sgl_descriptor_type {
	SPDK_NVME_SGL_TYPE_DATA_BLOCK		= 0x0,
	SPDK_NVME_SGL_TYPE_BIT_BUCKET		= 0x1,
	SPDK_NVME_SGL_TYPE_SEGMENT		= 0x2,
	SPDK_NVME_SGL_TYPE_LAST_SEGMENT		= 0x3,
	SPDK_NVME_SGL_TYPE_KEYED_DATA_BLOCK	= 0x4,
	SPDK_NVME_SGL_TYPE_TRANSPORT_DATA_BLOCK	= 0x5,
	/* 0x6 - 0xE reserved */
	SPDK_NVME_SGL_TYPE_VENDOR_SPECIFIC	= 0xF
};

enum spdk_nvme_sgl_descriptor_subtype {
	SPDK_NVME_SGL_SUBTYPE_ADDRESS		= 0x0,
	SPDK_NVME_SGL_SUBTYPE_OFFSET		= 0x1,
	SPDK_NVME_SGL_SUBTYPE_TRANSPORT		= 0xa,
};

struct __attribute__((packed)) spdk_nvme_sgl_descriptor {
	uint64_t address;
	union {
		struct {
			uint8_t reserved[7];
			uint8_t subtype	: 4;
			uint8_t type	: 4;
		} generic;

		struct {
			uint32_t length;
			uint8_t reserved[3];
			uint8_t subtype	: 4;
			uint8_t type	: 4;
		} unkeyed;

		struct {
			uint64_t length		: 24;
			uint64_t key		: 32;
			uint64_t subtype	: 4;
			uint64_t type		: 4;
		} keyed;
	};
};

struct spdk_nvme_cmd {
	/* dword 0 */
	uint16_t opc	:  8;	/* opcode */
	uint16_t fuse	:  2;	/* fused operation */
	uint16_t rsvd1	:  4;
	uint16_t psdt	:  2;
	uint16_t cid;		/* command identifier */

	/* dword 1 */
	uint32_t nsid;		/* namespace identifier */

	/* dword 2-3 */
	uint32_t rsvd2;
	uint32_t rsvd3;

	/* dword 4-5 */
	uint64_t mptr;		/* metadata pointer */

	/* dword 6-9: data pointer */
	union {
		struct {
			uint64_t prp1;		/* prp entry 1 */
			uint64_t prp2;		/* prp entry 2 */
		} prp;

		struct spdk_nvme_sgl_descriptor sgl1;
	} dptr;

	/* dword 10-15 */
	uint32_t cdw10;		/* command-specific */
	uint32_t cdw11;		/* command-specific */
	uint32_t cdw12;		/* command-specific */
	uint32_t cdw13;		/* command-specific */
	uint32_t cdw14;		/* command-specific */
	uint32_t cdw15;		/* command-specific */
};

struct spdk_nvme_status {
	uint16_t p	:  1;	/* phase tag */
	uint16_t sc	:  8;	/* status code */
	uint16_t sct	:  3;	/* status code type */
	uint16_t rsvd2	:  2;
	uint16_t m	:  1;	/* more */
	uint16_t dnr	:  1;	/* do not retry */
};

/**
 * Completion queue entry
 */
struct spdk_nvme_cpl {
	/* dword 0 */
	uint32_t		cdw0;	/* command-specific */

	/* dword 1 */
	uint32_t		rsvd1;

	/* dword 2 */
	uint16_t		sqhd;	/* submission queue head pointer */
	uint16_t		sqid;	/* submission queue identifier */

	/* dword 3 */
	uint16_t		cid;	/* command identifier */
	struct spdk_nvme_status	status;
};

/**
 * Dataset Management range
 */
struct spdk_nvme_dsm_range {
	union {
		struct {
			uint32_t af		: 4; /**< access frequencey */
			uint32_t al		: 2; /**< access latency */
			uint32_t reserved0	: 2;

			uint32_t sr		: 1; /**< sequential read range */
			uint32_t sw		: 1; /**< sequential write range */
			uint32_t wp		: 1; /**< write prepare */
			uint32_t reserved1	: 13;

			uint32_t access_size	: 8; /**< command access size */
		} bits;

		uint32_t raw;
	} attributes;

	uint32_t length;
	uint64_t starting_lba;
};

union spdk_nvme_vs_register {
	uint32_t	raw;
	struct {
		/** indicates the tertiary version */
		uint32_t ter		: 8;
		/** indicates the minor version */
		uint32_t mnr		: 8;
		/** indicates the major version */
		uint32_t mjr		: 16;
	} bits;
};


struct spdk_nvme_power_state {
	uint16_t mp;				/* bits 15:00: maximum power */

	uint8_t reserved1;

	uint8_t mps		: 1;		/* bit 24: max power scale */
	uint8_t nops		: 1;		/* bit 25: non-operational state */
	uint8_t reserved2	: 6;

	uint32_t enlat;				/* bits 63:32: entry latency in microseconds */
	uint32_t exlat;				/* bits 95:64: exit latency in microseconds */

	uint8_t rrt		: 5;		/* bits 100:96: relative read throughput */
	uint8_t reserved3	: 3;

	uint8_t rrl		: 5;		/* bits 108:104: relative read latency */
	uint8_t reserved4	: 3;

	uint8_t rwt		: 5;		/* bits 116:112: relative write throughput */
	uint8_t reserved5	: 3;

	uint8_t rwl		: 5;		/* bits 124:120: relative write latency */
	uint8_t reserved6	: 3;

	uint8_t reserved7[16];
};

struct __attribute__((packed)) __attribute__((aligned)) spdk_nvme_ctrlr_data {
	/* bytes 0-255: controller capabilities and features */

	/** pci vendor id */
	uint16_t		vid;

	/** pci subsystem vendor id */
	uint16_t		ssvid;

	/** serial number */
	int8_t			sn[SPDK_NVME_CTRLR_SN_LEN];

	/** model number */
	int8_t			mn[SPDK_NVME_CTRLR_MN_LEN];

	/** firmware revision */
	uint8_t			fr[SPDK_NVME_CTRLR_FR_LEN];

	/** recommended arbitration burst */
	uint8_t			rab;

	/** ieee oui identifier */
	uint8_t			ieee[3];

	/** controller multi-path I/O and namespace sharing capabilities */
	struct {
		uint8_t multi_port	: 1;
		uint8_t multi_host	: 1;
		uint8_t sr_iov		: 1;
		uint8_t reserved	: 5;
	} cmic;

	/** maximum data transfer size */
	uint8_t			mdts;

	/** controller id */
	uint16_t		cntlid;

	/** version */
	union spdk_nvme_vs_register	ver;

	/** RTD3 resume latency */
	uint32_t		rtd3r;

	/** RTD3 entry latency */
	uint32_t		rtd3e;

	/** optional asynchronous events supported */
	struct {
		uint32_t	reserved1 : 8;

		/** Supports sending Namespace Attribute Notices. */
		uint32_t	ns_attribute_notices : 1;

		/** Supports sending Firmware Activation Notices. */
		uint32_t	fw_activation_notices : 1;

		uint32_t	reserved2 : 22;
	} oaes;

	/** controller attributes */
	struct {
		/** Supports 128-bit host identifier */
		uint32_t	host_id_exhid_supported: 1;

		/** Supports non-operational power state permissive mode */
		uint32_t	non_operational_power_state_permissive_mode: 1;

		uint32_t	reserved: 30;
	} ctratt;

	uint8_t			reserved_100[12];

	/** FRU globally unique identifier */
	uint8_t			fguid[16];

	uint8_t			reserved_128[128];

	/* bytes 256-511: admin command set attributes */

	/** optional admin command support */
	struct {
		/* supports security send/receive commands */
		uint16_t	security  : 1;

		/* supports format nvm command */
		uint16_t	format    : 1;

		/* supports firmware activate/download commands */
		uint16_t	firmware  : 1;

		/* supports ns manage/ns attach commands */
		uint16_t	ns_manage  : 1;

		/** Supports device self-test command (SPDK_NVME_OPC_DEVICE_SELF_TEST) */
		uint16_t	device_self_test : 1;

		/** Supports SPDK_NVME_OPC_DIRECTIVE_SEND and SPDK_NVME_OPC_DIRECTIVE_RECEIVE */
		uint16_t	directives : 1;

		/** Supports NVMe-MI (SPDK_NVME_OPC_NVME_MI_SEND, SPDK_NVME_OPC_NVME_MI_RECEIVE) */
		uint16_t	nvme_mi : 1;

		/** Supports SPDK_NVME_OPC_VIRTUALIZATION_MANAGEMENT */
		uint16_t	virtualization_management : 1;

		/** Supports SPDK_NVME_OPC_DOORBELL_BUFFER_CONFIG */
		uint16_t	doorbell_buffer_config : 1;

		uint16_t	oacs_rsvd : 7;
	} oacs;

	/** abort command limit */
	uint8_t			acl;

	/** asynchronous event request limit */
	uint8_t			aerl;

	/** firmware updates */
	struct {
		/* first slot is read-only */
		uint8_t		slot1_ro  : 1;

		/* number of firmware slots */
		uint8_t		num_slots : 3;

		/* support activation without reset */
		uint8_t		activation_without_reset : 1;

		uint8_t		frmw_rsvd : 3;
	} frmw;

	/** log page attributes */
	struct {
		/* per namespace smart/health log page */
		uint8_t		ns_smart : 1;
		/* command effects log page */
		uint8_t		celp : 1;
		/* extended data for get log page */
		uint8_t		edlp: 1;
		/** telemetry log pages and notices */
		uint8_t		telemetry : 1;
		uint8_t		lpa_rsvd : 4;
	} lpa;

	/** error log page entries */
	uint8_t			elpe;

	/** number of power states supported */
	uint8_t			npss;

	/** admin vendor specific command configuration */
	struct {
		/* admin vendor specific commands use disk format */
		uint8_t		spec_format : 1;

		uint8_t		avscc_rsvd  : 7;
	} avscc;

	/** autonomous power state transition attributes */
	struct {
		/** controller supports autonomous power state transitions */
		uint8_t		supported  : 1;

		uint8_t		apsta_rsvd : 7;
	} apsta;

	/** warning composite temperature threshold */
	uint16_t		wctemp;

	/** critical composite temperature threshold */
	uint16_t		cctemp;

	/** maximum time for firmware activation */
	uint16_t		mtfa;

	/** host memory buffer preferred size */
	uint32_t		hmpre;

	/** host memory buffer minimum size */
	uint32_t		hmmin;

	/** total NVM capacity */
	uint64_t		tnvmcap[2];

	/** unallocated NVM capacity */
	uint64_t		unvmcap[2];

	/** replay protected memory block support */
	struct {
		uint8_t		num_rpmb_units	: 3;
		uint8_t		auth_method	: 3;
		uint8_t		reserved1	: 2;

		uint8_t		reserved2;

		uint8_t		total_size;
		uint8_t		access_size;
	} rpmbs;

	/** extended device self-test time (in minutes) */
	uint16_t		edstt;

	/** device self-test options */
	union {
		uint8_t	raw;
		struct {
			/** Device supports only one device self-test operation at a time */
			uint8_t	one_only : 1;

			uint8_t	reserved : 7;
		} bits;
	} dsto;

	/**
	 * Firmware update granularity
	 *
	 * 4KB units
	 * 0x00 = no information provided
	 * 0xFF = no restriction
	 */
	uint8_t			fwug;

	/**
	 * Keep Alive Support
	 *
	 * Granularity of keep alive timer in 100 ms units
	 * 0 = keep alive not supported
	 */
	uint16_t		kas;

	/** Host controlled thermal management attributes */
	union {
		uint16_t		raw;
		struct {
			uint16_t	supported : 1;
			uint16_t	reserved : 15;
		} bits;
	} hctma;

	/** Minimum thermal management temperature */
	uint16_t		mntmt;

	/** Maximum thermal management temperature */
	uint16_t		mxtmt;

	/** Sanitize capabilities */
	union {
		uint32_t	raw;
		struct {
			uint32_t	crypto_erase : 1;
			uint32_t	block_erase : 1;
			uint32_t	overwrite : 1;
			uint32_t	reserved : 29;
		} bits;
	} sanicap;

	uint8_t			reserved3[180];

	/* bytes 512-703: nvm command set attributes */

	/** submission queue entry size */
	struct {
		uint8_t		min : 4;
		uint8_t		max : 4;
	} sqes;

	/** completion queue entry size */
	struct {
		uint8_t		min : 4;
		uint8_t		max : 4;
	} cqes;

	uint16_t		maxcmd;

	/** number of namespaces */
	uint32_t		nn;

	/** optional nvm command support */
	struct {
		uint16_t	compare : 1;
		uint16_t	write_unc : 1;
		uint16_t	dsm: 1;
		uint16_t	write_zeroes: 1;
		uint16_t	set_features_save: 1;
		uint16_t	reservations: 1;
		uint16_t	timestamp: 1;
		uint16_t	reserved: 9;
	} oncs;

	/** fused operation support */
	uint16_t		fuses;

	/** format nvm attributes */
	struct {
		uint8_t		format_all_ns: 1;
		uint8_t		erase_all_ns: 1;
		uint8_t		crypto_erase_supported: 1;
		uint8_t		reserved: 5;
	} fna;

	/** volatile write cache */
	struct {
		uint8_t		present : 1;
		uint8_t		flush_broadcast : 2;
		uint8_t		reserved : 5;
	} vwc;

	/** atomic write unit normal */
	uint16_t		awun;

	/** atomic write unit power fail */
	uint16_t		awupf;

	/** NVM vendor specific command configuration */
	uint8_t			nvscc;

	uint8_t			reserved531;

	/** atomic compare & write unit */
	uint16_t		acwu;

	uint16_t		reserved534;

	/** SGL support */
	struct {
		uint32_t	supported : 2;
		uint32_t	keyed_sgl : 1;
		uint32_t	reserved1 : 13;
		uint32_t	bit_bucket_descriptor : 1;
		uint32_t	metadata_pointer : 1;
		uint32_t	oversized_sgl : 1;
		uint32_t	metadata_address : 1;
		uint32_t	sgl_offset : 1;
		uint32_t	transport_sgl : 1;
		uint32_t	reserved2 : 10;
	} sgls;

	uint8_t			reserved4[228];

	uint8_t			subnqn[256];

	uint8_t			reserved5[768];

	/** NVMe over Fabrics-specific fields */
	struct {
		/** I/O queue command capsule supported size (16-byte units) */
		uint32_t	ioccsz;

		/** I/O queue response capsule supported size (16-byte units) */
		uint32_t	iorcsz;

		/** In-capsule data offset (16-byte units) */
		uint16_t	icdoff;

		/** Controller attributes */
		struct {
			/** Controller model: \ref spdk_nvmf_ctrlr_model */
			uint8_t	ctrlr_model : 1;
			uint8_t reserved : 7;
		} ctrattr;

		/** Maximum SGL block descriptors (0 = no limit) */
		uint8_t		msdbd;

		uint8_t		reserved[244];
	} nvmf_specific;

	/* bytes 2048-3071: power state descriptors */
	struct spdk_nvme_power_state	psd[32];

	/* bytes 3072-4095: vendor specific */
	uint8_t			vs[1024];
};

struct __attribute__((packed)) spdk_nvme_primary_ctrl_capabilities {
	/**  controller id */
	uint16_t		cntlid;
	/**  port identifier */
	uint16_t		portid;
	/**  controller resource types */
	struct {
		uint8_t vq_supported	: 1;
		uint8_t vi_supported	: 1;
		uint8_t reserved	: 6;
	} crt;
	uint8_t			reserved[27];
	/** total number of VQ flexible resources */
	uint32_t		vqfrt;
	/** total number of VQ flexible resources assigned to secondary controllers */
	uint32_t		vqrfa;
	/** total number of VQ flexible resources allocated to primary controller */
	uint16_t		vqrfap;
	/** total number of VQ Private resources for the primary controller */
	uint16_t		vqprt;
	/** max number of VQ flexible Resources that may be assigned to a secondary controller */
	uint16_t		vqfrsm;
	/** preferred granularity of assigning and removing VQ Flexible Resources */
	uint16_t		vqgran;
	uint8_t			reserved1[16];
	/** total number of VI flexible resources for the primary and its secondary controllers */
	uint32_t		vifrt;
	/** total number of VI flexible resources assigned to the secondary controllers */
	uint32_t		virfa;
	/** total number of VI flexible resources currently allocated to the primary controller */
	uint16_t		virfap;
	/** total number of VI private resources for the primary controller */
	uint16_t		viprt;
	/** max number of VI flexible resources that may be assigned to a secondary controller */
	uint16_t		vifrsm;
	/** preferred granularity of assigning and removing VI flexible resources */
	uint16_t		vigran;
	uint8_t			reserved2[4016];
};


struct __attribute__((packed)) spdk_nvme_secondary_ctrl_entry {
	/** controller identifier of the secondary controller */
	uint16_t		scid;
	/** controller identifier of the associated primary controller */
	uint16_t		pcid;
	/** indicates the state of the secondary controller */
	struct {
		uint8_t is_online	: 1;
		uint8_t reserved	: 7;
	} scs;
	uint8_t	reserved[3];
	/** VF number if the secondary controller is an SR-IOV VF */
	uint16_t		vfn;
	/** number of VQ flexible resources assigned to the indicated secondary controller */
	uint16_t		nvq;
	/** number of VI flexible resources assigned to the indicated secondary controller */
	uint16_t		nvi;
	uint8_t			reserved1[18];
};


struct __attribute__((packed)) spdk_nvme_secondary_ctrl_list {
	/** number of Secondary controller entries in the list */
	uint8_t					number;
	uint8_t					reserved[31];
	struct spdk_nvme_secondary_ctrl_entry	entries[127];
};

struct spdk_nvme_ns_data {
	/** namespace size */
	uint64_t		nsze;

	/** namespace capacity */
	uint64_t		ncap;

	/** namespace utilization */
	uint64_t		nuse;

	/** namespace features */
	struct {
		/** thin provisioning */
		uint8_t		thin_prov : 1;

		/** NAWUN, NAWUPF, and NACWU are defined for this namespace */
		uint8_t		ns_atomic_write_unit : 1;

		/** Supports Deallocated or Unwritten LBA error for this namespace */
		uint8_t		dealloc_or_unwritten_error : 1;

		/** Non-zero NGUID and EUI64 for namespace are never reused */
		uint8_t		guid_never_reused : 1;

		uint8_t		reserved1 : 4;
	} nsfeat;

	/** number of lba formats */
	uint8_t			nlbaf;

	/** formatted lba size */
	struct {
		uint8_t		format    : 4;
		uint8_t		extended  : 1;
		uint8_t		reserved2 : 3;
	} flbas;

	/** metadata capabilities */
	struct {
		/** metadata can be transferred as part of data prp list */
		uint8_t		extended  : 1;

		/** metadata can be transferred with separate metadata pointer */
		uint8_t		pointer   : 1;

		/** reserved */
		uint8_t		reserved3 : 6;
	} mc;

	/** end-to-end data protection capabilities */
	struct {
		/** protection information type 1 */
		uint8_t		pit1     : 1;

		/** protection information type 2 */
		uint8_t		pit2     : 1;

		/** protection information type 3 */
		uint8_t		pit3     : 1;

		/** first eight bytes of metadata */
		uint8_t		md_start : 1;

		/** last eight bytes of metadata */
		uint8_t		md_end   : 1;
	} dpc;

	/** end-to-end data protection type settings */
	struct {
		/** protection information type */
		uint8_t		pit       : 3;

		/** 1 == protection info transferred at start of metadata */
		/** 0 == protection info transferred at end of metadata */
		uint8_t		md_start  : 1;

		uint8_t		reserved4 : 4;
	} dps;

	/** namespace multi-path I/O and namespace sharing capabilities */
	struct {
		uint8_t		can_share : 1;
		uint8_t		reserved : 7;
	} nmic;

	/** reservation capabilities */
	union {
		struct {
			/** supports persist through power loss */
			uint8_t		persist : 1;

			/** supports write exclusive */
			uint8_t		write_exclusive : 1;

			/** supports exclusive access */
			uint8_t		exclusive_access : 1;

			/** supports write exclusive - registrants only */
			uint8_t		write_exclusive_reg_only : 1;

			/** supports exclusive access - registrants only */
			uint8_t		exclusive_access_reg_only : 1;

			/** supports write exclusive - all registrants */
			uint8_t		write_exclusive_all_reg : 1;

			/** supports exclusive access - all registrants */
			uint8_t		exclusive_access_all_reg : 1;

			/** supports ignore existing key */
			uint8_t		ignore_existing_key : 1;
		} rescap;
		uint8_t		raw;
	} nsrescap;
	/** format progress indicator */
	struct {
		uint8_t		percentage_remaining : 7;
		uint8_t		fpi_supported : 1;
	} fpi;

	/** deallocate logical features */
	union {
		uint8_t		raw;
		struct {
			/**
			 * Value read from deallocated blocks
			 *
			 * 000b = not reported
			 * 001b = all bytes 0x00
			 * 010b = all bytes 0xFF
			 *
			 * \ref spdk_nvme_dealloc_logical_block_read_value
			 */
			uint8_t	read_value : 3;

			/** Supports Deallocate bit in Write Zeroes */
			uint8_t	write_zero_deallocate : 1;

			/**
			 * Guard field behavior for deallocated logical blocks
			 * 0: contains 0xFFFF
			 * 1: contains CRC for read value
			 */
			uint8_t	guard_value : 1;

			uint8_t	reserved : 3;
		} bits;
	} dlfeat;

	/** namespace atomic write unit normal */
	uint16_t		nawun;

	/** namespace atomic write unit power fail */
	uint16_t		nawupf;

	/** namespace atomic compare & write unit */
	uint16_t		nacwu;

	/** namespace atomic boundary size normal */
	uint16_t		nabsn;

	/** namespace atomic boundary offset */
	uint16_t		nabo;

	/** namespace atomic boundary size power fail */
	uint16_t		nabspf;

	/** namespace optimal I/O boundary in logical blocks */
	uint16_t		noiob;

	/** NVM capacity */
	uint64_t		nvmcap[2];

	uint8_t			reserved64[40];

	/** namespace globally unique identifier */
	uint8_t			nguid[16];

	/** IEEE extended unique identifier */
	uint64_t		eui64;

	/** lba format support */
	struct {
		/** metadata size */
		uint32_t	ms	  : 16;

		/** lba data size */
		uint32_t	lbads	  : 8;

		/** relative performance */
		uint32_t	rp	  : 2;

		uint32_t	reserved6 : 6;
	} lbaf[16];

	uint8_t			reserved6[192];

	uint8_t			vendor_specific[3712];
};

// #if 0

struct spdk_nvme_reservation_acquire_data {
	/** current reservation key */
	uint64_t		crkey;
	/** preempt reservation key */
	uint64_t		prkey;
};

struct __attribute__((packed)) spdk_nvme_reservation_status_data {
	/** reservation action generation counter */
	uint32_t		gen;
	/** reservation type */
	uint8_t			rtype;
	/** number of registered controllers */
	uint16_t		regctl;
	uint16_t		reserved1;
	/** persist through power loss state */
	uint8_t			ptpls;
	uint8_t			reserved[14];
};

struct __attribute__((packed)) spdk_nvme_reservation_status_extended_data {
	struct spdk_nvme_reservation_status_data	data;
	uint8_t						reserved[40];
};


struct __attribute__((packed)) spdk_nvme_registered_ctrlr_data {
	/** controller id */
	uint16_t		cntlid;
	/** reservation status */
	struct {
		uint8_t		status    : 1;
		uint8_t		reserved1 : 7;
	} rcsts;
	uint8_t			reserved2[5];
	/** 64-bit host identifier */
	uint64_t		hostid;
	/** reservation key */
	uint64_t		rkey;
};

struct __attribute__((packed)) spdk_nvme_registered_ctrlr_extended_data {
	/** controller id */
	uint16_t		cntlid;
	/** reservation status */
	struct {
		uint8_t		status    : 1;
		uint8_t		reserved1 : 7;
	} rcsts;
	uint8_t			reserved2[5];
	/** reservation key */
	uint64_t		rkey;
	/** 128-bit host identifier */
	uint8_t			hostid[16];
	uint8_t			reserved3[32];
};

struct spdk_nvme_reservation_register_data {
	/** current reservation key */
	uint64_t		crkey;
	/** new reservation key */
	uint64_t		nrkey;
};

struct spdk_nvme_reservation_key_data {
	/** current reservation key */
	uint64_t		crkey;
};


/**
 * Reservation notification log
 */
struct spdk_nvme_reservation_notification_log {
	/** 64-bit incrementing reservation notification log page count */
	uint64_t	log_page_count;
	/** Reservation notification log page type */
	uint8_t		type;
	/** Number of additional available reservation notification log pages */
	uint8_t		num_avail_log_pages;
	uint8_t		reserved[2];
	uint32_t	nsid;
	uint8_t		reserved1[48];
};

/**
 * Error information log page (\ref SPDK_NVME_LOG_ERROR)
 */
struct spdk_nvme_error_information_entry {
	uint64_t		error_count;
	uint16_t		sqid;
	uint16_t		cid;
	struct spdk_nvme_status	status;
	uint16_t		error_location;
	uint64_t		lba;
	uint32_t		nsid;
	uint8_t			vendor_specific;
	uint8_t			trtype;
	uint8_t			reserved30[2];
	uint64_t		command_specific;
	uint16_t		trtype_specific;
	uint8_t			reserved42[22];
};

/* Commands Supported and Effects Data Structure */
struct spdk_nvme_cmds_and_effect_entry {
	/** Command Supported */
	uint16_t csupp : 1;

	/** Logic Block Content Change  */
	uint16_t lbcc  : 1;

	/** Namespace Capability Change */
	uint16_t ncc   : 1;

	/** Namespace Inventory Change */
	uint16_t nic   : 1;

	/** Controller Capability Change */
	uint16_t ccc   : 1;

	uint16_t reserved1 : 11;

	/* Command Submission and Execution recommendation
	 * 000 - No command submission or execution restriction
	 * 001 - Submitted when there is no outstanding command to same NS
	 * 010 - Submitted when there is no outstanding command to any NS
	 * others - Reserved
	 * \ref command_submission_and_execution in section 5.14.1.5 NVMe Revision 1.3
	 */
	uint16_t cse : 3;

	uint16_t reserved2 : 13;
};

/* Commands Supported and Effects Log Page */
struct spdk_nvme_cmds_and_effect_log_page {
	/** Commands Supported and Effects Data Structure for the Admin Commands */
	struct spdk_nvme_cmds_and_effect_entry admin_cmds_supported[256];

	/** Commands Supported and Effects Data Structure for the IO Commands */
	struct spdk_nvme_cmds_and_effect_entry io_cmds_supported[256];

	uint8_t reserved0[2048];
};

// #if 0

/*
 * Get Log Page – Telemetry Host/Controller Initiated Log (Log Identifiers 07h/08h)
 */
struct spdk_nvme_telemetry_log_page_hdr {
	/* Log page identifier */
	uint8_t    lpi;
	uint8_t    rsvd[4];
	uint8_t    ieee_oui[3];
	/* Data area 1 last block */
	uint16_t   dalb1;
	/* Data area 2 last block */
	uint16_t   dalb2;
	/* Data area 3 last block */
	uint16_t   dalb3;
	uint8_t    rsvd1[368];
	/* Controller initiated data avail */
	uint8_t    ctrlr_avail;
	/* Controller initiated telemetry data generation */
	uint8_t    ctrlr_gen;
	/* Reason identifier */
	uint8_t    rsnident[128];
	uint8_t    telemetry_datablock[512];
};


/**
 * Sanitize status sstat field
 */
struct spdk_nvme_sanitize_status_sstat {
	uint16_t status			: 3;
	uint16_t complete_pass		: 5;
	uint16_t global_data_erase	: 1;
	uint16_t reserved		: 7;
};

/**
 * Sanitize log page
 */
struct spdk_nvme_sanitize_status_log_page {
	/* Sanitize progress */
	uint16_t				sprog;
	/* Sanitize status */
	struct spdk_nvme_sanitize_status_sstat	sstat;
	/* CDW10 of sanitize command */
	uint32_t				scdw10;
	/* Estimated overwrite time in seconds */
	uint32_t				et_overwrite;
	/* Estimated block erase time in seconds */
	uint32_t				et_block_erase;
	/* Estimated crypto erase time in seconds */
	uint32_t				et_crypto_erase;
	uint8_t					reserved[492];
};


/**
 * Asynchronous Event Request Completion
 */
union spdk_nvme_async_event_completion {
	uint32_t raw;
	struct {
		uint32_t async_event_type	: 3;
		uint32_t reserved1		: 5;
		uint32_t async_event_info	: 8;
		uint32_t log_page_identifier	: 8;
		uint32_t reserved2		: 8;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_ARBITRATION
 */
union spdk_nvme_feat_arbitration {
	uint32_t raw;
	struct {
		/** Arbitration Burst */
		uint32_t ab : 3;

		uint32_t reserved : 5;

		/** Low Priority Weight */
		uint32_t lpw : 8;

		/** Medium Priority Weight */
		uint32_t mpw : 8;

		/** High Priority Weight */
		uint32_t hpw : 8;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_POWER_MANAGEMENT
 */
union spdk_nvme_feat_power_management {
	uint32_t raw;
	struct {
		/** Power State */
		uint32_t ps : 5;

		/** Workload Hint */
		uint32_t wh : 3;

		uint32_t reserved : 24;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_LBA_RANGE_TYPE
 */
union spdk_nvme_feat_lba_range_type {
	uint32_t raw;
	struct {
		/** Number of LBA Ranges */
		uint32_t num : 6;

		uint32_t reserved : 26;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_TEMPERATURE_THRESHOLD
 */
union spdk_nvme_feat_temperature_threshold {
	uint32_t raw;
	struct {
		/** Temperature Threshold */
		uint32_t tmpth : 16;

		/** Threshold Temperature Select */
		uint32_t tmpsel : 4;

		/** Threshold Type Select */
		uint32_t thsel : 2;

		uint32_t reserved : 10;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_ERROR_RECOVERY
 */
union spdk_nvme_feat_error_recovery {
	uint32_t raw;
	struct {
		/** Time Limited Error Recovery */
		uint32_t tler : 16;

		/** Deallocated or Unwritten Logical Block Error Enable */
		uint32_t dulbe : 1;

		uint32_t reserved : 15;
	} bits;
};

// #if 0

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_VOLATILE_WRITE_CACHE
 */
union spdk_nvme_feat_volatile_write_cache {
	uint32_t raw;
	struct {
		/** Volatile Write Cache Enable */
		uint32_t wce : 1;

		uint32_t reserved : 31;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_NUMBER_OF_QUEUES
 */
union spdk_nvme_feat_number_of_queues {
	uint32_t raw;
	struct {
		/** Number of I/O Submission Queues Requested */
		uint32_t nsqr : 16;

		/** Number of I/O Completion Queues Requested */
		uint32_t ncqr : 16;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_INTERRUPT_COALESCING
 */
union spdk_nvme_feat_interrupt_coalescing {
	uint32_t raw;
	struct {
		/** Aggregation Threshold */
		uint32_t thr : 8;

		/** Aggregration time */
		uint32_t time : 8;

		uint32_t reserved : 16;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION
 */
union spdk_nvme_feat_interrupt_vector_configuration {
	uint32_t raw;
	struct {
		/** Interrupt Vector */
		uint32_t iv : 16;

		/** Coalescing Disable */
		uint32_t cd : 1;

		uint32_t reserved : 15;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_WRITE_ATOMICITY
 */
union spdk_nvme_feat_write_atomicity {
	uint32_t raw;
	struct {
		/** Disable Normal */
		uint32_t dn : 1;

		uint32_t reserved : 31;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_AUTONOMOUS_POWER_STATE_TRANSITION
 */
union spdk_nvme_feat_autonomous_power_state_transition {
	uint32_t raw;
	struct {
		/** Autonomous Power State Transition Enable */
		uint32_t apste : 1;

		uint32_t reserved : 31;
	} bits;
};

// #if 0
// after

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_HOST_MEM_BUFFER
 */
union spdk_nvme_feat_host_mem_buffer {
	uint32_t raw;
	struct {
		/** Enable Host Memory */
		uint32_t ehm : 1;

		/** Memory Return */
		uint32_t mr : 1;

		uint32_t reserved : 30;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_KEEP_ALIVE_TIMER
 */
union spdk_nvme_feat_keep_alive_timer {
	uint32_t raw;
	struct {
		/** Keep Alive Timeout */
		uint32_t kato : 32;
	} bits;
};


/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_HOST_CONTROLLED_THERMAL_MANAGEMENT
 */
union spdk_nvme_feat_host_controlled_thermal_management {
	uint32_t raw;
	struct {
		/** Thermal Management Temperature 2 */
		uint32_t tmt2 : 16;

		/** Thermal Management Temperature 1 */
		uint32_t tmt1 : 16;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_NON_OPERATIONAL_POWER_STATE_CONFIG
 */
union spdk_nvme_feat_non_operational_power_state_config {
	uint32_t raw;
	struct {
		/** Non-Operational Power State Permissive Mode Enable */
		uint32_t noppme : 1;

		uint32_t reserved : 31;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_SOFTWARE_PROGRESS_MARKER
 */
union spdk_nvme_feat_software_progress_marker {
	uint32_t raw;
	struct {
		/** Pre-boot Software Load Count */
		uint32_t pbslc : 8;

		uint32_t reserved : 24;
	} bits;
};

/**
 * Data used by Set Features/Get Features \ref SPDK_NVME_FEAT_HOST_IDENTIFIER
 */
union spdk_nvme_feat_host_identifier {
	uint32_t raw;
	struct {
		/** Enable Extended Host Identifier */
		uint32_t exhid : 1;

		uint32_t reserved : 31;
	} bits;
};

/**
 * Firmware slot information page (\ref SPDK_NVME_LOG_FIRMWARE_SLOT)
 */
struct spdk_nvme_firmware_page {
	struct {
		uint8_t	active_slot	: 3; /**< Slot for current FW */
		uint8_t	reserved3	: 1;
		uint8_t	next_reset_slot	: 3; /**< Slot that will be active at next controller reset */
		uint8_t	reserved7	: 1;
	} afi;

	uint8_t			reserved[7];
	uint8_t			revision[7][8]; /** Revisions for 7 slots (ASCII strings) */
	uint8_t			reserved2[448];
};


struct spdk_nvme_ns_list {
	uint32_t ns_list[1024];
};

struct spdk_nvme_ctrlr_list {
	uint16_t ctrlr_count;
	uint16_t ctrlr_list[2047];
};

enum spdk_nvme_secure_erase_setting {
	SPDK_NVME_FMT_NVM_SES_NO_SECURE_ERASE	= 0x0,
	SPDK_NVME_FMT_NVM_SES_USER_DATA_ERASE	= 0x1,
	SPDK_NVME_FMT_NVM_SES_CRYPTO_ERASE	= 0x2,
};

enum spdk_nvme_pi_location {
	SPDK_NVME_FMT_NVM_PROTECTION_AT_TAIL	= 0x0,
	SPDK_NVME_FMT_NVM_PROTECTION_AT_HEAD	= 0x1,
};

enum spdk_nvme_pi_type {
	SPDK_NVME_FMT_NVM_PROTECTION_DISABLE		= 0x0,
	SPDK_NVME_FMT_NVM_PROTECTION_TYPE1		= 0x1,
	SPDK_NVME_FMT_NVM_PROTECTION_TYPE2		= 0x2,
	SPDK_NVME_FMT_NVM_PROTECTION_TYPE3		= 0x3,
};

enum spdk_nvme_metadata_setting {
	SPDK_NVME_FMT_NVM_METADATA_TRANSFER_AS_BUFFER	= 0x0,
	SPDK_NVME_FMT_NVM_METADATA_TRANSFER_AS_LBA	= 0x1,
};

// before
// #if 0

struct spdk_nvme_format {
	uint32_t	lbaf		: 4;
	uint32_t	ms		: 1;
	uint32_t	pi		: 3;
	uint32_t	pil		: 1;
	uint32_t	ses		: 3;
	uint32_t	reserved	: 20;
};

struct spdk_nvme_protection_info {
	uint16_t	guard;
	uint16_t	app_tag;
	uint32_t	ref_tag;
};

/* Data structures for sanitize command */
/* Sanitize - Command Dword 10 */
struct spdk_nvme_sanitize {
	/* Sanitize Action (SANACT) */
	uint32_t sanact	: 3;
	/* Allow Unrestricted Sanitize Exit (AUSE) */
	uint32_t ause	: 1;
	/* Overwrite Pass Count (OWPASS) */
	uint32_t owpass	: 4;
	/* Overwrite Invert Pattern Between Passes */
	uint32_t oipbp	: 1;
	/* No Deallocate after sanitize (NDAS) */
	uint32_t ndas	: 1;
	/* reserved */
	uint32_t reserved	: 22;
};

/* Sanitize Action */
enum spdk_sanitize_action {
	/* Exit Failure Mode */
	SPDK_NVME_SANITIZE_EXIT_FAILURE_MODE	= 0x1,
	/* Start a Block Erase sanitize operation */
	SPDK_NVME_SANITIZE_BLOCK_ERASE		= 0x2,
	/* Start an Overwrite sanitize operation */
	SPDK_NVME_SANITIZE_OVERWRITE		= 0x3,
	/* Start a Crypto Erase sanitize operation */
	SPDK_NVME_SANITIZE_CRYPTO_ERASE		= 0x4,
};

/** Parameters for SPDK_NVME_OPC_FIRMWARE_COMMIT cdw10: commit action */
enum spdk_nvme_fw_commit_action {
	/**
	 * Downloaded image replaces the image specified by
	 * the Firmware Slot field. This image is not activated.
	 */
	SPDK_NVME_FW_COMMIT_REPLACE_IMG			= 0x0,
	/**
	 * Downloaded image replaces the image specified by
	 * the Firmware Slot field. This image is activated at the next reset.
	 */
	SPDK_NVME_FW_COMMIT_REPLACE_AND_ENABLE_IMG	= 0x1,
	/**
	 * The image specified by the Firmware Slot field is
	 * activated at the next reset.
	 */
	SPDK_NVME_FW_COMMIT_ENABLE_IMG			= 0x2,
	/**
	 * The image specified by the Firmware Slot field is
	 * requested to be activated immediately without reset.
	 */
	SPDK_NVME_FW_COMMIT_RUN_IMG			= 0x3,
};

/** Parameters for SPDK_NVME_OPC_FIRMWARE_COMMIT cdw10 */
struct spdk_nvme_fw_commit {
	/**
	 * Firmware Slot. Specifies the firmware slot that shall be used for the
	 * Commit Action. The controller shall choose the firmware slot (slot 1 - 7)
	 * to use for the operation if the value specified is 0h.
	 */
	uint32_t	fs		: 3;
	/**
	 * Commit Action. Specifies the action that is taken on the image downloaded
	 * with the Firmware Image Download command or on a previously downloaded and
	 * placed image.
	 */
	uint32_t	ca		: 3;
	uint32_t	reserved	: 26;
};

#endif