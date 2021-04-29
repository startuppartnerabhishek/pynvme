#
#  BSD LICENSE
#
#  Copyright (c) Crane Chu <cranechu@gmail.com>
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
#    * Neither the name of Intel Corporation nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

setup(
    ext_modules=cythonize(
        [Extension(
            "nvme",
            ["./driver/cython/driver_wrap.pyx"],

            # include paths
            include_dirs = ['../spdk/include', './driver/include'],

            # dpdk prebuilt static libraries
            libraries=['uuid', 'numa', 'pthread'],

            # spdk static libraries
            extra_objects=[
                # spdk
                '../spdk/build/lib/libspdk_pynvme.a',
                '../spdk/build/lib/libspdk_nvme.a',
                '../spdk/build/lib/libspdk_env_dpdk.a',
                '../spdk/build/lib/libspdk_util.a',
                '../spdk/build/lib/libspdk_sock.a',
                '../spdk/build/lib/libspdk_rpc.a',
                '../spdk/build/lib/libspdk_log.a',

                # force link symbols in these libraries
                '-Wl,--whole-archive',
                '../spdk/build/lib/libspdk_json.a',
                '../spdk/build/lib/libspdk_jsonrpc.a',
                '../spdk/build/lib/libspdk_sock_posix.a',
                '-Wl,--no-whole-archive',

                # dpdk
                '../spdk/dpdk/build/lib/librte_eal.a',
                '../spdk/dpdk/build/lib/librte_mbuf.a',
                '../spdk/dpdk/build/lib/librte_ring.a',
                '../spdk/dpdk/build/lib/librte_mempool.a',
                '../spdk/dpdk/build/lib/librte_bus_pci.a',
                '../spdk/dpdk/build/lib/librte_pci.a',
                '../spdk/dpdk/build/lib/librte_kvargs.a',
            ],
        )]
    )
)
