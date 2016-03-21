# Info

**This project has been deprecated in favor of the newer [The AllJoyn / Muzzley Gateway Connector](https://github.com/muzzley/alljoyn-muzzley-gateway-connector).**

# AllJoyn Muzzley Connector #

This repo contains the code for the AllJoyn-Muzzley connector C++ client library.

# How do I get set up?

##Dependencies

* libcap-dev
* libxml2-dev
* libgupnp-1.0-dev
* libgssdp-1.0-dev
* libglib2.0-dev
* libssl-dev
* libcrypto-dev
* [Muzzley](https://github.com/muzzley/muzzley-client-cpp)

Install the Muzzley C++ library following the README file and enable HTTP and logs.

#Build Instructions for x86_64

1) Follow the instructions in [here](https://allseenalliance.org/docs-and-downloads/documentation/configuring-build-environment-linux-platform) to set up your ubuntu machine to build AllJoyn.

2) Create a new folder named "alljoyn-muzzley" on your home directory.

3) Use the "alljoyn-muzzley" folder as your workspace and create the following directory structure to setup your environment.

```
core/

    gwagent/   (https://git.allseenalliance.org/gerrit/gateway/gwagent.git)

	service_framework/   (https://git.allseenalliance.org/gerrit/lighting/service_framework.git)

	ajtcl/   (https://git.allseenalliance.org/gerrit/core/ajtcl.git)

	alljoyn/ (https://git.allseenalliance.org/gerrit/core/alljoyn.git)

base_tcl/ (https://git.allseenalliance.org/gerrit/services/base_tcl.git)

base/ (https://git.allseenalliance.org/gerrit/services/base.git)

services/

       base_tcl/ (https://git.allseenalliance.org/gerrit/services/base_tcl.git)

       base/ (https://git.allseenalliance.org/gerrit/services/base.git)

```

4) Open the service_framework folder and Backup the SConscript file:

> $ cd core/service_framework/

> $ cp SConscript SConscript.bak

5) Replace the SConscript file with the one provided on this repo.

6) Open the "standard_core_library" and paste there the "alljoyn_muzzley_connector" folder from this repo, containing the AllJoyn-Muzzley connector:

> $ cd core/service_framework/standard_core_library

> $ cp alljoyn_muzzley_connector $HOME/alljoyn-muzzley/core/service_framework/standard_core_library

8) Open a command terminal and from under the core/alljoyn/ directory, run the command "scons" to build the core modules for x86_64 target.

> $ scons V=1 OS=linux CPU=x86_64 BINDINGS="cpp" WS=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps"

9) Open a command terminal and from under the core/service_framework/ directory, run the command "scons" to build the lighting service framework for x86_64 target.

> $ scons V=1 OS=linux CPU=x86_64 BINDINGS="cpp" WS=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps"

11) If needed, run the following "scons" command to clean the build files

> $ scons -c


##Run Tests

1) Launch AllJoyn Daemon:

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/cpp/lib/

> $ cd core/alljoyn/build/linux/x86_64/debug/dist/cpp/bin

> $ ./alljoyn-daemon

2) Controller Service:

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/cpp/lib/

> $ cd core/service_framework/build/linux/standard_core_library/lighting_controller_service/bin

> $ ./lighting_controller_service -f

3) Lamp (Virtual Lamp):

> $ cd core/service_framework/build/linux/thin_core_library/lamp_service/bin/

> $ ./lamp_service


4) Connector:

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/cpp/lib/

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/controlpanel/lib/

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/notification/lib/

> $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/alljoyn-muzzley/core/alljoyn/build/linux/x86_64/debug/dist/services_common/lib/

> $ cd core/service_framework/build/linux/standard_core_library/alljoyn-muzzley-connector/connector

> $ ./alljoyn_muzzley_connector


#Build Instructions for OpenWRT

Prepare the OpenWRT Toolchain 

1) Download the OpenWRT source code from: 

> $ git clone git://git.openwrt.org/14.07/openwrt.git

2) Download the config file for the correspondent hardware where the OpenWRT will be run from [here](https://downloads.openwrt.org/barrier_breaker/14.07/) and paste it on the root of the openwrt project.

3) Copy downloaded config file to .config (for example):

> $ cp config.ar71xx_generic .config

4) Copy feeds.conf.default to feeds.conf (if not already done):

> $ cp feeds.conf.default feeds.conf

