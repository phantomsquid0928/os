// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

#define NDIRECT 6 //12 4 + 2 * 128 + 128 * 128 6 

#define LEVEL1SH 4
#define LEVEL2SH 2
#define LEVEL3SH 1
#define LEVEL1 (LEVEL1SH * BSIZE / sizeof(uint))
#define LEVEL2 (LEVEL2SH * BSIZE / sizeof(uint) * BSIZE / sizeof(uint))
#define LEVEL3 (LEVEL3SH * BSIZE / sizeof(uint) * BSIZE / sizeof(uint) * BSIZE / sizeof(uint))
// #define NINDIRECT (16644 * BSIZE / sizeof(uint)) //bsize / sizeof(uint) 16644, indirect datas bsize / sizeof(uint) = 128 -> 128개의 indirect datas, 1block -> 128개
#define NINDIRECT LEVEL1 + LEVEL2 + LEVEL3
// #define NINDIRECT BSIZE / sizeof(uint)
#define MAXFILE (NDIRECT + NINDIRECT) // 12 + 128 = 140

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+7];   // Data block addresses ndirect + 1  NDIRECT+7
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

