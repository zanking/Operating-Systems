//Nicholas Clement

insertTrig = semaphore()
searchTrig = semaphore()

insertionMutex = semaphore()
isInserter = semaphore()
isSearcher = semaphore()
everything = semaphore()


)

/*Insert*/

/*start by making sure we lock out other inserters*/
insertTrig.wait(isInserter)
insertionMutex.wait()
/*execute critical section here*/
insertionMutex.signal() //wake up other inserters
insertTrig.signal(isInserter)

/*search*/

//make sure that deleters know we are in
searchTrig.wait(isSearcher)
/*critical section*/
searchTrig.signal(isSearcher) //signal that we are out

/*Delete*/

isInserter.wait()
isSearcher.wait()

/*critical section*/

isInserter.signal() //wakeup all other processes
isSearcher.signial()


//We know that deleter cares about searchers and inserters,
//so we have to watch for both

//Inserter only cares about deleter and other inserters

//reader only cares about deleters