5) Add the following line to the end of the file:
```
src-git alljoyn https://git.allseenalliance.org/gerrit/core/openwrt_feed;barrier_breaker

```

6) Update the feed information:

> $ ./scripts/feeds update -a

    
7) Add the the packages from the feeds to build system (luci interface is not needed but recommended to configure the router using a web interface):

> $ ./scripts/feeds install -a -p alljoyn

> $ ./scripts/feeds install libgupnp

> $ ./scripts/feeds install libgssdp

> $ ./scripts/feeds install -a -p luci

8) Enable AllJoyn in the build:

> $ make menuconfig

```
     Networking --->
          < > alljoyn --->
               < > alljoyn-about
               < > alljoyn-c
               < > alljoyn-config
                    < > alljoyn-config-samples
               < > alljoyn-controlpanel
                    < > alljoyn-controlpanel-samples
               < > alljoyn-notification
                    < > alljoyn-notification-samples
               < > alljoyn-onboarding
                    < > alljoyn-onboarding-samples
               < > alljoyn-sample_apps
               < > alljoyn-samples
               < > alljoyn-service_common
     LuCI --->
          < > Collections --->
               < > luci     
          < > Themes --->
               < > luci-themes-openwrt
     Libraries --->
          < > libxml2
          < > libgupnp
          < > libgssdp
          
```
9) Make the firmware image including the correspondent configuration using the command:
> $ make

10) Flash it in the router firmware and wait for reboot.

11) SSH to your router with two different shells and execute the following commands on each one to test the AllJoyn framework:
> $ /usr/bin/AboutService

> $ /usr/bin/AboutClient

12) Prepare the OpenWRT toolchain to be able to cross-compile the Muzzley library and the AllJoyn-Muzzley connector for the specific router hardware target.

> $ export PATH=$PATH:$HOME/openwrt/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin

> $ export STAGING_DIR=$HOME/openwrt/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2

> $ export AJ_ROOT="$HOME/alljoyn-muzzley"

> $ export TARGET_OS="openwrt"

> $ export TARGET_CPU="openwrt"

> $ export AJ_DIST="$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/debug/dist"

> $ export OPENWRT_BASE="$HOME/openwrt"

> $ export OPENWRT_TOOLCHAIN_BASE=$OPENWRT_BASE/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/

> $ export OPENWRT_TARGET_BASE=$OPENWRT_BASE/staging_dir/target-mips_34kc_uClibc-0.9.33.2/

> $ export TARGET=mips-openwrt-linux-uclibc


13) Cross-compile Muzzley Library to OpenWRT.

> $ autoreconf -fi

> $ ./configure --build=x86_64-unknown-linux-gnu --host=mips-openwrt-linux-uclibc --prefix=$HOME/openwrt-muzzley-lib CXXFAGS="-std=c++0x" --enable-http --enable-logs

> $ make CC=mips-openwrt-linux-uclibc-gcc LD=mips-openwrt-linux-uclibc-ld

> $ make install


14) Copy the compiled Muzzley library to the /usr/lib folder on the OpenWRT toolchain folder.

> $ cd $HOME/openwrt-muzzley-lib/lib

> $ scp -r . $OPENWRT_TOOLCHAIN_BASE/usr/lib

> $ cd $HOME/openwrt-muzzley-lib/include

> $ scp -r . $OPENWRT_TOOLCHAIN_BASE/usr/include


15) Copy the compiled Muzzley library to the /usr/lib folder on the router.

> $ cd $HOME/openwrt-muzzley-lib/lib

> $ rm libmuzzley.a

> $ scp -r . root@192.168.1.1:/usr/lib


16) Cross-compile AllJoyn-Muzzley Connector to OpenWRT.

