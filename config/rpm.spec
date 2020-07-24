Name:		%{pkgname}
Version:	%{pkgversion}
Release:	%{pkgrelease}
Summary:	A part of Purple Project
License:	LGPLv2+

Requires: python

%description
A part of Purple Project

%prep
#Normally this involves unpacking the sources and applying any patches.

%build
#This generally involves the equivalent of a "make".

%install
#This generally involves the equivalent of a "make install".
cp -rf %{rootfs}/* $RPM_BUILD_ROOT/

%check

%pre
getent group purple >/dev/null || groupadd -f -r purple
if ! getent passwd ircd-hybrid >/dev/null; then
  useradd -r -G purple -d / -s /sbin/nologin -c "ircd-hybrid user" ircd-hybrid
fi
exit 0

%post
%systemd_post ircd-hybrid.service
/bin/systemctl enable ircd-hybrid.service >/dev/null 2>&1
/bin/systemctl start ircd-hybrid.service >/dev/null 2>&1

%preun
%systemd_preun ircd-hybrid.service

#postun
/bin/systemctl daemon-reload >/dev/null 2>&1 || :
if [ $1 -ge 1 ] ; then
    /bin/systemctl try-restart ircd-hybrid.service >/dev/null 2>&1 || :
fi

%clean

%files
%defattr(-,root,root,-)
%dir %attr(0755, root, root) %{_libdir}/ircd-hybrid/
%{_libdir}/ircd-hybrid/*
%{_bindir}/*
%{_sysconfdir}/*
%{_mandir}/man8/*
%dir %{_datadir}/ircd-hybrid/
%{_datadir}/ircd-hybrid/*
%{_unitdir}/ircd-hybrid.service
