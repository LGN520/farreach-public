mvn clean
mvn compile assembly:single
bash generate_jnifiles.sh com.inswitchcache.core.SocketHelper
cd jnisrc; make all; cd ..
