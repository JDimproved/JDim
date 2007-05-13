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
%define         main_ver      1.8.8
%define         strtag        svn1020_trunk
%define         repoid        24814


# Defined by vendor
#
%define         vendor_rel    2.%{strtag}
# Tag name changed from vendor to vendorname so as not to
# overwrite Vendor entry in Summary
%define         vendorname    fedora
%define         gtkmmdevel    gtkmm24-devel
%define         icondir       %{_datadir}/icons/hicolor/96x96/apps/

# Define this if this is pre-version
%define         pre_release   0

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
%doc help/
%{_bindir}/%{name}
%{_datadir}/applications/%{vendorname}-%{name}.desktop
%{icondir}/%{name}.png

%changelog
* Sat May 12 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-2.svn1020_trunk
- svn 1020

* Tue Apr  3 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-1
- 1.8.8

* Fri Mar 30 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-0.3.rc070330
- 1.8.8 rc 070330

* Fri Mar 23 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-0.3.beta070324
- 1.8.8 beta 070324

* Sat Mar 17 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-0.2.beta070317
- 1.8.8 beta 070317

* Sun Feb 18 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.8-0.1.beta070218
- 1.8.8 beta 070218

* Fri Feb  2 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.5-1
- 1.8.5

* Sun Jan 21 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.5-0.3.rc071121
- 1.8.5 rc 071121

* Sun Jan 14 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.5-0.3.beta071114
- 1.8.5 beta 070114

* Sun Jan  7 2007 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.5-0.2.beta061227
- Add fix for zero-inserted dat problem

* Tue Dec 26 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.5-0.1.beta061227
- 1.8.5 beta 061227

* Sun Dec 17 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.1-1
- 1.8.1

* Tue Dec 12 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.1-0.2.rc061213
- 1.8.1 rc 061213

* Sat Dec  2 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.1-0.2.beta061202
- 1.8.1 beta 061202

* Tue Nov 14 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-1
- 1.8.0

* Wed Nov  8 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.5.rc061108
- 1.8.0 rc 061108

* Fri Nov  3 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.5.beta061103
- 1.8.0 beta 061103

* Sat Oct 28 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.4.cvs061028
- Detect libSM and libICE for x86_64.
- cvs 061028 (23:59 JST)

* Wed Oct 25 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.3.beta061023
- Remove some category from desktop files due to
  desktop-file-utils change.

* Tue Oct 24 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.2.beta061023
- 1.8.0 beta 061023

* Sun Oct 22 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.1.cvs061022
- cvs 061022 (23:59 JST)

* Mon Oct  9 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.8.0-0.1.beta061009
- 1.8.0 beta 061009

* Sat Oct  7 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.7.0-2
- Add libSM-devel to BuildRequires.

* Wed Sep 27 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.7.0-1
- 1.7.0

* Mon Sep 25 2006 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 1.7.0-0.1.rc060921
- Import to Fedora Extras.

* Sun Mar  9 2006 Houritsuchu <houritsuchu@hotmail.com>
- Version up.
- add icon

* Sat Feb 25 2006 Houritsuchu <houritsuchu@hotmail.com>
- first
