#!/bin/bash

# Copyright (c) 2014 AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

function Usage() {
	set +ex
	echo >&2 "
Runs GTest executable lsftest

Usage: $(basename -- "$0") [-d lsf_dist] [-c configfile] [-s alljoyn_dist]
where
	-d lsf_dist	# path to the service_framework/build/linux tree, used to find lighting_controller_service and lamp_service exe
	-c configfile	# name of config file
			#	default 'lsftest.conf'
        -s alljoyn_dist # path to the service_framework/build/linux/x86(or x86_64)/dist tree, used to find the alljoyn and the base services libraries
"
	exit 2
}

: read commandline options

set -e

lsf_dist=''
configfile='*.conf'
alljoyn_dist=''

while getopts s:c:d: option
do
	case "$option" in
	( s ) alljoyn_dist="$OPTARG" ;;
	( c ) configfile="$OPTARG" ;;
	( d ) lsf_dist="$OPTARG" ;;
	( \? ) Usage ;;
	esac
done

gtests=''
if test "$OPTIND" -gt 0 -a "$OPTIND" -le $#
then
	shift ` expr $OPTIND - 1 `
	while test $# -ge 1
	do
		case "$1" in
		( *[!a-zA-Z0-9_-]* | '' ) echo >&2 "error, $1:  allowed characters are [a-zA-Z0-9_-]" ; Usage ;;
		( * ) gtests="$gtests $1" ;;
		esac
		shift
	done
fi

if test -z "$gtests"
then
	gtests="lsftest"
fi

: check commandline options

ckControllerbin() {
	ckvar=$1
	binvar=$2
	eval ckval="\$$1"
	binval=` cd "$ckval/standard_core_library/lighting_controller_service/bin" > /dev/null && pwd ` || : ok
	if test -z "$binval"
	then
		echo >&2 "error, could not find 'standard_core_library/lighting_controller_service/bin' subdirectory in $ckvar=$ckval"
		exit 2
	fi
	eval $binvar="'$binval'"
}

ckLampbin() {
	ckvar=$1
	binvar=$2
	eval ckval="\$$1"
	binval=` cd "$ckval/thin_core_library/lamp_service/bin" > /dev/null && pwd ` || : ok
	if test -z "$binval"
	then
		echo >&2 "error, could not find 'thin_core_library/lamp_service/bin' subdirectory in $ckvar=$ckval"
		exit 2
	fi
	eval $binvar="'$binval'"
}

# lsf_dist
if test -z "$lsf_dist"
then
	echo >&2 "error, -d path is required"
	Usage
fi
ckControllerbin lsf_dist controller_bin
ckLampbin lsf_dist lamp_bin

# alljoyn_dist
if test -z "$alljoyn_dist"
then
	echo >&2 "error, -s path is required"
	Usage
fi

gtest_bin=$( cd "$(dirname -- "$0")" > /dev/null && pwd )
if test -z "$gtest_bin" -o "gtest_bin" = /
then
	: unknown error trap
	exit 2
fi

options="-f"
gtest_bin_p="$gtest_bin"
export HOME="$PWD"

AllJoynLibDir="$alljoyn_dist/cpp/lib"
if [ -f "$AllJoynLibDir/liballjoyn.so" ]; then
    ls -ld "$AllJoynLibDir/liballjoyn.so"
    export LD_LIBRARY_PATH="$AllJoynLibDir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
else
    echo "warning, liballjoyn.so does NOT exist!"
fi

AboutLibDir="$alljoyn_dist/about/lib"
if [ -f "$AboutLibDir/liballjoyn_about.so" ]; then
    ls -ld "$AboutLibDir/liballjoyn_about.so"
    export LD_LIBRARY_PATH="$AboutLibDir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
else
    echo "warning, liballjoyn_about.so does NOT exist!"
fi

ConfigLibDir="$alljoyn_dist/config/lib"
if [ -f "$ConfigLibDir/liballjoyn_config.so" ]; then
    ls -ld "$ConfigLibDir/liballjoyn_config.so"
    export LD_LIBRARY_PATH="$ConfigLibDir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
else
    echo "warning, liballjoyn_config.so does NOT exist!"
fi

NotificationLibDir="$alljoyn_dist/notification/lib"
if [ -f "$NotificationLibDir/liballjoyn_notification.so" ]; then
    ls -ld "$NotificationLibDir/liballjoyn_notification.so"
    export LD_LIBRARY_PATH="$NotificationLibDir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
else
    echo "warning, liballjoyn_notification.so does NOT exist!"
fi

ServicesCommonLibDir="$alljoyn_dist/services_common/lib"
if [ -f "$ServicesCommonLibDir/liballjoyn_services_common.so" ]; then
    ls -ld "$ServicesCommonLibDir/liballjoyn_services_common.so"
    export LD_LIBRARY_PATH="$ServicesCommonLibDir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
else
    echo "warning, liballjoyn_services_common.so does NOT exist!"
fi

echo "# runall test plan:"
ls >/dev/null -ld "$controller_bin/lighting_controller_service" || {
	echo >&2 "error, lighting_controller_service exe not found"
	exit 2
}
echo "# $controller_bin/lighting_controller_service $options > controller.log &"

ls >/dev/null -ld "$lamp_bin/lamp_service" || {
	echo >&2 "error, lamp_service exe not found"
	exit 2
}
echo "# $lamp_bin/lamp_service > lamp.log &"

for i in $gtests
do
	ls >/dev/null -ld "$gtest_bin/$i" || ls >/dev/null -ld "$gtest_bin/$i.exe" || {
		echo >&2 "error, $i exe not found"
		exit 2
	}
	c="$( echo "$configfile" | sed -e 's,\*,'$i',g' )"
	ls >/dev/null -ld "$c" || {
		echo >&2 "error, configfile $c not found"
		exit 2
	}
	echo "# python test_harness.py -c $c -t $i -p $gtest_bin_p > $i.log"
done

: begin

export ER_DEBUG_ALL=0

: start lighting_controller_service and lamp_service

rm -f controller.log
rm -f lamp.log

killall -v lighting_controller_service || : ok
(
	set -ex
	cd "$controller_bin"
	pwd
	date

	./lighting_controller_service $options; xit=$?

	date
	set +x
	echo exit status $xit
) > controller.log 2>&1 </dev/null &

killall -v lamp_service || : ok
(
	set -ex
	cd "$lamp_bin"
	pwd
	date

        rm -f ajlite.nvram
	./lamp_service; xit=$?

	date
	set +x
	echo exit status $xit
) > lamp.log 2>&1 </dev/null &

sleep 5

: run gtest executables

xit=0
for i in $gtests
do
	# configfile for gtest $i
	c="$( echo "$configfile" | sed -e 's,\*,'$i',g' )"

	rm -f $c.t
        cat $c > $c.t
	rm -f "$i.log"

	sleep 5
	set -x
	: run $i

	date
	python -u test_harness.py -c $c.t -t $i -p "$gtest_bin_p" > "$i.log" 2>&1 < /dev/null || : exit status is $? / IGNORE IT
	date
	set +x
	tail -1 "$i.log" | grep "exiting with status 0" || xit=1

	case "$xit" in
	0 ) echo $i PASSED ;;
	* ) echo $i FAILED, see $i.log for info ;;
	esac

	sleep 5
done

sleep 5
killall -v lighting_controller_service || : ok
sleep 5
killall -v lamp_service || : ok
sleep 5

echo exit status $xit
exit $xit
