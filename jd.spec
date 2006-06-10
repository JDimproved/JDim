Summary: jd 2ch browser
Name: jd
Version: 151b_060611
Release: 1
Group: Applications/Internet
URL: http://www.geocities.jp/jd4linux/
Source0: %{name}-%{version}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: gtkmm24 glibmm24 pango openssl zlib
BuildRequires: libtool automake autoconf gtkmm24-devel glibmm24-devel pango-devel openssl-devel zlib-devel
License: GPL

%description 
jd is the browser for 2ch.It's similar to OpenJaneDoe.

%prep
%setup -q

%build
sh autogen.sh
%configure
make

%install
%makeinstall
%{__install} -D -m 644 %{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
%{__install} -D -m 644 %{name}.png %{buildroot}%{_datadir}/pixmaps/%{name}.png

rm -rf $RPM_BUILD_ROOT/usr/lib
rm -rf $RPM_BUILD_ROOT/usr/src

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{name}-%{version}/

%post

%files
%defattr(-,root,root)
%doc README COPYING ChangeLog
%{_bindir}/jd
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png

%changelog
* Sun Mar 9 2006 Houritsuchu <houritsuchu@hotmail.com>
- Version up.
- add icon

* Sat Feb 25 2006 Houritsuchu <houritsuchu@hotmail.com>
- first

