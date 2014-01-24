# -*- coding: utf-8 -*-

from setuptools import setup, find_packages


with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

setup(
    name='AugmentedTime',
    version='0.0.1',
    description='AugmentedTime Python Library',
    long_description=readme,
    author='Rob Colantuoni',
    author_email='rgc@colantuoni.com',
    url='https://github.com/UB-Distributed-Computing/AugmentedTimePython',
    license=license,
    packages=find_packages(exclude=('tests', 'docs'))
)

