from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

ext_module_list = [
            Extension(
                "nvme_sim",

                ["./driver/cython/driver_wrap.pyx"],

                # include paths
                include_dirs = ['../spdk/include', './driver/include'],

                # extra standard libraries
                libraries=[],

                # sim static libraries
                extra_objects=[
                    './build/libdrvsim.a',
                ],
            ),
]

setup(
    ext_modules = cythonize(ext_module_list)
)