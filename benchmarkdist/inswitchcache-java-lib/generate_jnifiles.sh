if [ $# -ne 1 ]
then
	#echo "Usage: bash generate_jnifiles.sh modulename classpath"
	#echo "Example: bash generate_jnifiles.sh core site.ycsb.SocketHelper"
	echo "Usage: bash generate_jnifiles.sh classpath"
	echo "Example: bash generate_jnifiles.sh com.inswitchcache.core.SocketHelper"
	exit
fi

#modulename=$1
#classpath=$2
classpath=$1

#javah -d ./jnisrc -jni -classpath core/target/classes/:${modulename}/target/classes/ ${classpath}
javah -d ./jnisrc -jni -classpath target/classes/ ${classpath}
