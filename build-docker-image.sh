#!/bin/bash

progdir=$(dirname $(readlink -f $0))

$progdir/create-docker-files.sh

exec docker build --tag dynainstrumentsoss/eco-g45-linux-bsp .
