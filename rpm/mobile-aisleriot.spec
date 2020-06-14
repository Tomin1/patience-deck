Name:       mobile-aisleriot
Summary:    Mobile port of GNOME Aisleriot
Version:    0.1
Release:    1
# GNOME Aisleriot is GPLv3+ and this uses its assets
License:    GPLv3+
URL:        TBD
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils
# TODO: And gc and libunistring probably too!
BuildRequires: guile22

%description
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

# TODO: And gc and libunistring probably too!
cp -L /usr/lib/libguile-2.2.so %{buildroot}/%{_datadir}/%{name}/

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
