mx switchos1 python ptf_popserver/table_configure.py 0 > ptf_popserver0.out &
mx switchos2 python ptf_popserver/table_configure.py 1 > ptf_popserver1.out &
mx switchos3 python ptf_popserver/table_configure.py 2 > ptf_popserver2.out &
mx switchos4 python ptf_popserver/table_configure.py 3 > ptf_popserver3.out &
# mx switchos1 python ptf_popserver/table_configure.py 0 &
# mx switchos2 python ptf_popserver/table_configure.py 1 &
# mx switchos3 python ptf_popserver/table_configure.py 2 &
