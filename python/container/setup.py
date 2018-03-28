from distutils.core import setup, Extension

container = Extension('container',
                    sources = ['containermodule.c'])

setup (name = 'container',
       version = '1.0',
       description = 'Package for run python in container',
       ext_modules = [container])
