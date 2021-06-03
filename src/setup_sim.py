from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import json
import os

agent_lib_path=None


# find location of agent-libs
with open("../conf/simconf.json") as f:
    json_conf = json.load(f)
    agent_lib_path = json_conf['global']['nsv_test_root_path'] + 'build/'
    if (not os.path.isabs(agent_lib_path)):
        agent_lib_path = '../' + agent_lib_path

print('setup_py.sim: Will look for agent-libs in ' + agent_lib_path)

ext_module_list = [
            Extension(
                "nvme_sim",

                ["./driver/cython/driver_wrap.pyx"],

                # include paths
                include_dirs = ['../spdk/include', './driver/include'],

                # extra standard libraries
                libraries=['ev', 'pthread', 'uuid'],

                # sim static libraries
                extra_objects=[
                    './build/libdrvsim.a',
                    agent_lib_path + 'libpdsnvmedriver.so',
                    agent_lib_path + 'libpdsnvmepci.so'
                ],
            ),
]

setup(
    ext_modules = cythonize(ext_module_list)
)
