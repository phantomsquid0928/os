#include "types.h"
#include "stat.h"
#include "user.h"



char buf[512];
char lines[512][1024];
int nums[512];
int line;

/**
 * mod
 * discription
 * usage: htac <lines> <filename>
*/
// void htac(int fd) {
//   int n, i = 0, j = 0, k = 0;
  
//   while(1) {
//     char temp;
//     if ((n = read(fd, &temp, sizeof(temp))) <= 0) break;
//     ///printf(1, "%c", temp);
//     lines[i % line][j] = temp;
//     if (temp == '\n') {
//       nums[i % line] = j;
//       j = 0;
//       i++;
//       memset(lines[i % line], 0, sizeof(lines[i % line]));
//     }
//     else {
//       j++;
//     }
//   }

//   for (k = line - 1; k >= 0; k--) {

//     //printf(1, "%s", lines[(k + i + 1) % line]);
//     if (write(1, lines[(k + i + 1) % line], nums[(k + i + 1) % line]) != nums[(k + i + 1) % line]) {
//       printf(1, "htac: write error\n");
//     }
//     char temp = '\n';
//     write(1, &temp, 1);

//   }
// }
/**
 * mod
 * discription
 * usage : htac <LINES> <FILENAME>
*/

void htac(int fd)
{
  int n, i = 0;
  int cnt = 0;
  lseek(fd, 0, 2); 
  char temp = 0;
  //printf(1, ":::: %d\n", sizeof(temp));
  int last = -1;
  while(1) {
    i++;
    if (lseek(fd, -2, 0) == 2) { //reached file front
    printf(11 , "REACHED\n\n");
      n = read(fd, buf, sizeof(buf));
      if (write(1, buf, n) != n) {
        printf(1, "htac: write error\n");
        exit();
      }
      break;
    }
    n = read(fd, &temp, sizeof(temp));
    if (temp == '\n') {
      cnt++;
      if (last == -1) {
        while((n = read(fd, buf, sizeof(buf))) > 0) {
          if (write(1, buf, n) != n) {
            printf(1, "htac: write error\n");
            exit();
          }
          write(1, "\n", 1);
        }
        if (n < 0) {
          printf(1, "htac: read error\n");
          exit();
        }
      }
      else {
        int cur = i;
        while((n = read(fd, buf, cur > sizeof(buf) ? sizeof(buf) : cur)) > 0) {
          if (write(1, buf, n) != n) {
            printf(1, "htac: write error\n");
            exit();
          }
          cur = cur - n;
        }
      }
      lseek(fd, -i, 0);
      last = i;
      i = 0;
    }
    if (cnt == line) break;
  } 
}

  // while((n = read(fd, buf, sizeof(buf))) > 0) {
  //   if (write(1, buf, n) != n) {
  //     printf(1, "htac: write error\n");
  //     exit();
  //   }
  // }
  // if(n < 0){
  //   printf(1, "htac: read error\n");
  //   exit();
  // }
  //////
  // if ((n = read(fd, buf, sizeof(buf))) < 0) {
  //     printf(1, "htac: read error\n");
  //     exit();
  // }
  // if (write(1, buf, n) != n) {
  //     printf(1, "htac: write error\n");
  //     exit();
  // }


//   while((n = read(fd, buf, sizeof(buf))) > 0) {
//     if (write(1, buf, n) != n) {
//       printf(1, "htac: write error\n");
//       exit();
//     }
//   }
//   if(n < 0){
//     printf(1, "htac: read error\n");
//     exit();
//   }
//

int
main(int argc, char *argv[])
{
  int fd;

  if(argc <= 2 || argc >= 4){  //args 길이 3이 아니면 예외처리
    //cat(0);
    printf(1, "htac: usage >> htac <lines> <filename>\n");
    exit();
  }
  line = atoi(argv[1]);

  if((fd = open(argv[2], 0)) < 0){
    printf(1, "htac: cannot open %s\n", argv[2]);
    exit();
  }
  htac(fd);
  close(fd);
  exit();
}
