./mkdisk.sh
rm a.out
gcc -m32 main.c
sudo ./a.out diskimage