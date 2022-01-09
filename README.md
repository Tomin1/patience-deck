Patience Deck
=============
_Patience Deck_ is a collection of patience games for _Sailfish OS_.
It reimplements game engine from
[_GNOME Aisleriot_](https://wiki.gnome.org/Apps/Aisleriot) and utilises its
implementations of patience games including manual pages and artwork.

[Github repository](https://github.com/Tomin1/patience-deck/).

Building and installing
-----------------------
This application uses [_Guile_](https://www.gnu.org/software/guile/). It is not
packaged in _Sailfish OS_. You can find packaging specification files for it and
its dependencies in _rpm/misc/_ directory.

Use either _sfdk_ command from
[_Sailfish SDK_](https://sailfishos.org/wiki/Sailfish_SDK) or
[_Platform SDK_](https://sailfishos.org/wiki/Platform_SDK) to build _Patience
Deck_ and its dependencies. Below are instructions for building with sfdk
command. Building with _Platform SDK_ needs similar steps but you should follow
its guides when building. These instructions are tested only on Linux and only
using docker version of the build engine.

### Building generic version
This version needs some support libraries that are built separately and must be
installed to SDK target and device to build and run _Patience Deck_. While
building support libraries can take a long time, following builds of _Patience
Deck_ are very quick to do.

The following instructions are for _SailfishOS-4.1.0.24-aarch64_ target but
substitute your own depending on which version of Sailfish or which
architecture you are targeting. If you switch targets, remember to clean up any
build artifacts in source directories. If you build for i486 target, see the
note below.

First set your target:

    $ sfdk config --push target SailfishOS-4.1.0.24-aarch64

Also set _output-prefix_ for packages that you build:

    $ sfdk config --global --push output-prefix "$HOME/RPMS"

Substitute your choice of directory at the end. You need to do this only once.
This is important to allow the SDK to find the dependencies later.

Now it's time to build the support libraries. You need to do this only once if
you are developing _Patience Deck_ because future builds can use the packages
that you have already built. Download the source package listed in each spec
file and extract it next to _Patience Deck_ source directory in the same parent
directory. Enter each directory and run the respective sfdk build command as
shown below.

Build support libraries starting with _gc_:

    $ sfdk --specfile ../patience-deck/rpm/misc/gc.spec build

Repeat for _libunistring_:

    $ sfdk --specfile ../patience-deck/rpm/misc/libunistring.spec build

And for _guile_:

    $ sfdk --specfile ../patience-deck/rpm/misc/guile.spec build

Go to _Patience Deck_ source directory and continue by building it.

Fetch _GNOME Aisleriot_ sources and apply patches:

    $ git submodule update --init
    $ sfdk apply

You need to do this step only once. After that _aisleriot/_ directory
contains the needed assets for _Patience Deck_.

Build the package:

    $ sfdk build

You can find the built packages in the directory you specified in
_output-prefix_ step. In addition to _patience-deck_ package you must install
also _gc_, _libunistring_ and _guile22_ packages to the device.

#### Note for i486 targets
Currently there seems to be a bug in glibc that breaks iconv but only on i486
targets. The only working way to build working libunistring and guile is to add
_/usr/lib/gconv_ to _/etc/ld.so.conf.d/gconv.conf_ in the target directory
structure on Platform SDK. This workaround does not seem to work on Sailfish SDK
at all and thus the build instructions above can not be used. This is not needed
for harbour builds as there is a working workaround for it already in
_tools/build\_deps.sh_ and that works also on Sailfish SDK.

### Building harbour version
[Harbour](https://harbour.jolla.com/) aka _Jolla Store_ compatible packaging
can be built using _--with harbour_ arguments. It builds all necessary
libraries for guile and bundles them in a single package. It may take tens of
minutes to build from scratch. Built libraries are cached so that rebuilding
_Patience Deck_ takes less time but if the files are removed, the following
build will rebuild everything.

The following instructions are for _SailfishOS-4.1.0.24-aarch64_ target but
substitute your own depending on which version of Sailfish or which
architecture you are targeting.

If you haven't installed _guile22-devel_ package to your target snapshot, you
need to patch your _mb2_ command or use a workaround as the current version of
the SDK has a bug with dependency handling. You can just comment out
_BuildRequires_ for _guile22-devel_ as a workaround. Alternatively apply the
change from [here](https://github.com/sailfishos/sdk-setup/pull/260).
Installing _guile22-devel_ package also works but to do that you must first
follow instructions to build the support libraries for the generic version.

First set your target:

    $ sfdk config --push target SailfishOS-4.1.0.24-aarch64

Fetch _GNOME Aisleriot_ sources and apply patches:

    $ git submodule update --init
    $ sfdk apply

You need to do this step only once. After that _aisleriot/_ directory
contains the needed assets for _Patience Deck_.

Build the package:

    $ sfdk build --with harbour

It will first install any missing dependencies. After that it will download and
build support libraries one-by-one which can take a long time. Last it builds
_Patience Deck_ and packages everything into one package that can be installed
on device. You may find the built package in _RPMS/_ directory or the directory
you have set your _output-prefix_ to point to.

If you need to do rebuilds, just repeat the building step. If you want to
rebuild support libraries, remove libs directory or one of its target
architecture subdirectories.

### Installing
Depending on which version you built, the resulting package is called
_patience-deck_ or _harbour-patience-deck_. If you have added your device on
your SDK settings and selected the correct device, you can deploy easily:

    $ sfdk deploy --sdk patience-deck
