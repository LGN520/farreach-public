#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# update network settings in all involved config files automatically

#set -x

source scripts/global.sh
if [ $# -ne 0 ]; then
	echo "Usage: bash scripts/local/update_network_settings.sh"
	exit
fi

function getlinenum() {
	tmpfile=$1
	result=$(awk -F '=' -v tmpsection=[$2] -v tmpkey=$3 '$0==tmpsection {flag=1; next} /^\[/ {flag = 0; next} flag && $1==tmpkey {print NR}' ${tmpfile})
	echo ${result}
}

configfile_list=("farreach/config.ini" "farreach/configs/config.ini.static.setup" "nocache/config.ini" "nocache/configs/config.ini.static.setup" "netcache/config.ini" "netcache/configs/config.ini.static.setup")

for tmp_configfile in ${configfile_list[@]}; do
	# Update network settings for main client
	tmp_secname="client0"
	tmp_keyname="client_ip"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${MAIN_CLIENT_TOSWITCH_IP}/g" ${tmp_configfile}
	tmp_keyname="client_mac"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${MAIN_CLIENT_TOSWITCH_MAC}/g" ${tmp_configfile}
	tmp_keyname="client_fpport"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${MAIN_CLIENT_TOSWITCH_FPPORT}/g" ${tmp_configfile}
	tmp_keyname="client_pipeidx"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${MAIN_CLIENT_TOSWITCH_PIPEIDX}/g" ${tmp_configfile}
	tmp_keyname="client_ip_for_client0"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${MAIN_CLIENT_LOCAL_IP}/g" ${tmp_configfile}

	# Update network settings for secondary client
	tmp_secname="client1"
	tmp_keyname="client_ip"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SECONDARY_CLIENT_TOSWITCH_IP}/g" ${tmp_configfile}
	tmp_keyname="client_mac"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SECONDARY_CLIENT_TOSWITCH_MAC}/g" ${tmp_configfile}
	tmp_keyname="client_fpport"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SECONDARY_CLIENT_TOSWITCH_FPPORT}/g" ${tmp_configfile}
	tmp_keyname="client_pipeidx"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SECONDARY_CLIENT_TOSWITCH_PIPEIDX}/g" ${tmp_configfile}
	tmp_keyname="client_ip_for_client0"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SECONDARY_CLIENT_LOCAL_IP}/g" ${tmp_configfile}

	# Update network settings for first server
	tmp_secname="server0"
	tmp_keyname="server_ip"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_TOSWITCH_IP}/g" ${tmp_configfile}
	tmp_keyname="server_mac"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_TOSWITCH_MAC}/g" ${tmp_configfile}
	tmp_keyname="server_fpport"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_TOSWITCH_FPPORT}/g" ${tmp_configfile}
	tmp_keyname="server_pipeidx"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_TOSWITCH_PIPEIDX}/g" ${tmp_configfile}
	tmp_keyname="server_ip_for_controller"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_LOCAL_IP}/g" ${tmp_configfile}

	# Update network settings for second server
	tmp_secname="server1"
	tmp_keyname="server_ip"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_TOSWITCH_IP}/g" ${tmp_configfile}
	tmp_keyname="server_mac"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_TOSWITCH_MAC}/g" ${tmp_configfile}
	tmp_keyname="server_fpport"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_TOSWITCH_FPPORT}/g" ${tmp_configfile}
	tmp_keyname="server_pipeidx"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_TOSWITCH_PIPEIDX}/g" ${tmp_configfile}
	tmp_keyname="server_ip_for_controller"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_LOCAL_IP}/g" ${tmp_configfile}

	# Update network settings for controller
	tmp_secname="controller"
	tmp_keyname="controller_ip_for_server"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${CONTROLLER_LOCAL_IP}/g" ${tmp_configfile}
	tmp_keyname="controller_ip_for_switchos"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${CONTROLLER_LOCAL_IP}/g" ${tmp_configfile}

	# Update network settings for switch
	tmp_secname="switch"
	tmp_keyname="switchos_ip"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SWITCHOS_LOCAL_IP}/g" ${tmp_configfile}
	tmp_keyname="switch_recirport_pipeline1to0"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SWITCH_RECIRPORT_PIPELINE1TO0}/g" ${tmp_configfile}
	tmp_keyname="switch_recirport_pipeline0to1"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SWITCH_RECIRPORT_PIPELINE0TO1}/g" ${tmp_configfile}

	# Update CPU settings for first server
	tmp_secname="server0"
	tmp_keyname="server_worker_corenum"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_WORKER_CORENUM}/g" ${tmp_configfile}
	tmp_keyname="server_total_corenum"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER0_TOTAL_CORENUM}/g" ${tmp_configfile}

	# Update CPU settings for second server
	tmp_secname="server1"
	tmp_keyname="server_worker_corenum"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_WORKER_CORENUM}/g" ${tmp_configfile}
	tmp_keyname="server_total_corenum"
	tmp_linenum=$(getlinenum ${tmp_configfile} ${tmp_secname} ${tmp_keyname})
	sed -i "${tmp_linenum}s/^${tmp_keyname}=.*/${tmp_keyname}=${SERVER1_TOTAL_CORENUM}/g" ${tmp_configfile}
done
