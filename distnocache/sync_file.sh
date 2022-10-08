filename=$1

scp $filename ssy@bf1:~/NetBuffer/distnocache/$filename
scp $filename ssy@bf3:~/NetBuffer/distnocache/$filename
scp $filename ssy@dl13:~/projects/NetBuffer/distnocache/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/distnocache/$filename
scp $filename ssy@dl16:~/projects/NetBuffer/distnocache/$filename
