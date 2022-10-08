filename=$1

scp $filename ssy@bf1:~/NetBuffer/nocache/$filename
scp $filename ssy@dl13:~/projects/NetBuffer/nocache/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/nocache/$filename
scp $filename ssy@dl16:~/projects/NetBuffer/nocache/$filename
