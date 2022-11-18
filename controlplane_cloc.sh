# NOTE: we only count the core LOC related with our design, yet not count the common LOC shared with other components

#set -x

function getControlplaneLOC() {
	tmpdirname=$1

	echo "(1) Switch-OS LOC:"
	cloc ${tmpdirname}/switchos.c common/special_case.* ${tmpdirname}/outband_packet_format.* ${tmpdirname}/message_queue_impl.h ${tmpdirname}/concurrent_map_impl.h
	echo ""

	echo "(2) Controller LOC:"
	cloc ${tmpdirname}/controller.c ${tmpdirname}/outband_packet_format.*
	echo ""
}

echo "Control-plane LOC of farreach"
getControlplaneLOC farreach
