//Nicholas Clement

/*Combos
  4 hackers 0 ms
  4 ms 0 hackers
  2 hackers 2 ms
  */

  int numberHackers;
  int numberEmp;

  HackerArrives(){
    numberHackers++;
    signal(counter);
  }

  EmpArrives(){
    numberEmp++;
    signal(counter);
  }

  /*define a count helper function*/

  monitor_Counter(){

    void checknumbers(numberEmp,numberHackers){
      //check if we have a multiple of twos for
      //both the hackers and emps
      if numberHackers%2 == 0 && numberEmp%2 == 0{
        BoardBoat();
        RowBoat();
        numberEmp = numberEmp - 2;
        numberHackers = numberHackers -2;
        return();
      }
      //check if we have a multiple of four for emps

      else if numberEmp%4 ==0{
        BoardBoat(numberEmp);
        RowBoat(nubmerEmp);
        numberEmp = numberEmp -4;
        return();
      }
      //check if we have amultiple of four for hackers
      else if numberHackers%4 ==0{
        boardBoat(numberHackers);
        RowBoat(numberHackers);
        numberHackers = numberHackers -4;
        return();
      }
    }

    init(){
      void checknumbers();
    }
  }
