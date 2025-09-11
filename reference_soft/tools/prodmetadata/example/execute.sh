#!/bin/bash

./prodmetadataEncoder -ipmc pmd.txt -iomf omf.txt -opmc payload.dat -ccfg 2 -ocfg 3 2 1 0
./prodmetadataDecoder -ipmc payload.dat -iomf omf.txt -ccfg 2 -ocfg 3 2 1 0