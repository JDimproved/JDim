##########################################
# For using svn: do
# export SVNROOT="http://svn.sourceforge.jp/svnroot/jd4linux/jd"
# svn checkout $SVNROOT/trunk
# mv trunk jd-%%{main_ver}-%%{strtag}
# tar czf jd-%%{main_ver}-%%{strtag}.tgz jd-%%{main_ver}-%%{strtag}
##########################################

##########################################
# Defined by upsteam
#
%define         main_ver      2.8.9
%define         strtag        180424
%define         repoid        ?????
# Define this if this is pre-version
%define         pre_release   0
##########################################

##########################################
# Defined by vendor
#
%define         vendor_rel    1
# Tag name changed from vendor to vendorname so as not to
# overwrite Vendor entry in Summary
%define         vendorname    fedora
%define         gtkmmdevel    gtkmm24-devel
%define         fontpackage   fonts-japanese
%define         icondir       %{_datadir}/icons/hicolor/96x96/apps/
##########################################

##########################################
%if %{pre_release}
%define         rel           0.%{vendor_rel}.%{strtag}%{?dist}
%else
%define         rel           %{vendor_rel}%{?dist}
%endif

# By default, Migemo support is disabled.
%if             0%{?fedora} >= 5
%define         _with_migemo  1
%endif
%define         migemo_dict   %{_datadir}/cmigemo/utf-8/migemo-dict
##########################################

Name:           jdim
Version:        %{main_ver}
Release:        %{rel}
Summary:        A 2ch browser

Group:          Applications/Internet
License:        GPLv2
URL:            http://jd4linux.osdn.jp/
Source0:        http://downloads.sourceforge.jp/jd4linux/%{repoid}/%{name}-%{main_ver}-%{strtag}.tgz
#Source0:	%{name}-%{main_ver}-%{strtag}.tgz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  %{gtkmmdevel}
BuildRequires:  libtool automake
BuildRequires:  gnutls-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libSM-devel
%if 0%{?_with_migemo} >= 1
BuildRequires:  cmigemo-devel
%endif
Requires:       %{fontpackage}


%description
JDim is a 2ch browser based on gtkmm.

%prep
%setup -q -n %{name}-%{main_ver}-%{strtag}
#find . -name .svn | sort -r | xargs %{__rm} -rf

%build
sh autogen.sh

# set TZ for __TIME__
export TZ='Asia/Tokyo'

%configure \
%if 0%{?_with_migemo} >= 1
   --with-migemo \
   --with-migemodict=%{migemo_dict}
%endif

%{__make} %{?_smp_mflags}

%install
%{__rm} -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%{__mkdir_p} $RPM_BUILD_ROOT%{icondir}
%{__install} -p -m 644 %{name}.png $RPM_BUILD_ROOT%{icondir}

desktop-file-install \
   --vendor %{vendorname} \
   --dir $RPM_BUILD_ROOT%{_datadir}/applications \
   --delete-original \
   $RPM_BUILD_ROOT%{_datadir}/applications/%{name}.desktop

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
%{_datadir}/applications/%{vendorname}-%{name}.desktop
%{_datadir}/pixmaps/%{name}.png
%{icondir}/%{name}.png

%changelog
* Sun Mar  9 2006 Houritsuchu <houritsuchu@hotmail.com>
- Version up.
- add icon

* Sat Feb 25 2006 Houritsuchu <houritsuchu@hotmail.com>
- first
