rm mydisk
sudo dd if=/dev/zero of=mydisk bs=1024 count=1440
sudo mkfs mydisk 1440
(cd /mnt; sudo rmdir lost+found)
sudo umount /mnt