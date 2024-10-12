from setuptools import setup, Extension

# Define the extension module
container = Extension("container", sources=["containermodule.c"])

# Use setuptools to setup the package
setup(
    name="container",
    version="1.0",
    description="Package for running Python in a container",
    ext_modules=[container],
)
