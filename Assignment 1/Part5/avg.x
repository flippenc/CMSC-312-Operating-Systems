/*
 * The average procedure receives an array of real
 * numbers and returns the average of their
 * values. This toy service handles a maximum of
 * 200 numbers.
 * http://www.linuxjournal.com/article/2204?page=0,1
 */

const MAXAVGSIZE  = 200;

struct input_data 
  {
  char input_data<200>;
  };

program AVERAGEPROG {
    version AVERAGEVERS {
        input_data AVERAGE(input_data) = 1;
    } = 1;
} = 22855;
