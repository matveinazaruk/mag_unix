FILENAME='test' 
MOUNT_ROOT='/mnt/labunix' 
dd of=$FILENAME if=/dev/zero bs=1M count=512 
sudo mount -o loop $FILENAME $MOUNT_ROOT 
sudo mkdir $MOUNT_ROOT/test1 
sudo mkdir $MOUNT_ROOT/test2 
sudo touch $MOUNT_ROOT/test1.txt 
sudo touch $MOUNT_ROOT/test2/test2.txt
