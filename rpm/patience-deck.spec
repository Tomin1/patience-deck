%bcond_with harbour
Name:       %{?with_harbour:harbour-}patience-deck
Summary:    Collection of patience games
Version:    0.8
Release:    1
# GNOME Aisleriot is GPLv3+ and this uses its assets
License:    GPLv3
URL:        https://github.com/Tomin1/patience-deck/
Source0:    %{name}-%{version}.tar.bz2
Patch0:     0001-Fix-errors-with-anglo.svg.patch
Patch1:     0002-Use-let-instead-of-letrec.patch
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils
BuildRequires:  git-core
BuildRequires:  python3-base
BuildRequires:  python3-lxml
BuildRequires:  librsvg-tools
BuildRequires:  gettext-devel

%if %{with harbour}
BuildRequires: automake autoconf libtool
%define __provides_exclude_from ^%{_datadir}/.*$
%define __requires_exclude ^libcrypt|libffi|libgc|libgmp|libguile|libltdl|libunistring.*$
%else
BuildRequires:  guile22-devel
%endif

%description
%{summary} for Sailfish. Based on GNOME Aisleriot.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
%if %{with harbour}
export CACHE=%{_builddir}/libs/%{_arch}
tools/build_deps.sh %{?_smp_mflags}
%endif

export NAME="%{name}"
export VERSION="$(git describe --tags)"
touch src/patience-deck.cpp

%qmake5
make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%if %{with harbour}
export CACHE=%{_builddir}/libs/%{_arch}
%endif
export NAME="%{name}"

%qmake5_install

desktop-file-install \
    --dir %{buildroot}%{_datadir}/applications \
%if %{with harbour}
    --set-key=Exec --set-value=harbour-patience-deck \
    --set-key=Icon --set-value=harbour-patience-deck \
%endif
     patience-deck.desktop

%if %{with harbour}
mkdir -p %{buildroot}%{_datadir}/%{name}/lib/
cp -P %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libgc.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libunistring.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libgmp.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libltdl.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libffi.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libguile-2.2.so* \
      %{buildroot}%{_datadir}/%{name}/lib/
rm %{buildroot}/usr/share/harbour-patience-deck/lib/libguile-2.2.so.*-gdb.scm
for dir in ice-9 language/scheme 'language/tree-il*' 'rnrs*' srfi system
do
    path=%{_datadir}/%{name}/share/guile/2.2/${dir}
    mkdir -p %{buildroot}${path%/*}
    cp -r %{_builddir}/libs/%{_arch}/built/${path} \
          %{buildroot}${path%/*}
    path=%{_datadir}/%{name}/lib/guile/2.2/ccache/${dir}
    mkdir -p %{buildroot}${path%/*}
    cp -r %{_builddir}/libs/%{_arch}/built${path} \
          %{buildroot}${path%/*}
done
mkdir -p %{buildroot}%{_datadir}/%{name}/lib/licenses
cp libs/%{_arch}/guile-*/COPYING \
   %{buildroot}%{_datadir}/%{name}/lib/licenses/COPYING.GPL3
cp libs/%{_arch}/guile-*/COPYING.LESSER \
   %{buildroot}%{_datadir}/%{name}/lib/licenses/COPYING.LESSER
cp libs/%{_arch}/gc-*/README.QUICK \
   %{buildroot}%{_datadir}/%{name}/lib/licenses/gc.README
cp libs/%{_arch}/libffi-*/LICENSE \
   %{buildroot}%{_datadir}/%{name}/lib/licenses/libffi.LICENSE
cp libs/%{_arch}/libtool-*/libltdl/COPYING.LIB \
   %{buildroot}%{_datadir}/%{name}/lib/licenses/COPYING.LIB

mv %{buildroot}%{_datadir}/applications/patience-deck.desktop \
   %{buildroot}%{_datadir}/applications/harbour-patience-deck.desktop

mv %{buildroot}%{_datadir}/%{name}/qml/patience-deck.qml \
   %{buildroot}%{_datadir}/%{name}/qml/harbour-patience-deck.qml
%endif

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
