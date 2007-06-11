##########################################
# For using svn: do
# export SVNROOT="http://svn.sourceforge.jp/svnroot/jd4linux/jd"
# svn checkout $SVNROOT/trunk
# cd trunk
# svn checkout $SVNROOT/help
# chmod -R go+rX .
# cd ..
# mv trunk jd-%%{main_ver}-%%{strtag}
# tar czf jd-%%{main_ver}-%%{strtag}.tgz jd-%%{main_ver}-%%{strtag}
#
##########################################
# Defined by upsteam
#
%define         main_ver      1.9.5
%define         strtag        beta070611
%define         repoid        24814


# Defined by vendor
#
%define         vendor_rel    1
# Tag name changed from vendor to vendorname so as not to
# overwrite Vendor entry in Summary
%define         vendorname    fedora
%define         gtkmmdevel    gtkmm24-devel
%define         icondir       %{_datadir}/icons/hicolor/96x96/apps/

# Define this if this is pre-version
%define         pre_release   1

%if %{pre_release}
%define         rel           0.%{vendor_rel}.%{strtag}%{?dist}
%else
%define         rel           %{vendor_rel}%{?dist}
%endif


##########################################

Name:           jd
Version:        %{main_ver}
Release:        %{rel}
Summary:        A 2ch browser

Group:          Applications/Internet
License:        GPL
URL:            http://jd4linux.sourceforge.jp/
Source0:        http://osdn.dl.sourceforge.jp/jd4linux/%{repoid}/%{name}-%{main_ver}-%{strtag}.tgz
#Source0:	%{name}-%{main_ver}-%{strtag}.tgz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  %{gtkmmdevel}
BuildRequires:  libtool automake
BuildRequires:  openssl-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libSM-devel
Requires:       fonts-japanese


%description
JD is a 2ch browser based on gtkmm2.

%prep
%setup -q -n %{name}-%{main_ver}-%{strtag}
find . -name .svn | sort -r | xargs %{__rm} -rf

%build
sh autogen.sh

# set TZ for __TIME__
export TZ='Asia/Tokyo'

%configure
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%{__mkdir_p} $RPM_BUILD_ROOT%{_datadir}/applications
%{__mkdir_p} $RPM_BUILD_ROOT%{icondir}

%{__install} -p -m 644 %{name}.png $RPM_BUILD_ROOT%{icondir}

# desktop-file-tools 0.10->0.11 change
# 0.11 no longer accepts Application, X-Fedora, X-Red-Hat-Base
desktop-file-install \
   --vendor %{vendorname} \
   --dir $RPM_BUILD_ROOT%{_datadir}/applications \
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
%{_datadir}/applications/%{vendorname}-%{name}.desktop
%{icondir}/%{name}.png

%changelog
* Sun Mar  9 2006 Houritsuchu <houritsuchu@hotmail.com>
- Version up.
- add icon

* Sat Feb 25 2006 Houritsuchu <houritsuchu@hotmail.com>
- first
