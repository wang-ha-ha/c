TOPDIR=`pwd`
echo TOPDIR=${TOPDIR} > .config
echo APPS=lib_a_so lib_b_so app_a app_b>> .config

export LD_LIBRARY_PATH=${TOPDIR}/out/lib/:${LD_LIBRARY_PATH}