%bcond_with harbour
Name:       %{?with_harbour:harbour-}patience-deck
Summary:    Collection of patience games
Version:    0.3.0
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
BuildRequires:  guile22-devel
BuildRequires:  git-core
%if %{with harbour}
%define __provides_exclude_from ^%{_datadir}/.*$
%define __requires_exclude ^libcrypt|libffi|libgc|libgmp|libguile|libltdl|libunistring.*$
%endif

%description
%{summary} for Sailfish. Based on GNOME Aisleriot.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
%if %{with harbour}
export CACHE=%{_builddir}/libs/%{_arch}
tools/build_deps.sh
%endif

export NAME="%{name}"
export VERSION="$(git describe --tags)"
touch src/patience-deck.cpp

%qmake5
make %{?_smp_mflags}

g++ -o tools/convert tools/convert.cpp -fPIC \
    $(pkg-config --cflags --libs Qt5Core Qt5Gui Qt5Svg)

%install
rm -rf %{buildroot}

export NAME="%{name}"

%qmake5_install

chmod -x %{buildroot}/%{_datadir}/%{name}/games/*.scm

desktop-file-install \
    --dir %{buildroot}%{_datadir}/applications \
%if %{with harbour}
    --set-key=Exec --set-value=harbour-patience-deck \
    --set-key=Icon --set-value=harbour-patience-deck \
%endif
     patience-deck.desktop

%if %{with harbour}
mv %{buildroot}%{_datadir}/applications/patience-deck.desktop \
   %{buildroot}%{_datadir}/applications/harbour-patience-deck.desktop
%endif

tools/convert data/patience-deck.svg \
    %{buildroot}%{_datadir}/icons/hicolor/%1/apps/%{name}.png \
    86x86 108x108 128x128 172x172

%if %{with harbour}
mkdir -p %{buildroot}%{_datadir}/%{name}/lib/
cp -P %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libgc.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libunistring.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libgmp.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libltdl.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libffi.so* \
      %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/libguile-2.2.so* \
      %{buildroot}%{_datadir}/%{name}/lib/
rm /usr/share/harbour-patience-deck/lib/libguile-2.2.so.1.4.2-gdb.scm
mkdir -p %{buildroot}%{_datadir}/%{name}/share/guile/
cp -r %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/share/guile/2.2/ \
      %{buildroot}%{_datadir}/%{name}/share/guile/
mkdir -p %{buildroot}%{_datadir}/%{name}/lib/guile/2.2/
cp -r %{_builddir}/libs/%{_arch}/built%{_datadir}/%{name}/lib/guile/2.2/ccache \
      %{buildroot}%{_datadir}/%{name}/lib/guile/2.2/
#find %{buildroot}%{_datadir}/%{name}/lib/guile/ -name '*.go' -exec strip '{}' \+
%endif

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
