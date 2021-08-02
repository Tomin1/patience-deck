Patience Deck
=============
Patience Deck is a collection of patience games for Sailfish OS.
It reimplements game engine from [GNOME
Aisleriot](https://wiki.gnome.org/Apps/Aisleriot) and utilises its
implementations of patience games including manual pages and artwork.

[Github repository](https://github.com/Tomin1/patience-deck/).

Building and installing
-----------------------
Use [Sailfish OS Platform SDK](https://sailfishos.org/wiki/Platform_SDK)
to build this. Proper [Application
SDK](https://sailfishos.org/wiki/Application_SDK) support is planned to
be added soon.

This application uses [Guile](https://www.gnu.org/software/guile/). You
can find packaging specification files for it and its dependencies in
***rpm/misc/*** directory.
[Build](https://sailfishos.org/wiki/Building_packages) and install them
and their devel packages (gc, gc-devel, libunistring,
libunistring-devel, guile22, guile22-devel) to your platform SDK target.
Build guile last as it depends on the other packages. Then you can build
Patience Deck. You must install the dependencies (gc, libunistring and
guile22) also on your device or emulator before you can install this
application.

[Harbour](https://harbour.jolla.com/) compatible packaging can be built
using "--with harbour" arguments. It builds all necessary libraries for
guile and bundles them in single package. It may take tens of minutes to
build from scratch.
