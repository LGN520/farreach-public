mx switchos1 python ptf_cachefrequencyserver/table_configure.py 0 > ptf_cachefrequencyserver0.out &
mx switchos2 python ptf_cachefrequencyserver/table_configure.py 1 > ptf_cachefrequencyserver1.out &
mx switchos3 python ptf_cachefrequencyserver/table_configure.py 2 > ptf_cachefrequencyserver2.out &
mx switchos4 python ptf_cachefrequencyserver/table_configure.py 3 > ptf_cachefrequencyserver3.out &
# mx switchos1 python ptf_popserver/table_configure.py 0 &
# mx switchos2 python ptf_popserver/table_configure.py 1 &
# mx switchos3 python ptf_popserver/table_configure.py 2 &
