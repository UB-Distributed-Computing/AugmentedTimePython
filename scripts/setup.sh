#!/bin/sh

echo """
colorscheme elflord 
set nobk
set nowb
set tabstop=4
set shiftwidth=4
set softtabstop=4
set smarttab
set expandtab
set autoindent
set cindent
set background=dark
""" >> ~/.vimrc

yes Y|sudo apt-get update
yes Y|sudo apt-get install gcc g++ make autoconf automake git libtool

sudo rm -rf zeromq-4.0.4.tar.gz zeromq-4.0.4
wget http://download.zeromq.org/zeromq-4.0.4.tar.gz
tar xvf zeromq-4.0.4.tar.gz

curpath=`pwd`
cd zeromq-4.0.4
sudo ./configure
sudo make
sudo make install
cd $curpath
sudo rm -rf zeromq-4.0.4.tar.gz zeromq-4.0.4

curpath=`pwd`
rm -rf ~/AT
mkdir -p ~/AT
cd ~/AT
git clone https://github.com/UB-Distributed-Computing/AugmentedTimePython.git
cd AugmentedTimePython/AT-ZMQ
make
cd $curpath
exit
