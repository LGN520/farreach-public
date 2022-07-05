DIRNAME="netreach-v4-lsm"

ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; ./loadfinish_client"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; ./loadfinish_client"
