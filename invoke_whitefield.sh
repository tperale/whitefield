#!/bin/bash

RED="\033[0;31m"
BLUE="\033[0;34m"
BLINK="\033[0;5m"
NC="\033[0m"

if [ "$1" == "gdb" ]; then
	shift
	cmdprefix="gdb --args"
fi

[[ ! -f "$1" ]] && echo "Usage: $0 <wf_config_file>" && exit
cfgfile=$(pwd)"/$1"

cd $(dirname $0)
[[ ! -f "config.inc" ]] && echo "Need to start whitefield from base folder!!" && exit 1
. config.inc

export LD_LIBRARY_PATH=$AIRLINE_NS3/build/lib:$BINDIR
export FORKER=$BINDIR/wf_forker
export LOGPATH=log
export MONITOR_PORT=$MONITOR_PORT
export AIRLINE_ERR=$LOGPATH/airline_error.log
export AIRLINE_LOG=$LOGPATH/airline.log
#export NS_LOG="*=level_warn:LrWpanMac=level_all"
rm -f $LOGPATH/*.log 2>/dev/null

diagnose() {
	grep "^ERROR" $AIRLINE_LOG
	echo -e "${BLINK}Start Failed.${NC}"
	echo "1. Check logs in folder:$LOGPATH."
	echo "2. Enable and check coredumps."
	exit 1
}

func_childret() {
	kill -0 $wf_ps 2>/dev/null
	[[ $? -ne 0 ]] && diagnose
	jobs -l | grep "Running" >/dev/null
	echo "Started OK"
	echo "Use './scripts/monitor.sh' to check status."
	exit $?
}

wfpid=$(pgrep -u $(whoami) -x whitefield)
[[ $? -eq 0 ]] && echo "Whitefield(pid=$wfpid) is already in execution!" && exit 1
mkdir $LOGPATH pcap 2>/dev/null
if [ "$cmdprefix" == "" ]; then #Regular execution
	trap func_childret SIGCHLD
	set -m
	$BINDIR/whitefield $* >$AIRLINE_LOG 2>&1 &
	wf_ps=$!
	sleep 1
	echo
else #GDB execution
	$cmdprefix $BINDIR/whitefield $*
fi
