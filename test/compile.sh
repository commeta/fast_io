#!/bin/bash

make clean
make
make test

cp -f /home/commeta/project/kernel/fast_io/modules/fast_io.so /lib/php/20230831/fast_io.so

php test.php