> $ scons V=1 ICE=off BR=on BT=off WS=off CPU=openwrt OS=openwrt BINDINGS="cpp" SERVICES="about,notification,controlpanel,config,onboarding,sample_apps" TARGET_CFLAGS="-Os -pipe -mips32r2 -mtune=74kc -fPIC -fno-caller-saves -fhonour-copts -Wno-error=unused-but-set-variable -msoft-float" "TARGET_CC=$TARGET-gcc" "TARGET_CXX=$TARGET-g++" "TARGET_AR=$TARGET-ar" "TARGET_RANLIB=$TARGET-ranlib" "TARGET_LINK=$TARGET-gcc" "TARGET_CPPFLAGS=-I$OPENWRT_TARGET_BASE/usr/include -I$OPENWRT_TARGET_BASE/include -I$OPENWRT_TOOLCHAIN_BASE/usr/include -I$OPENWRT_TOOLCHAIN_BASE/include" "TARGET_PATH=$OPENWRT_TOOLCHAIN_BASE/bin:$OPENWRT_BASE/staging_dir/host/bin:$PATH" "STAGING_DIR=$OPENWRT_TARGET_BASE" "TARGET_LINKFLAGS=-L$OPENWRT_TARGET_BASE/usr/lib" "CXXFLAGS=$CXXFLAGS -I$AJ_DIST/cpp/inc -I$AJ_DIST/about/inc -I$AJ_DIST/services_common/inc -I$AJ_DIST/notification/inc -I$AJ_DIST/controlpanel/inc -I$AJ_DIST/services_common/inc" "LDFLAGS=$LDFLAGS -L$AJ_DIST/cpp/lib -L$AJ_DIST/about/lib -L$AJ_DIST/services_common/lib -L$AJ_DIST/notification/lib -L$AJ_DIST/controlpanel/lib"

17) Replace the AllJoyn library installed on the router (14.06) with the one cross-compiled with the previous command (14.12) .

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/cpp/lib

> $ scp liballjoyn.so root@192.168.1.1:/usr/lib

> $ scp liballjoyn_about.so root@192.168.1.1:/usr/lib

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/config/lib

> $ scp liballjoyn_config.so root@192.168.1.1:/usr/lib

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/controlpanel/lib

> $ scp liballjoyn_controlpanel.so root@192.168.1.1:/usr/lib

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/notification/lib

> $ scp liballjoyn_notification.so root@192.168.1.1:/usr/lib

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/onboarding/lib

> $ scp liballjoyn_onboarding.so root@192.168.1.1:/usr/lib

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/services_common/lib

> $ scp liballjoyn_services_common.so root@192.168.1.1:/usr/lib


18) Replace the AllJoyn Daemon installed on the router (14.06) with the one cross-compiled with the previous command (14.12).

> $ cd $HOME/alljoyn-muzzley/core/alljoyn/build/openwrt/openwrt/debug/dist/cpp/bin

> $ scp alljoyn-daemon root@192.168.1.1:/usr/bin

19) Reboot the router.

20) Copy the Alljoyn-Muzzley connector to the /bin folder on the router.

> $ cd core/service_framework/build/linux/standard_core_library/alljoyn_muzzley_connector/connector

> $ scp alljoyn_muzzley_connector root@192.168.1.1:/bin

21) Run it.

> 1 - The Alljoyn Daemon should be running (in only one machine) on local network.

> 2 - Start the Alljoyn-Muzzley-Connector. (If it does not start, delete the “muzzley_semaphore.txt” file, on the same folder.)

> 3 - Open the Muzzley app on the phone and add the channel using the “+”. (Remove it first, if the channel has been already added before. To avoid the “error:417”, the channel must be added on the phone before trying to add new components.)

> 4 - Open the Luminaire App on one Android phone connected to the same network, and enable the controller service. (The Muzzley app must be run on a different phone.)

> 5 - Now the Alljoyn plugs or lights can be connected to power. (They will announce themselves on the local network and the Muzzley connector will send that info to the Muzzley cloud.)

> 6 - Check if the lights can be controlled using the "LSF Sample App", and the plugs or with with the "Alljoyn On" application.

> 7 - Open the Muzzley application on the phone and start to interact with your devices.

22) Command line options:

> The following list of options is only used for development and troubleshooting purposes. To connect with the Muzzley cloud, all default values are already being applied, and don’t need any change.

| Command | Description |
| ------------- | ----------- |
| --core-endpointhost | set the Muzzley Core Endpointhost |
| --api-endpointhost | set the Muzzley API Endpointhost |
| --api-port | set the Muzzley API Port number |
| --manager-endpointhost | set the Muzzley Manager Endpointhost |
| --manager-port | set the Muzzley Manager Port number |
| --lighting-profileid | set the Muzzley Lighting Profileid |
| --lighting-app-token | set the Muzzley Lighting AppToken |
| --lighting-port | set the Muzzley Lighting UPNP Port number |
| --plugs-profileid | set the Muzzley Plugs Profileid |
| --plugs-app-token | set the Muzzley Plugs AppToken |
| --plugs-port | set the Muzzley Plugs UPNP Port number |
| --color-mode | set the Muzzley Interface Color Mode |
| --help | show this help text |
