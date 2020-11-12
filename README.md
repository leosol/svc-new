# SVC-NEW Ransomware
Just sharing some knoledge related to SVC-NEW (a type of ransomware in linux environments - MD5 4bb2f87100fca40bfbb102e48ef43e65). 

**Summary**: 
Encryption ratio was only 0.3% of a 1GB file - located at the begining of the file (see samples)



### Ransomware logic - based on file size
This ransomware loads a table with it's behavior based on file size.
At first, you can find this table under global variable **Ransom_Logic**.
*Even* rows are file size indexes. *Odd* rows are the size of encrypted blocks. 

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
I did some testing with different file sizes and noticed that the part of the code that calculates the number of blocks that should be encrypted ***was only returning 1*** as number of blocks meaning that only one block should be encrypted (maybe a bug). My max file had **1GB** of size. Samples can be found here (ziped): [samples](./samples/).

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

### Encryption Ratio - only 0.3% of a 1GB file
```sh
-rw-r--r-- 1 leandro leandro      10240 Nov 11 15:10  file1.txt => (100%)
-rw-r--r-- 1 leandro leandro   20972032 Nov 11 15:12  file2.txt => (4%)
-rw-r--r-- 1 leandro leandro  167772160 Nov 11 15:10  file3.txt => (1.25%)
-rw-r--r-- 1 leandro leandro 1342110020 Nov 11 15:11  file4.txt => (0.3124%)
```

### File recovery based on backup
I'm not sure but, maybe, you can be able to recover your files if you have a *backup* and the encrypted part of it didn't change. If you work in a data center with multiple backups, it can be nice to compare the encrypted part with more than one backup. Guess that the larger the file, more chances you have to recover your data. 

**Don't forget to take out the last 512 bytes of the RSA signature!**
