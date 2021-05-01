Name:       patience-deck
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

%description
%{summary} for Sailfish. Based on GNOME Aisleriot.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
export NAME="%{name}"
export VERSION="$(git describe --tags)"
touch src/patience-deck.cpp

%qmake5
make %{?_smp_mflags}

g++ -o tools/convert tools/convert.cpp -fPIC \
    $(pkg-config --cflags --libs Qt5Core Qt5Gui Qt5Svg)

%install
rm -rf %{buildroot}

%qmake5_install

chmod -x %{buildroot}/%{_datadir}/%{name}/games/*.scm

desktop-file-install --delete-original \
    --dir %{buildroot}%{_datadir}/applications \
     %{buildroot}%{_datadir}/applications/*.desktop

tools/convert data/patience-deck.svg \
    %{buildroot}%{_datadir}/icons/hicolor/%1/apps/patience-deck.png \
    86x86 108x108 128x128 172x172

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
