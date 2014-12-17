%define major_version 8.10.0
%define tar_name php5-midgard

%global php_zendabiver %((echo 0; php -i 2>/dev/null | sed -n 's/^PHP Extension => //p') | tail -1)
%define php_extdir %(php-config --extension-dir 2>/dev/null)

%if 0%{?suse_version}
%define php_confdir php5/conf.d
%define rpm_name php5-midgard
%else
%define php_confdir php.d
%define rpm_name php-midgard
%endif

Name:           %{rpm_name}
Version:        %{major_version}
Release:        OBS
Summary:        PHP extension for Midgard

Group:          Development/Languages
License:        LGPLv2+
URL:            http://www.midgard-project.org/
Source0:        %{url}download/%{tar_name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  php-devel >= 5.1
BuildRequires:  midgard-core-devel >= %{major_version}

%if 0%{?suse_version}
Provides:       php-midgard = %{version}-%{release}
%else
%if %{?php_zend_api}0
Requires:       php(zend-abi) = %{php_zend_api}
Requires:       php(api) = %{php_core_api}
%else
Requires:       php-zend-abi = %{php_zendabiver}
%endif
Obsoletes:      php5-midgard < 8.09.6
Provides:       php5-midgard = %{version}-%{release}
%endif
Obsoletes:      midgard-php4 < 1.9

%description
The %{name} package contains a dynamic shared object that will add
Midgard support to PHP. Midgard is a persistent storage framework which 
is used e.g. by the Midgard Content Management System. PHP is an 
HTML-embeddable scripting language. If you need Midgard support for PHP 
applications, you will need to install this package in addition to the 
php package.


%prep
%setup -q -n %{tar_name}-%{version}


%build
export PHP_RPATH=no
phpize
%configure
make %{?_smp_mflags}


%install
%if 0%{?suse_version} == 0
rm -rf $RPM_BUILD_ROOT
mkdir -p $(dirname $RPM_BUILD_ROOT)
mkdir $RPM_BUILD_ROOT
%endif
install -D -m 755 .libs/midgard.so $RPM_BUILD_ROOT%{php_extdir}/midgard.so
install -D -m 644 midgard.ini $RPM_BUILD_ROOT%{_sysconfdir}/%{php_confdir}/midgard.ini


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc COPYING
%{php_extdir}/midgard.so
%config(noreplace) %{_sysconfdir}/%{php_confdir}/midgard.ini


%changelog
* Thu Jul 16 2009 Jarkko Ala-Louvesniemi <jval@puv.fi> 8.09.5
- Initial OBS package based on the Fedora spec.
