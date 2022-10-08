filename=$1

scp $filename ssy@bf1:~/NetBuffer/distfarreachlimit/$filename
scp $filename ssy@bf3:~/NetBuffer/distfarreachlimit/$filename
scp $filename ssy@dl13:~/projects/NetBuffer/distfarreachlimit/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/distfarreachlimit/$filename
scp $filename ssy@dl16:~/projects/NetBuffer/distfarreachlimit/$filename
