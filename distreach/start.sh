# mx h3 ./server 0 > tmp_server0.out &
# mx h4 ./server 1 > tmp_server1.out &
# mx h5 ./server 2 > tmp_server2.out &
# mx h6 ./server 3 > tmp_server3.out &
# mx h7 ./server 4 > tmp_server4.out &
# mx h8 ./server 5 > tmp_server5.out &


# mx switchos1 ./switchos 0 > tmp_switchos0.out &
# mx switchos2 ./switchos 1 > tmp_switchos1.out &
# mx switchos3 ./switchos 2 > tmp_switchos2.out &

# mx h3 ./controller 0 > tmp_controller0.out &
# mx h5 ./controller 1 > tmp_controller1.out &
# mx h7 ./controller 2 > tmp_controller2.out &

./startserver.sh 
./localscriptsbmv2/launchswitchostestbed.sh