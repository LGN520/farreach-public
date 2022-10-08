filename=$1

scp $filename ssy@bf1:~/NetBuffer/distfarreach/$filename
scp $filename ssy@bf3:~/NetBuffer/distfarreach/$filename
scp $filename ssy@dl13:~/projects/NetBuffer/distfarreach/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/distfarreach/$filename
scp $filename ssy@dl16:~/projects/NetBuffer/distfarreach/$filename
