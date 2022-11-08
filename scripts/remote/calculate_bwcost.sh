if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash scripts/remote/calculate_bwcost.sh"
	exit
fi

methodname=${DIRNAME}
if [ "x${methodname}" != "xfarreach" ] || [ "x${snapshot_period}" == "x0" ]
then
	echo "[ERROR] you can only use this script for FarReach with >0 snapshot period!"
	exit
fi

filedir=${SERVER_ROOTPATH}/${methodname}
filename="tmp_controller_bwcost.out"
filepath=${filedir}/${filename}

echo "copy statistics file ${filename} from main server"
scp ${USER}@${SERVER0}:${filepath} ${CLIENT_ROOTPATH}

cd scripts/local/
#python calculate_bwcost_helper.py ${CLIENT_ROOTPATH}/${filename} ${snapshot_period}
python calculate_bwcost_helper.py ${CLIENT_ROOTPATH}/${filename}
cd ../../
