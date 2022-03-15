#!/bin/bash
#
# Build all libs so that they can be shipped with the application

GC=gc-8.0.4
LIBUNISTRING=libunistring-0.9.10
GMP=gmp-6.2.1
LIBTOOL=libtool-2.4.6
LIBFFI=libffi-3.3
GUILE=guile-2.2.7

_PREFIX=${CACHE}/built
_LIBDIR=${CACHE}/built/usr/share/harbour-patience-deck/lib
_INCLUDEDIR=${CACHE}/built/include

SOURCES=(
    https://www.hboehm.info/gc/gc_source/${GC}.tar.gz
    https://ftp.gnu.org/gnu/libunistring/${LIBUNISTRING}.tar.gz
    https://gmplib.org/download/gmp/${GMP}.tar.xz
    https://ftp.gnu.org/gnu/libtool/${LIBTOOL}.tar.gz
    https://sourceware.org/pub/libffi/${LIBFFI}.tar.gz
    https://ftp.gnu.org/gnu/guile/${GUILE}.tar.gz
)

download_sources() {
    echo "Downloading..."
    mkdir -p ${CACHE}/
    for source in ${SOURCES[@]}
    do
        if [ ! -e "${CACHE}/${source##*/}" ]
        then
            curl -o "${CACHE}/${source##*/}" "${source}"
        fi
    done
    echo "Done downloading"
}

verify() {
    echo "Verifying..."
    sha256sum -c << EOF
436a0ddc67b1ac0b0405b61a9675bca9e075c8156f4debd1d06f3a56c7cd289d  gc-8.0.4.tar.gz
a82e5b333339a88ea4608e4635479a1cfb2e01aafb925e1290b65710d43f610b  libunistring-0.9.10.tar.gz
fd4829912cddd12f84181c3451cc752be224643e87fac497b69edddadc49b4f2  gmp-6.2.1.tar.xz
e3bd4d5d3d025a36c21dd6af7ea818a2afcd4dfc1ea5a17b39d7854bcd0c06e3  libtool-2.4.6.tar.gz
72fba7922703ddfa7a028d513ac15a85c8d54c8d67f55fa5a4802885dc652056  libffi-3.3.tar.gz
44b4c5fbbe257ccdebea18420212c9b3e90c3c86a54920d8554039fc6769a007  guile-2.2.7.tar.gz
EOF
    echo "Done verifying"
}

unpack() {
    for source in ${SOURCES[@]}
    do
        echo "Unpacking ${source##*/}"
        tar xf "${CACHE}/${source##*/}"
    done
}

build_bdwgc() {
    if [ ! -e ${_LIBDIR}/libgc.so ] || [ ! -e ${GC}/ ]
    then
        echo "Building libgc..."
        pushd ${GC}/
        autoreconf -i -f

        ./configure \
                --disable-docs \
                --disable-static \
                --enable-cplusplus \
                --enable-large-config \
                --enable-threads=posix \
                --prefix="/usr/share/harbour-patience-deck/"

        make $@
        make DESTDIR=${_PREFIX}/ install
        popd
        echo "Built libgc"
    else
        echo "libgc already built"
    fi
} # bdwgc

build_libunistring() {
    if [ ! -e ${_LIBDIR}/libunistring.so ] || [ ! -e ${LIBUNISTRING}/ ]
    then
        echo "Building libunistring..."
        pushd ${LIBUNISTRING}/
        LD_LIBRARY_PATH=/usr/lib/gconv
        export LD_LIBRARY_PATH
        ./configure \
                --disable-rpath \
                --disable-static \
                --prefix="/usr/share/harbour-patience-deck/"
        make $@
        make DESTDIR=${_PREFIX}/ install
        export -n LD_LIBRARY_PATH
        popd
        echo "Built libunistring"
    else
        echo "libunistring already built"
    fi
} # libunistring

build_libgmp() {
    if [ ! -e ${_LIBDIR}/libgmp.so ] || [ ! -e ${GMP}/ ]
    then
        echo "Building libgmp..."
        pushd ${GMP}/
        autoreconf -i -f
        ./configure \
                --disable-docs \
                --disable-static \
                --enable-fat \
                --enable-cxx \
                --prefix="/usr/share/harbour-patience-deck/"
        make $@
        make DESTDIR=${_PREFIX}/ install
        popd
        echo "Built libgmp"
    else
        echo "libgmp already built"
    fi
} # libgmp

