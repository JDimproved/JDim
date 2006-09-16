# Defined by upsteam
#
%define         main_ver      1.70
%define         minor_ver     060914.b
# Define this if this is pre-version
%define         pre_release   1

%define         icondir       %{_datadir}/icons/hicolor/96x96/apps/

# Defined by vendor
%{!?vendor:     %define vendor      fedora}
%{!?vender_rel: %define vendor_rel  1}
%{!?category:   %define category    X-Fedora}

%if %{pre_release}
%define         rel           0.%{vendor_rel}.%{minor_ver}
%else
%define         rel           %{vendor_rel}
%endif

Name:           jd
Version:        %{main_ver}
Release:        %{rel}%{?dist}
Summary:        A 2ch browser

Group:          Applications/Internet
License:        GPL
URL:            http://jd4linux.sourceforge.jp/
Source0:        http://osdn.dl.sourceforge.jp/jd4linux/21736/%{name}-%{main_ver}-%{minor_ver}.tgz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  gtkmm24-devel
BuildRequires:  libtool automake
BuildRequires:  openssl-devel
BuildRequires:  desktop-file-utils
Requires:       fonts-japanese

%description
JD is a 2ch browser based on gtkmm2.

%prep
%setup -q -n %{name}-%{main_ver}-%{minor_ver}
find . -name CVS | sort -r | xargs %{__rm} -rf

%build
sh autogen.sh

%configure
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%{__mkdir_p} $RPM_BUILD_ROOT%{_datadir}/applications
%{__mkdir_p} $RPM_BUILD_ROOT%{icondir}

%{__install} -p -m 644 %{name}.png $RPM_BUILD_ROOT%{icondir}

# leave X-Red-Hat-Base included.
desktop-file-install \
   --vendor %{vendor} \
   --dir $RPM_BUILD_ROOT%{_datadir}/applications \
   --add-category %{category} \
   %{name}.desktop

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%post
touch --no-create %{_datadir}/icons/hicolor || :
%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :

%postun
touch --no-create %{_datadir}/icons/hicolor || :
%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :

%files
%defattr(-,root,root,-)
%doc COPYING ChangeLog README
%{_bindir}/%{name}
%{_datadir}/applications/%{vendor}-%{name}.desktop
%{icondir}/%{name}.png

%changelog
* Sun Mar  9 2006 Houritsuchu <houritsuchu@hotmail.com>
- Version up.
- add icon

* Sat Feb 25 2006 Houritsuchu <houritsuchu@hotmail.com>
- first
