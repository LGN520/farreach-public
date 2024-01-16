cd ..
cd common; make all; 

cd ..
cd benchmark/inswitchcache-java-lib; 
bash compile.sh; 
cd ../../
cd benchmark/ycsb; 
bash compile.sh; 
cd ../../

