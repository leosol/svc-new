# SVC-NEW Ransomware
Just sharing some knoledge related to SVC-NEW (a type of ransomware in linux environments - MD5 4bb2f87100fca40bfbb102e48ef43e65). 
...
I'm just an amateur trying to learn a little bit of reverse engineering. So, I appreciate tips, suggestions and criticisms too, especially those that help me improve! I hope it will be useful in some way. Good luck!

### Ransomware logic - based on file size
This ransomware loads a table with the behavior based on file size.
At first, you can find this table under global named variable **Ransom_Logic**.
Even rows are file size indexes. Odd rows are the size of encrypted blocks.

![Alt text](pictures/Capture-Dump-Ransom-Logic.PNG?raw=true "Mem Dump")

```
0.  0x0             0xa00000
1.  0x100000        0x300000
2.  0xa00000        0x6400000
3.  0x100000        0x1e00000
4.  0x6400000       0x40000000
5.  0x200000        0xc800000
6.  0x40000000      0x280000000
7.  0x400000        0x40000000
8.  0x280000000     0x1900000000
9.  0x800000        0x280000000
10. 0x1900000000    0x640000000000
11. 0x800000        0x1900000000
```
This table can be translated to decimal/byte values below.
I did some testing with different file sizes and noticed that the part of the code that calculates the number of blocks that should be read ***was only returning 1*** as number of blocks. Meaning that only one block should be encrypted. My max file had **1GB** of size.

```
0x0           0xa00000      => 0x100000 (10240 bytes) 
0xa00000      0x6400000     => 0x100000 (10240 bytes) 
0x6400000     0x40000000    => 0x200000 (2097152 bytes)
0x40000000    0x280000000   => 0x400000 (4194304 bytes)
0x280000000   0x1900000000  => 0x800000 (8388608 bytes)
0x1900000000  0x640000000000 => 0x800000 (8388608 bytes)

0          -> 10485760        (10.49MB) (10240 bytes encrypted, or 3145728, or a multiple of both) 
10485760   -> 104857600       (104.9MB) (10240 bytes encrypted, or 31457280, or a multiple of both) 
104857600  -> 1073741824      (1.074GB) (2097152 bytes encrypted, or 209715200, or a multiple of both)
1073741824 -> 10737418240     (10.74GB) (4194304 bytes encrypted, or 10737418240, or a multiple of both)
10737418240 -> 107374182400   (107GB)	  (8388608 bytes encrypted, or 107374182400, or ...)
107374182400-> 109951162777600 (110 TB)  (8388608 bytes encrypted, you already know right?)
```

### File recovery based on backup
I'm not sure but maybe, you can be able to recover your files if you have a *backup* and the modified part of didn't change. If you work in a data center with multiple backups, it can be nice to compare the modified part with more than one backup. Guess that the larger the file, more chances you have. 
**Don't forget to take out the last 512 bytes of the RSA signature!**
