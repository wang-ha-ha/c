#!/bin/bash
type 7z >/dev/null 
if [ $? -eq 1 ];then
    echo "no 7z in your env"
    exit -1;
fi

echo "Attention! The unzipped folder will be deleted!   y/n?"
read -n 1
echo ""
if [ $REPLY = n ];then
    exit
fi

DELETE_DIR=($(ls ))
for DELETE in ${DELETE_DIR[@]};
do
    if [ -d ${DELETE} ];then
        rm -rf ${DELETE}
    fi

done

DIR=($(find -name "*.7z"))
for DIR_NAME in ${DIR[@]};
do
    DIR_NAME=${DIR_NAME%???}
    if [ ! -d ${DIR_NAME} ];then
        echo "uncompress "${DIR_NAME}".7z to "${DIR_NAME}
        7z x ${DIR_NAME}.7z -o${DIR_NAME}/ > /dev/null
    else
        echo ${DIR_NAME} "already exist"
    fi
done

FILE=($(find -name "*.7z"))
for FILE_UNCOMP in ${FILE[@]};
do
    if [ ! -d ${FILE_UNCOMP%???} ];then
        echo "uncompress "${FILE_UNCOMP}
        7z x ${FILE_UNCOMP} -o${FILE_UNCOMP%/*.7z}/ > /dev/null
    else
        echo ${FILE_UNCOMP%???} "already exist"
    fi
done

