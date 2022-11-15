COMMON_EXCLUSIONS="! -path *reserved_files* ! -path *results* ! -path *tofino-backup* ! -path *visulization* ! -iname *.o ! -iname *.d ! -iname *.bak ! -iname *.txt"

# common template
#find ./${tmpdirname} -type f \( ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" \)

#set -x

function getLOC() {
	tmpdirname=$1

	echo "(1) Software LOC:"
	cfiles=($(find ./${tmpdirname} -type f \( ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" ! -iname "*.p4" ! -iname "*.py" \)))
	cfiles+=($(find ./common -type f \( ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" ! -iname "*.p4" ! -iname "*.py" \)))
	cfiles+=($(find ./benchmark/inswitchcache-java-lib -type f \( ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" ! -iname "*.p4" ! -iname "*.py" \)))
	cfiles+=($(find ./benchmark/ycsb/${tmpdirname} -type f \( ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" ! -iname "*.p4" ! -iname "*.py" \)))
	echo "${cfiles[*]}" | xargs cloc
	echo ""

	echo "(2) Ptf LOC:"
	pyfiles=($(find ./${tmpdirname} -type f \( -iname "*.py" ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" \)))
	echo "${pyfiles[*]}" | xargs cloc
	p4files=($(find ./${tmpdirname} -type f \( -iname "*.p4" ${COMMON_EXCLUSIONS} ! -path "*deprecate*" ! -iname "*.out" \)))
	p4loc=0
	for ((i=0;i<${#p4files[*]};i++)); do
		tmploc=$(wc -l ${p4files[$i]} | awk {'print $1'})
		p4loc=$(expr ${p4loc} + ${tmploc})
	done
	echo ""

	echo "(3) P4 LOC: ${p4loc}"
}

getLOC farreach
