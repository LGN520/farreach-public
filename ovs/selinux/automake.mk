# Copyright (C) 2016 Nicira, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without warranty of any kind.

EXTRA_DIST += \
        selinux/openvswitch-custom.fc.in \
        selinux/openvswitch-custom.te.in

PHONY: selinux-policy

selinux-policy: selinux/openvswitch-custom.te selinux/openvswitch-custom.fc
	$(MAKE) -C selinux/ -f /usr/share/selinux/devel/Makefile

CLEANFILES += \
	selinux/openvswitch-custom.te \
	selinux/openvswitch-custom.pp \
	selinux/openvswitch-custom.fc \
	selinux/openvswitch-custom.if
