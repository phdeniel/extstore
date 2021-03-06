%define sourcename @CPACK_SOURCE_PACKAGE_FILE_NAME@
%global dev_version %{lua: extraver = string.gsub('@LIBEXTSTORE_EXTRA_VERSION@', '%-', '.'); print(extraver) }

Name: libextstore 
Version: @LIBEXTSTORE_BASE_VERSION@
Release: 0%{dev_version}%{?dist}
Summary: Library to access to a namespace inside a KVS
License: LGPLv3 
Group: Development/Libraries
Url: http://github.com/phdeniel/libextstore
Source: %{sourcename}.tar.gz
BuildRequires: cmake libini_config-devel
BuildRequires: gcc
Requires: libini_config
Provides: %{name} = %{version}-%{release}

# Conditionally enable KVS and object stores
#
# 1. rpmbuild accepts these options (gpfs as example):
#    --without redis

%define on_off_switch() %%{?with_%1:ON}%%{!?with_%1:OFF}

# A few explanation about %bcond_with and %bcond_without
# /!\ be careful: this syntax can be quite messy
# %bcond_with means you add a "--with" option, default = without this feature
# %bcond_without adds a"--without" so the feature is enabled by default

@BCOND_RADOS@ rados
%global use_rados %{on_off_switch rados}

@BCOND_MOTR@ motr
%global use_motr %{on_off_switch motr}

%description
The libextstore is a library that allows of a POSIX namespace built on top of
a Key-Value Store.

%package devel
Summary: Development file for the library libextstore
Group: Development/Libraries
Requires: %{name} = %{version}-%{release} pkgconfig
Provides: %{name}-devel = %{version}-%{release}

%package crud_cache
Summary: The CRUD cache based backend for libextstore
Group: Applications/System
Requires: %{name} = %{version}-%{release} librados2
Provides: %{name}-extstore-crud_cache = %{version}-%{release}

%description crud_cache
This package contains a library for using POSIX as a backed for libextstore

%package posix
Summary: The (dummy) POSIX  based backend for libextstore
Group: Applications/System
Requires: %{name} = %{version}-%{release} librados2
Provides: %{name}-extstore-posix = %{version}-%{release}

%description posix
This package contains a library for using POSIX as a backed for libextstore

# RADOS
%if %{with rados}
%package rados
Summary: The RADOS based backend for libextstore
Group: Applications/System
Requires: %{name} = %{version}-%{release} librados2
Provides: %{name}-extstore-rados = %{version}-%{release}

%description rados
This package contains a library for using RADOS as a backed for libextstore
%endif

# MOTR
%if %{with motr}
%package motr
Summary: The MOTR based backend for libextstore
Group: Applications/System
Requires: %{name} = %{version}-%{release} cortx-motr
Requires: libiosea-hash = %{version}-%{release}
Requires: libm0common = %{version}-%{release}
BuildRequires: libiosea-hash-devel = %{version}-%{release}
Provides: %{name}-motr = %{version}-%{release}

%description motr
This package contains libraries for using CORTX-MOTR as a backend for libextstore
%endif


%description devel
The libextstore is a library that allows of a POSIX namespace built on top of
a Key-Value Store.
This package contains tools for libextstore.

%prep
%setup -q -n %{sourcename}

%build
cmake . 

make %{?_smp_mflags} || make %{?_smp_mflags} || make

%install

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig
mkdir -p %{buildroot}%{_includedir}/iosea
mkdir -p %{buildroot}%{_sysconfdir}/kvsns.d
install -m 644 include/iosea/extstore.h  %{buildroot}%{_includedir}/iosea

install -m 644 extstore/posix_store/libextstore_posix.so %{buildroot}%{_libdir}
install -m 644 extstore/crud_cache/libextstore_crud_cache.so %{buildroot}%{_libdir}
install -m 644 extstore/crud_cache/libobjstore_cmd.so %{buildroot}%{_libdir}
%if %{with rados gnutls}
install -m 644 extstore/rados/libextstore_rados.so %{buildroot}%{_libdir}
%endif
%if %{with motr}
install -m 644 extstore/motr/libextstore_motr.so %{buildroot}%{_libdir}
%endif

install -m 644 libextstore.pc  %{buildroot}%{_libdir}/pkgconfig
install -m 644 kvsns.ini %{buildroot}%{_sysconfdir}/kvsns.d

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/kvsns.d/kvsns.ini

%files devel
%defattr(-,root,root)
%{_libdir}/pkgconfig/libextstore.pc
%{_includedir}/iosea/extstore.h

%files posix
%{_libdir}/libextstore_posix.so*

%files crud_cache
%{_libdir}/libextstore_crud_cache.so*
%{_libdir}/libobjstore_cmd.so*

%if %{with motr}
%files motr
%{_libdir}/libextstore_motr.so*
%endif

%if %{with rados}
%files rados
%{_libdir}/libextstore_rados.so*
%endif


%changelog
* Wed Nov  3 2021 Philippe DENIEL <philippe.deniel@cea.fr> 1.3.0
- Better layering between kvsns, kvsal aand extstore. 
