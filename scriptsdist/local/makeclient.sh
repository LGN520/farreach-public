cd ..
cd common; make all; 

cd ..
cd benchmarkdist/inswitchcache-java-lib; 
bash compile.sh; 
cd ../../
cd benchmarkdist/ycsb; 
bash compile.sh; 
cd ../../

