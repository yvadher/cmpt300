# cmpt300
Operating system course projects

qemu-system-x86_64 -m 64M -hda ../debian_squeeze_amd64_standard.qcow2 -append "root=/dev/sda1 console=ttys0,115200n8 console=tty0" -kernel arch/x86_64/boot/bzImage -net nic,vlan=1 -net user,vlan=1 -redir tcp:2222::22

gcc -std=c99 -D _GNU_SOURCE -static process_ancestor_test.c -o test_process

scp -P 2222 test_process  root@localhost:~

