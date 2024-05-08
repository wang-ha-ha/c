#!/bin/bash
logger_info()
{
    if [ $# -ne 2 ]; then
        echo "input param $#"
        return 1
    fi
    case $1 in
    error_info)    echo -e "\033[31m"[ERROR]$2"\033[0m"
    ;;
    warning_info)  echo -e "\033[33m"[WARN]$2"\033[0m"
    ;;
    normal_info)   echo -e "\033[32m"[INFO]$2"\033[0m"
    ;;
    *) echo "not support info $1 !!!"
    ;;
    esac

    return 0
}

#logger_info error_info "test error"
#logger_info warning_info "test warning"
#logger_info normal_info "test normal"
#logger_info normal_info "test normal" 123