build_libltdl() {
    if [ ! -e ${_LIBDIR}/libltdl.so ] || [ ! -e ${LIBTOOL}/ ]
    then
        echo "Building libltdl..."
        pushd ${LIBTOOL}/libltdl
        autoreconf
        ./configure \
                --disable-static \
                --enable-ltdl-install \
                --prefix="/usr/share/harbour-patience-deck/"
        make $@
        make DESTDIR=${_PREFIX}/ install
        popd
        echo "Built libltdl"
    else
        echo "libltdl already built"
    fi
} # libltdl

build_libffi() {
    if [ ! -e ${_LIBDIR}/libffi.so ] || [ ! -e ${LIBFFI}/ ]
    then
        echo "Building libffi..."
        pushd ${LIBFFI}/
        autoreconf -i -f
        ./configure \
                --disable-docs \
                --disable-multi-os-directory \
                --disable-static \
                --libdir=/lib \
                --exec-prefix=/usr/share/harbour-patience-deck \
                --prefix=/usr/share/harbour-patience-deck
        make $@
        make DESTDIR=${_PREFIX}/usr/share/harbour-patience-deck install
        popd
        echo "Built libffi"
    else
        echo "libffi already built"
    fi
} # libffi

build_guile() {
    if [ ! -e ${_LIBDIR}/libguile-2.2.so ] || [ ! -e ${GUILE}/ ]
    then
        echo "Building guile..."
        pushd ${GUILE}/
        patch -p1 << 'EOF'
--- guile-2.2.7/configure.ac.old	2021-05-15 17:29:32.876457455 +0300
+++ guile-2.2.7/configure.ac	2021-05-15 17:29:44.559502157 +0300
@@ -812,7 +812,7 @@
 #   cuserid - on Tru64 5.1b the declaration is documented to be available
 #       only with `_XOPEN_SOURCE' or some such.
 #
-AC_CHECK_HEADERS([crypt.h netdb.h pthread.h pthread_np.h sys/param.h sys/resource.h sys/file.h sys/mman.h])
+AC_CHECK_HEADERS([netdb.h pthread.h pthread_np.h sys/param.h sys/resource.h sys/file.h sys/mman.h])
 AC_CHECK_FUNCS(chroot flock getlogin cuserid getpriority setpriority getpass sethostname gethostname)
 AC_CHECK_DECLS([sethostname, hstrerror, cuserid])
 
@@ -829,9 +829,9 @@
 # AC_SEARCH_LIBS lets us add -lcrypt to LIBS only if crypt() is not in the
 # libraries already in that list.
 #
-AC_SEARCH_LIBS(crypt, crypt,
-  [AC_DEFINE([HAVE_CRYPT],1,
-             [Define to 1 if you have the `crypt' function.])])
+#AC_SEARCH_LIBS(crypt, crypt,
+#  [AC_DEFINE([HAVE_CRYPT],1,
+#             [Define to 1 if you have the `crypt' function.])])
 
 # When compiling with GCC on some OSs (Solaris, AIX), _Complex_I doesn't
 # work; in the reported cases so far, 1.0fi works well instead.  According
EOF
        LD_LIBRARY_PATH=/usr/lib/gconv:${_LIBDIR}
        PKG_CONFIG_PATH=${_LIBDIR}/pkgconfig
        export LD_LIBRARY_PATH
        export PKG_CONFIG_PATH
        autoreconf -i -f
        sed '/^\s*acl_libdirstem2=/s/=/=lib/' -i configure
        ./configure \
                --disable-static \
                --disable-networking \
                --disable-deprecated \
                --disable-error-on-warning \
                --with-libgmp-prefix=${CACHE}/built/usr/share/harbour-patience-deck/ \
                --with-libunistring-prefix=${CACHE}/built/usr/share/harbour-patience-deck/ \
                --with-libltdl-prefix=${CACHE}/built/usr/share/harbour-patience-deck/ \
                --with-modules=ice-9 \
                --prefix="/usr/share/harbour-patience-deck/"
        sed '/HAVE_CRYPT/s/1/0/' -i config.h
        echo '#define HAVE_GC_IS_HEAP_PTR 1' >> config.h
        echo '#define HAVE_GC_MOVE_DISAPPEARING_LINK 1' >> config.h
        make $@
        make DESTDIR=${_PREFIX}/ install
        export -n LD_LIBRARY_PATH
        export -n PKG_CONFIG_PATH
        popd
        echo "Built guile"
    else
        echo "Guile already built"
    fi
} # guile

set -e

mkdir -p ${CACHE}/
pushd ${CACHE}/
download_sources
verify
unpack
mkdir -p ${_LIBDIR}/ ${_INCLUDEDIR}/
build_bdwgc $@
build_libunistring $@
build_libgmp $@
build_libltdl $@
build_libffi $@
build_guile $@
popd
