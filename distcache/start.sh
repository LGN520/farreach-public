mx h3 ./server 0 > tmp_server0.out &
mx h4 ./server 1 > tmp_server1.out &
mx h5 ./server 2 > tmp_server2.out &
mx h6 ./server 3 > tmp_server3.out &
mx h7 ./server 4 > tmp_server4.out &
mx h8 ./server 5 > tmp_server5.out &
mx h9 ./server 6 > tmp_server6.out &
mx h10 ./server 7 > tmp_server7.out &

mx switchos1 ./switchos 0 > tmp_switchos0.out &
mx switchos2 ./switchos 1 > tmp_switchos1.out &
mx switchos3 ./switchos 2 > tmp_switchos2.out &
mx switchos4 ./switchos 3 > tmp_switchos2.out &

mx spines1 ./switchos 0 spine > tmp_spineswitchos0.out &
mx spines2 ./switchos 1 spine > tmp_spineswitchos1.out &
mx spines3 ./switchos 2 spine > tmp_spineswitchos2.out &
mx spines4 ./switchos 3 spine > tmp_spineswitchos2.out &

mx h3 ./controller 0 > tmp_controller0.out &
mx h5 ./controller 1 > tmp_controller1.out &
mx h7 ./controller 2 > tmp_controller2.out &
mx h9 ./controller 3 > tmp_controller2.out &