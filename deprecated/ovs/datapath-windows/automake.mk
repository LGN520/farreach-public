EXTRA_DIST += \
	datapath-windows/Package/package.VcxProj \
	datapath-windows/Package/package.VcxProj.user \
	datapath-windows/include/OvsDpInterfaceExt.h \
	datapath-windows/include/OvsDpInterfaceCtExt.h \
	datapath-windows/misc/DriverRecommendedRules.ruleset \
	datapath-windows/misc/OVS.psm1 \
	datapath-windows/misc/install.cmd \
	datapath-windows/misc/uninstall.cmd \
	datapath-windows/ovsext.sln \
	datapath-windows/ovsext/Actions.c \
	datapath-windows/ovsext/Actions.h \
	datapath-windows/ovsext/Atomic.h \
	datapath-windows/ovsext/BufferMgmt.c \
	datapath-windows/ovsext/BufferMgmt.h \
	datapath-windows/ovsext/Conntrack-ftp.c \
	datapath-windows/ovsext/Conntrack-icmp.c \
	datapath-windows/ovsext/Conntrack-other.c \
	datapath-windows/ovsext/Conntrack-related.c \
	datapath-windows/ovsext/Conntrack-nat.c \
	datapath-windows/ovsext/Conntrack-tcp.c \
	datapath-windows/ovsext/Conntrack-nat.h \
	datapath-windows/ovsext/Conntrack.c \
	datapath-windows/ovsext/Conntrack.h \
	datapath-windows/ovsext/Datapath.c \
	datapath-windows/ovsext/Datapath.h \
	datapath-windows/ovsext/Debug.c \
	datapath-windows/ovsext/Debug.h \
	datapath-windows/ovsext/DpInternal.h\
	datapath-windows/ovsext/Driver.c \
	datapath-windows/ovsext/Ethernet.h \
	datapath-windows/ovsext/Event.c \
	datapath-windows/ovsext/Event.h \
	datapath-windows/ovsext/Flow.c \
	datapath-windows/ovsext/Flow.h \
	datapath-windows/ovsext/Gre.h \
	datapath-windows/ovsext/Gre.c \
	datapath-windows/ovsext/IpFragment.c \
	datapath-windows/ovsext/IpFragment.h \
	datapath-windows/ovsext/IpHelper.c \
	datapath-windows/ovsext/IpHelper.h \
	datapath-windows/ovsext/Jhash.c \
	datapath-windows/ovsext/Jhash.h \
	datapath-windows/ovsext/Mpls.h \
	datapath-windows/ovsext/NetProto.h \
	datapath-windows/ovsext/Netlink/Netlink.c \
	datapath-windows/ovsext/Netlink/Netlink.h \
	datapath-windows/ovsext/Netlink/NetlinkBuf.c \
	datapath-windows/ovsext/Netlink/NetlinkBuf.h \
	datapath-windows/ovsext/Netlink/NetlinkError.h \
	datapath-windows/ovsext/Netlink/NetlinkProto.h \
	datapath-windows/ovsext/Offload.c \
	datapath-windows/ovsext/Offload.h \
	datapath-windows/ovsext/Oid.c \
	datapath-windows/ovsext/Oid.h \
	datapath-windows/ovsext/PacketIO.c \
	datapath-windows/ovsext/PacketIO.h \
	datapath-windows/ovsext/PacketParser.c \
	datapath-windows/ovsext/PacketParser.h \
	datapath-windows/ovsext/Recirc.c \
	datapath-windows/ovsext/Recirc.h \
	datapath-windows/ovsext/Stt.c \
	datapath-windows/ovsext/Stt.h \
	datapath-windows/ovsext/Switch.c \
	datapath-windows/ovsext/Switch.h \
	datapath-windows/ovsext/Tunnel.c \
	datapath-windows/ovsext/Tunnel.h \
	datapath-windows/ovsext/TunnelFilter.c \
	datapath-windows/ovsext/TunnelIntf.h \
	datapath-windows/ovsext/Types.h \
	datapath-windows/ovsext/User.c \
	datapath-windows/ovsext/User.h \
	datapath-windows/ovsext/Util.c  \
	datapath-windows/ovsext/Util.h \
	datapath-windows/ovsext/Vport.c \
	datapath-windows/ovsext/Vport.h \
	datapath-windows/ovsext/Vxlan.c \
	datapath-windows/ovsext/Vxlan.h \
	datapath-windows/ovsext/Geneve.c \
	datapath-windows/ovsext/Geneve.h \
	datapath-windows/ovsext/ovsext.inf \
	datapath-windows/ovsext/ovsext.rc \
	datapath-windows/ovsext/ovsext.vcxproj \
	datapath-windows/ovsext/ovsext.vcxproj.user \
	datapath-windows/ovsext/precomp.h \
	datapath-windows/ovsext/precompsrc.c \
	datapath-windows/ovsext/resource.h

datapath_windows_analyze: all
	MSBuild.exe //nologo //maxcpucount datapath-windows/ovsext.sln /target:Build /property:Configuration="Win10Analyze"
	MSBuild.exe //nologo //maxcpucount datapath-windows/ovsext.sln /target:Build /property:Configuration="Win8.1Analyze"
	MSBuild.exe //nologo //maxcpucount datapath-windows/ovsext.sln /target:Build /property:Configuration="Win8Analyze"

datapath_windows: all
	MSBuild.exe //nologo //maxcpucount datapath-windows/ovsext.sln /target:Build /property:Configuration="Win10Debug"
	MSBuild.exe //nologo //maxcpucount datapath-windows/ovsext.sln /target:Build /property:Configuration="Win10Release"
