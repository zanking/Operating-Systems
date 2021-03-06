#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> //file control options
#include <string.h>

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
        // gather user input as buffer and write with size of length of buffer
        write(file, buffer, strlen(buffer));
        while(getchar() != '\n'); //make sure we dont fall in a loop
        break;

      case 'r': case 'R':

        read(file, buffer, strlen(buffer));
        printf("Data read from device: %s\n", buffer);
        // read in the entire buffer and print it
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
