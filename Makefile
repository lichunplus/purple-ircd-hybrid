PURPLE_IRCD_HYBRID := ${CURDIR}
NAME := purple-ircd-hybrid
VERSION := 0.0.0
RELEASE := 000

BUILD_DIR = ${PURPLE_IRCD_HYBRID}/build_dir
ROOTFS = ${PURPLE_IRCD_HYBRID}/rootfs
RPMBUILD_DIR = ${PURPLE_IRCD_HYBRID}/rpmbuild
RPM_PKGNAME = $(NAME)-$(VERSION).rpm

all:
	@mkdir -p ${BUILD_DIR}
	cd ${BUILD_DIR} && \
    ../ircd-hybrid-8.2.31/configure \
    --prefix=${ROOTFS}/usr \
    --libdir=${ROOTFS}/usr/lib64 \
    --sysconfdir=${ROOTFS}/etc \
    --enable-epoll \
    --with-tls='none' && \
    $(MAKE)

help:
	@echo "Usage: make [-jN] [rpm]"
	@echo "       make clean"

rpm: all
	cd ${BUILD_DIR} && $(MAKE) install
	install -d -m 0755 ${ROOTFS}/usr/lib/systemd/system
	cp ./config/ircd-hybrid.service ${ROOTFS}/usr/lib/systemd/system/
	cp ./config/ircd.conf ${ROOTFS}/etc/ircd.conf
	rpmbuild --define '_topdir     $(RPMBUILD_DIR)'     \
             --define 'rootfs      $(ROOTFS)'           \
             --define 'pkgname     $(NAME)'             \
             --define 'pkgversion  $(VERSION)'          \
             --define 'pkgrelease  $(RELEASE)'          \
             -bb config/rpm.spec
	cp -f $(RPMBUILD_DIR)/RPMS/x86_64/$(NAME)-$(VERSION)*.rpm $(RPM_PKGNAME)

clean:
	@rm -rf ${BUILD_DIR} ${ROOTFS} ${RPMBUILD_DIR}
	@rm -rf ${RPM_PKGNAME}

.PHONY: all help rpm clean
