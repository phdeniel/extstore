%define sourcename @CPACK_SOURCE_PACKAGE_FILE_NAME@
%global dev_version %{lua: extraver = string.gsub('@LIBKVSNS_EXTRA_VERSION@', '%-', '.'); print(extraver) }

Name: libkvsns 
Version: @LIBKVSNS_BASE_VERSION@
Release: 0%{dev_version}%{?dist}
Summary: Library to access to a namespace inside a KVS
License: LGPLv3 
Group: Development/Libraries
Url: http://github.com/phdeniel/libkvsns
Source: %{sourcename}.tar.gz
BuildRequires: cmake hiredis-devel
BuildRequires: gcc
Requires: redis hiredis

%description
The libkvsns is a library that allows of a POSIX namespace built on top of
a Key-Value Store.

%package devel
Summary: Development file for the library libkvsns
Group: Development/Libraries
Requires: %{name} = %{version}-%{release} pkgconfig

%description devel
The libkvsns is a library that allows of a POSIX namespace built on top of
a Key-Value Store.
This package contains the development headers for libkvsns.

%prep
%setup -q -n %{sourcename}

%build
cmake .
make %{?_smp_mflags} || make %{?_smp_mflags} || make

%install

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig
mkdir -p %{buildroot}%{_includedir}
install -m 644 libkvsns.so %{buildroot}%{_libdir}
install -m 644 kvsns.h  %{buildroot}%{_includedir}
install -m 644 libkvsns.pc  %{buildroot}%{_libdir}/pkgconfig


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_libdir}/libkvsns.so*

%files devel
%defattr(-,root,root)
%{_libdir}/pkgconfig/libkvsns.pc
%{_includedir}/kvsns.h

%changelog
* Mon Jul 25 2016 Philippe DENIEL <philippe.deniel@cea.fr> 0.9.i
- First alpha version