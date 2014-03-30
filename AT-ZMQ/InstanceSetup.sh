#!/bin/sh

echo "nameserver 8.8.8.8" | sudo tee /etc/resolv.conf > /dev/null
sudo apt-get update
sudo apt-get -q -y install git automake autoconf g++ gcc make libtool ntp
if [ $? -ne 0 ] ; then
echo "installing packages failed";
fi

export LD_LIBRARY_PATH=`pwd`/lib;
export AWS_HOME=`pwd`;
