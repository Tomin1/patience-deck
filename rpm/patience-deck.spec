Name:       patience-deck
Summary:    Collection of patience games
Version:    0.1.2
Release:    1
# GNOME Aisleriot is GPLv3+ and this uses its assets
License:    GPLv3+
URL:        https://github.com/Tomin1/patience-deck/
Source0:    %{name}-%{version}.tar.bz2
Patch0:     0001-Fix-errors-with-anglo.svg.patch
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils
BuildRequires:  guile22-devel

%description
%{summary} for Sailfish. Based on GNOME Aisleriot.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
%qmake5
make %{?_smp_mflags}

g++ -o icons/convert icons/convert.cpp -fPIC \
    $(pkg-config --cflags --libs Qt5Core Qt5Gui Qt5Svg)

%install
rm -rf %{buildroot}

%qmake5_install

chmod -x %{buildroot}/%{_datadir}/%{name}/games/*.scm

desktop-file-install --delete-original \
    --dir %{buildroot}%{_datadir}/applications \
     %{buildroot}%{_datadir}/applications/*.desktop

icons/convert icons/svg/patience-deck.svg \
    %{buildroot}%{_datadir}/icons/hicolor/%1/apps/patience-deck.png \
    86x86 108x108 128x128 172x172

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
