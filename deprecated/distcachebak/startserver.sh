mx h3 ./server 0 > tmp_server0.out &
mx h4 ./server 1 > tmp_server1.out &

mx switchos1 ./switchos spine > tmp_switchos0.out &
mx switchos2 ./switchos leaf > tmp_switchos1.out &

mx h3 ./controller 0 > tmp_controller0.out &

mx h3 ./reflector leaf > tmp_reflector_leaf.out &
mx h1 ./reflector spine > tmp_reflector_spine.out &