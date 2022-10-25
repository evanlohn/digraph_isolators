#!/bin/bash
curl -o nauty27r4.tar.gz https://pallini.di.uniroma1.it/nauty27r4.tar.gz
tar xvzf nauty27r4.tar.gz
cd nauty27r4
./configure
make
cd ..
ln -s nauty27r4/nauty.a nauty.a
ln -s nauty27r4/nauty.h nauty.h
ln -s nauty27r4/naurng.h naurng.h
ln -s nauty27r4/nausparse.h nausparse.h
ln -s nauty27r4/nautinv.h nautinv.h
ln -s nauty27r4/naututil.h naututil.h
make
