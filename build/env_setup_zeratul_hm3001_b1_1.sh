if [ -f build/env_setup_zeratul_hm3001_b1_1.sh ]; then
    JOBS="8"
    BUILD_TYPE="release"
    # BUILD_TYPE="debug" #debug kernel信息会全部打印在控制台,版本使用rcS_debug这个启动脚本，默认是不启动我们的app，还会使用添加了tftpd等功能的跟文件系统，添加了一些调试相关的usb驱动
    VERSION="0.0.1"

    SOC_TYPE="SOC_T40N"
    BOARD_VERSION="HM3001" #项目名称，沿用了君正的命令为了兼容他们其他的脚本;原始SDK该值：V1
    TRANSFER_MODE="IIC"

    FLASH_TYPE="NAND"
    SENSOR_NUM="one"
    SENSOR="sc301IoT"
    SENSOR1=""
    FLASH=""
    PRODUCT_MODE=""
    SUIT_RELAY_EN=""
    STATION_AP_MODE=""
    WIFI_CHIP="bl616"
    FB_LCD=""
    TOOL_CHAIN="7.2.0"
    USER_MODE_70MAI="0"
    FACTORY_MODE_70MAI="0"

    #新增
    CUSTOM="70MAI" #用来在其他脚本里面添加定制化操作
    BOARD_VERSION_LOWER=${BOARD_VERSION,,} #转换成小写
    WDR_ENABLED="0" # 1:上电初始化成HDR模式，0：上电初始化成线性模式
    HARDWARE_VERSION="b1_1" #博流wifi的开发板
    AI_ALG="mi" #70mai mi
    IS_HASH="1" 

    KERNEL_CONFIG_70MAI="zeratul_SOC_T40_70mai_${BOARD_VERSION_LOWER}_${HARDWARE_VERSION}_defconfig"  #对应的内核配置文件zeratul_SOC_T40_70mai_hm1003_c1_1_defconfig

    KERNEL_NECESSARY_MODULE_70MAI="nfs.ko nfsv2.ko nfsv3.ko sunrpc.ko lockd.ko grace.ko usb-common.ko udc-core.ko dwc2.ko libcomposite.ko videobuf2-vmalloc.ko usb_f_uvc.ko g_webcam.ko u_ether.ko u_serial.ko usbcore.ko usb_f_acm.ko usb_f_ecm.ko usb_f_ecm_subset.ko usb_f_mass_storage.ko usb_f_obex.ko usb_f_rndis.ko  usb_f_serial.ko usbhid.ko g_mass_storage.ko g_ether.ko g_serial.ko" #存放在system不会被删掉
    KERNEL_UNNECESSARY_MODULE_70MAI="" #release版本会从system里面删除掉
    KERNEL_MODULE_70MAI="\"${KERNEL_NECESSARY_MODULE_70MAI} ${KERNEL_UNNECESSARY_MODULE_70MAI}\"" #存放在system,其他ko编译出来默认存放在rootfs里面

    while getopts "j:s:Uf" arg
    do
        case $arg in
        j)
            JOBS=$OPTARG
            ;;
        s)
            SENSOR=$OPTARG
            ;;
        U)
            USER_MODE_70MAI="1"
            ;;
        f)
            FACTORY_MODE_70MAI="1"
            ;;
        ?)
            echo "unkonw argument"
            exit 1
            ;;
        esac
    done

    source build/env_setup_zeratul_hm3001.sh
else

    echo "Please run source at top of isvp"
fi
