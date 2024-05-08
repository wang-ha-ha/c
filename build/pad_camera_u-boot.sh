#!/bin/bash

# 客户自行编译的uboot固件
INPUT_FILE=""

# SDK 中自带的低功耗快起固件
BOOT_FILE=""

# 输出文件
OUTPUT_FILE=""

UBOOT_OFFSET=36

while getopts "i:o:b:" arg
do
    case $arg in
	i)
	    INPUT_FILE=$OPTARG
	    ;;
	o)
	    OUTPUT_FILE=$OPTARG
	    ;;
	b)
	    BOOT_FILE=$OPTARG
	    ;;
        ?)
            echo "unkonw argument"
            exit 1
	    ;;
    esac
done

function usage()
{
    echo "usage:"
    echo "  ./pad_camera_u-boot.sh -i <input_file> -o <output_file> -b <boot.bin fw>"
    echo "eg:"
    echo "  ./pad_camera_u-boot.sh -i u-boot-with-spl.bin -o boot_custom -b \${ZRT_ENV_TOP_DIR}/os/uboot/bin/boot_SOC_T31Z_V1.bin"
}

if [ "${INPUT_FILE}" = "" ]; then
    echo "error: Must set input file path"
    usage
    exit 1
fi

if [ "${OUTPUT_FILE}" = "" ]; then
    echo "error: Must set output file path"
    usage
    exit 1
fi

if [ "${BOOT_FILE}" = "" ]; then
    echo "error: Must set boot file path"
    usage
    exit 1
fi

if [ ! -f ${INPUT_FILE} ]; then
    echo "error: Can't find input file: "${INPUT_FILE}
    exit 1
fi

if [ ! -f ${BOOT_FILE} ]; then
    echo "error: Can't find boot file: "${BOOT_FILE}
fi

echo "  Input file: "${INPUT_FILE}
echo "  Output file: "${OUTPUT_FILE}
echo "  boot file: "${BOOT_FILE}

rm -rf ${OUTPUT_FILE}
touch ${OUTPUT_FILE}

#这里做法：拿出第一阶段的头部数据（36K）+ 第二阶段u-boot.bin数据 = SPL + uboot
cp ${BOOT_FILE} ${OUTPUT_FILE}
dd if=${INPUT_FILE} of=${OUTPUT_FILE} bs=1K seek=${UBOOT_OFFSET}

echo "Generate customized boot at: "${OUTPUT_FILE}
