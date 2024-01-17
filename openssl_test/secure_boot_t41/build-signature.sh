#!/bin/bash
rm -rf out

# cd uboot;
#make clean
# make scb_t41_sfc0_nor -j16
# cp u-boot-with-spl.bin ../sinature_uboot_pss/input_source

cd sinature_uboot_pss; ./signature_uboot.sh 1
mv out ../

