//Nicholas Clement

typedef struct {
int value;
struct process *list;
} semaphore;
/*lock everything at fist*/

semaphore x1 = 0;
semaphore x2 = 0;
semaphore x3 = 0;
semaphore x4 = 0;
semaphore x5 = 0;

x1(){
  signal(x1);
}

x2(){
  wait(x1);
  /*wakeup all prior process*/
  signal(x1);
  signal(x2);
}

x3(){
  wait(x4);
  signal(x3);
  //Difficult because process x3 has to wait for process
  //x4 and process x4 has to wait for process x3
}

x4(){
  wait(x1);
  wait(x3);
  signal(x4);
}


x5(){
  wait(x1);
  wait(x3);
  wait(x4);
  signal(x5);
}
