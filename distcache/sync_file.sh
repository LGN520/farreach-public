filename=$1

scp -r $filename ssy@bf1:~/NetBuffer/distcache/$filename
scp -r $filename ssy@bf3:~/NetBuffer/distcache/$filename
scp -r $filename ssy@dl13:~/projects/NetBuffer/distcache/$filename
scp -r $filename ssy@dl15:~/projects/NetBuffer/distcache/$filename
scp -r $filename ssy@dl16:~/projects/NetBuffer/distcache/$filename
