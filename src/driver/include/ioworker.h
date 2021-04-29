/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Crane Chu <cranechu@gmail.com>
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


// used for callback
struct ioworker_io_ctx {
  void* data_buf;
  void* write_buf;
  uint8_t opcode;
  uint32_t op_index;
  struct timeval time_sent;
  struct ioworker_global_ctx* gctx;
  struct ioworker_cmdlog cmd;

  uint32_t io_sequence_index;

  // next pending io
  STAILQ_ENTRY(ioworker_io_ctx) next;
};

struct ioworker_distribution_lookup {
  uint64_t lba_start;
  uint64_t lba_end;
};

struct ioworker_global_ctx {
  struct ioworker_args* args;
  struct ioworker_rets* rets;
  struct spdk_nvme_ns* ns;
  struct spdk_nvme_qpair *qpair;
  struct timeval due_time;
  struct timeval io_due_time;
  struct timeval io_delay_time;
  struct timeval time_next_sec;
  uint64_t io_count_till_last_sec;
  uint64_t sequential_lba;
  uint64_t io_count_sent;
  uint64_t io_count_cplt;
  uint64_t total_latency_us;
  uint32_t last_sec;
  uint32_t current_cmdlog_index;
  bool flag_finish;

  // replay io sequence
  ioworker_ioseq* io_sequence;
  uint32_t io_sequence_count;
  uint32_t io_sequence_index;
  struct timeval io_sequence_start;

  // distribution loopup table
  bool distribution;
  struct ioworker_distribution_lookup dl_table[10000];

  // io_size lookup table
  uint32_t sl_table[10000];

  uint8_t op_table[100];

  // pending io list
  STAILQ_HEAD(, ioworker_io_ctx)  pending_io_list;
};
