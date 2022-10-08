filename=$1

scp $filename ssy@bf1:~/NetBuffer/farreach/$filename
scp $filename ssy@dl13:~/projects/NetBuffer/farreach/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/farreach/$filename
scp $filename ssy@dl16:~/projects/NetBuffer/farreach/$filename
