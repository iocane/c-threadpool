#### About

This is an example of a thread pool, where a fixed number of threads
operate on fixed size queue of jobs. Jobs are represented as function 
pointers with user specified input. When jobs are added to the queue 
work threads pick them up and execute until completion. The job queue
is protected by a mutex and worker thread availability is managed with
a semaphore.

#### Building
1. Install [tup](http://gittup.org/tup/index.html).
1. Run 
   ```
   $ tup
   ```

#### Usage
The API is described in `threadpool.h`.

#### Tests
Run 
   ```
   test/test
   ``` 
to test and benchmark the library.
