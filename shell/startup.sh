#!/bin/bash

echo "`date '+%Y/%m/%d %k:%M:%S.%N'` system startup  BEGIN..."

conf_file="$HOME/conf/config.ini"
[ ! -f $conf_file ] && echo "$conf_file is not exist" && exit 1;

. $conf_file

commons_file="$HOME/bin/commons"
[ ! -f $commons_file ] && echo "$commons_file is not exist" && exit 1;

. $commons_file

sysflag="gta"
if [ "`echo $0 | sed 's/tdf.sh//g'`"  != "$0" ];then
	sysflag="tdf"
fi

ethdev=${ethdev:-eno1}
writeflag=${writeflag:-1}
workroot=${workroot:-/stock/work}
writeusr=${writeusr:-0}

ints_file="$HOME/conf/ints_$sysflag.json"
ints_bin="$HOME/bin/ints_$sysflag"
ints_log="$HOME/bin/log/ints_"$sysflag"_`date '+%Y%m%d'`.log"

index_bin="$HOME/bin/index_$sysflag"
index_log="$HOME/bin/log/index_"$sysflag"_`date '+%Y%m%d'`.log"

disp_file="$HOME/conf/disp.json"
user_file="$HOME/conf/user_privilege.json"
cfg_file="$HOME/conf/cfg.json"

dat2cli_bin="$HOME/bin/dat2cli"
moni_bin="$HOME/bin/moni.sh"
pidof_bin="/usr/sbin/pidof"

dat2cli_log="$HOME/bin/log/dat2cli_`date '+%Y%m%d'`.log"
moni_log="$HOME/bin/log/moni_`date '+%Y%m%d'`.log"

[ ! -f $ints_file ] && echo "$ints_file is not exist" && exit 1;
[ ! -f $ints_bin ] && echo "$ints_bin is not exist" && exit 1;
[ ! -f $index_bin ] && echo "$index_bin is not exist" && exit 1;

[ ! -f $cfg_file ] && echo "$cfg_file is not exist" && exit 1;
[ ! -f $disp_file ] && echo "$disp_file is not exist" && exit 1;
[ ! -f $user_file ] && echo "$user_file is not exist" && exit 1;

[ ! -f $dat2cli_bin ] && echo "$dat2cli_bin is not exist" && exit 1;
[ ! -f $moni_bin ] && echo "$moni_bin is not exist" && exit 1;
[ ! -f $pidof_bin ] && echo "$pidof_bin is not exist" && exit 1;

workd31=${workd31:-"/data/work"}
etflist=${etflist:-"510050,510180,510300,510500"}
etfpath=${etfpath:-$HOME/conf/etf}
index_stat_delay=${index_stat_delay:-"1000:300,1000:300"}


[ ! -d $workroot ] && echo "dir $workroot is not exist" && exit 1;
[ ! -d $workd31 ] && echo "dir $workd31 is not exist" && exit 1;
[ ! -d $etfpath ] && echo "dir $etfpath is not exist" && exit 1;

my_name=`whoami`
my_name=${my_name:-$USER}
my_flag=""

max_mq_msg_len=`grep "SysMqMaxLen" $cfg_file | sed 's/[^0-9]//g'`
if [ -z $max_mq_msg_len ];then
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` file $cfg_file SysMqMaxLen config error"
	exit 4;
fi


ints_bin_base=`basename $ints_bin`
index_bin_base=`basename $index_bin`

pids=`$pidof_bin -x $ints_bin_base $index_bin_base dat2cli moni.sh`

##如果有指定进程在运行，则检查是否有本用户的进程存在，如果有则提示退出
mypid=`check_mypid_exist $pids`
if [ $mypid -ne 0 ];then
	belong_cmd=`belong_cmd_mypid $mypid`
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` pid $mypid cmd=$belong_cmd user=$my_name is running"
	exit 2
fi

##清除mq队列
remove_mymq_all

##清除信号量
remove_mysem_all

##清空disp.json文件
cat > $disp_file << 'EOF'
{
    "users": ""
}
EOF

cd $HOME/bin
	
nohup stdbuf --output=L --error=L $ints_bin -w$writeflag -o$workroot -c$ints_file -r$disp_file -d$workd31 -l$max_mq_msg_len 1>>$ints_log 2>&1 &
sleep 1
$pidof_bin -x $ints_bin_base

if [ $? -ne 0 ]; then
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` $ints_bin_base is startup FAIL..";
	echo "$ints_bin -w$writeflag -o$workroot -c$ints_file -r$disp_file -d$workd31 -l$max_mq_msg_len"
	exit 3;
fi
echo "`date '+%Y/%m/%d %k:%M:%S.%N'` $ints_bin_base is startup SUCESS.."
	
##启动D31统计程序
nohup stdbuf --output=L --error=L $index_bin -w3 -s$workroot -o$workd31 -e$index_stat_delay -L$etflist -E$etfpath 1>>$index_log 2>&1 &
sleep 1
$pidof_bin -x $index_bin_base
if [ $? -ne 0 ]; then
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` $index_bin_base is startup FAIL..";
	echo "$index_bin -w3 -s$workroot -o$workd31 -e$index_stat_delay -L$etflist -E$etfpath"
	exit 3;
fi
echo "`date '+%Y/%m/%d %k:%M:%S.%N'` $index_bin_base is startup SUCESS.."

nohup stdbuf --output=L --error=L $dat2cli_bin -w$writeusr -o$workroot -p$cfg_file -r$disp_file -u$user_file 1>>$dat2cli_log 2>&1 &
sleep 1
$pidof_bin -x dat2cli
if [ $? -ne 0 ]; then
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` dat2cli is startup FAIL..";
	echo "$dat2cli_bin -w$writeusr -o$workroot -p$cfg_file -r$disp_file -u$user_file"
	exit 3;
fi

echo "`date '+%Y/%m/%d %k:%M:%S.%N'` dat2cli is startup SUCESS.."

nohup stdbuf --output=L --error=L $moni_bin $ethdev 1>>$moni_log 2>&1 &

sleep 1
$pidof_bin -x moni.sh
if [ $? -ne 0 ]; then
	echo "`date '+%Y/%m/%d %k:%M:%S.%N'` moni.sh startup FAIL..";
	echo "$moni_bin $ethdev"
	exit 3;
fi

echo "`date '+%Y/%m/%d %k:%M:%S.%N'` moni.sh is startup SUCESS.."

echo "`date '+%Y/%m/%d %k:%M:%S.%N'` system startup  OK..."

exit 0
