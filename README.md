Patience Deck
=============
This is a collection of patience games for Sailfish OS.

It utilises [GNOME Aisleriot's](https://wiki.gnome.org/Apps/Aisleriot)
implementations of patience games and artwork.

Building and installing
-----------------------
Use [Sailfish OS Platform SDK](https://sailfishos.org/wiki/Platform_SDK)
to build this. Application SDK support is planned to be added later.

This application uses [Guile](https://www.gnu.org/software/guile/). You
can find packaging specification files for it and its dependencies in
***rpm/misc/*** directory. Build and install them and their devel
packages to your platform SDK target. Then you can build this
application. You must install the dependencies also on your device
before you can install the application.

[Harbour](https://harbour.jolla.com/) compatible packaging that bundles
the needed libraries is planned to be implemented later.
