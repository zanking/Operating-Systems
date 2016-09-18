#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> //file control options

#define DEVICE "/dev/simple_char_device"
#define BUFF_SIZE 1024

int main(){
  char answer, buffer[BUFF_SIZE];
  int file = open(DEVICE, O_RDWR);
  while(1){
    printf("\nPress r to read from device.\nPress w to write to device.\nPress e to exit the device.\nPress anything else to keep reading or writing from the device.\n\nEnter your command:");
    scanf("%c", &answer);

    switch(answer) {

      case 'w': case 'W':

        printf("Enter the data you want to write to the device:");
        scanf("%s", buffer);
        write(file, buffer, BUFF_SIZE);
        while(getchar() != '\n'); //make sure we dont fall in a loop
        break;

      case 'r': case 'R':

        read(file, buffer, BUFF_SIZE);
        printf("Data read from device: %s\n", buffer);
        while(getchar()!='\n');
        break;

      case 'e': case 'E':

        return 0;
      default:
        while(getchar()!='\n');
    }
  }
  close(file);
  return 0;
}
