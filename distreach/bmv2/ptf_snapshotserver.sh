
mx switchos1 python ptf_snapshotserver/table_configure.py 0 >ptf_snapshotserver0.out &
mx switchos2 python ptf_snapshotserver/table_configure.py 1 >ptf_snapshotserver1.out &
mx switchos3 python ptf_snapshotserver/table_configure.py 2 >ptf_snapshotserver2.out &

# mx switchos1 python ptf_popserver/table_configure.py 0 &
# mx switchos2 python ptf_popserver/table_configure.py 1 &
# mx switchos3 python ptf_popserver/table_configure.py 2 &
