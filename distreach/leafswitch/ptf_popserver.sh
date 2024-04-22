mx switchos1 python ptf_popserver/table_configure.py 0 >ptf_popserver0.out &
mx switchos2 python ptf_popserver/table_configure.py 1 >ptf_popserver1.out &
mx switchos3 python ptf_popserver/table_configure.py 2 >ptf_popserver2.out &
mx switchos4 python ptf_popserver/table_configure.py 3 >ptf_popserver3.out &

mx switchos5 python ptf_popserver/table_configure.py 4 >ptf_popserver4.out &
mx switchos6 python ptf_popserver/table_configure.py 5 >ptf_popserver5.out &
mx switchos7 python ptf_popserver/table_configure.py 6 >ptf_popserver6.out &
mx switchos8 python ptf_popserver/table_configure.py 7 >ptf_popserver7.out &