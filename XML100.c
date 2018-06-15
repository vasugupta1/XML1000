/*Name: VASU GUPTA,
EMAIL:vasugupta1@gmail.com
Description: NESC Code which is used with XML1000 to collect temp, light values
based on the values we can broadcast signal to other XML1000 nodes for alarm.

3 Total Stages within the code

*/
#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include <stdio.h>
#include "dev/leds.h"

int buffersize = 12; //buffer used in the main function

unsigned short d1(float f) // Integer part
{
  return((unsigned short)f);
}


////This function will get the lux values everytime its called.
float getlight(){
float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096;
                  // ^ ADC-12 uses 1.5V_REF
    float I = V_sensor/100000; // xm1000 uses 100kohm resistor
    float light_lx = 0.625*1e6*I*1000;
   // printf("%uu lux\n", d1(getlight()));
    leds_on(LEDS_ALL);
    return light_lx;

}

//this function will print the array given to it
void printarray(int arr[],int size){
  int i ;
  for(i=0;i<size;i++){
     printf("%u lux\n", arr[i]);
  }
}

////This function will take the mean of the values in the buffer.
short mean (int arr[]){
  short mean;
  int add =0;
  int i ;
  for(i=0;i<buffersize;i++){
      add = add + arr[i];
  }
  mean = (add/ buffersize);
  return mean;
}

////This method was take from stackoverflow-
////This method calc the squareroot when provided with a number
short sqroot(short square)
{
    short root=square/3;
    int i;
    if (square <= 0) return 0;
    for (i=0; i<32; i++)
        root = (root + square / root) / 2;
    return root;
}

///this function calculates the std of a given buffer.

short std(int arr[],short mean){
  short std1;
  int i;
  short sum =0;
  short usearray[buffersize];
  for(i=0;i<buffersize;i++){
    usearray[i]=((arr[i]-mean)*(arr[i]-mean));
  }
  for(i=0;i<buffersize;i++){
    sum =sum + usearray[i];
  }
  short v1 = (sum/buffersize);
  std1 = sqroot(v1);
  printf("std:%u\n",std1 );
  return std1;
}

/*
Stage2 occurs when std value is between 20 and 100.(This relates to decent change in std)
this function will average first six and last six values of the buffer and take the average of them, the values are then ready to be sent but in this programe the values are just printed using the printarray function
*/
void stage2(int arr1[]){

  short arrayuse[2];

  int i;
  int b;
  int sum=0;
  int sum2=0;
  int samplesize=6;


      for(b=0;b<samplesize;b++){
        sum = sum + arr1[b];
        sum2 = sum2 + arr1[b+6];
      }

      arrayuse[0] = (sum/6);
      arrayuse[1]= (sum2/6);
      printf("STAGE2:\n");
     printarray((int)arrayuse,2);



}
/*
Stage2 occurs when std value is over 100.(this relates to large change in the std)
this function will average four values of the buffer and take the average of them, the values are then ready to be sent but in this programe the values are just printed using the printarray function
*/

void stage3(int arr2[]){

short arrayuse[3];

  int i;
  int b;
  int sum=0;
  int sum2=0;
  int sum3 =0;
  int samplesize=4;


      for(b=0;b<samplesize;b++){
        sum = sum + arr2[b];
        sum2 = sum2 + arr2[b+4];
        sum3 = sum3 + arr2[b+8];
      }

      arrayuse[0] = (sum/4);
      arrayuse[1]= (sum2/4);
      arrayuse[2]= (sum3/4);
      printf("STAGE3\n");
       printarray((int)arrayuse,3);

}

/*---------------------------------------------------------------------------*/
PROCESS(sensor_reading_process, "start");
AUTOSTART_PROCESSES(&sensor_reading_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_reading_process, ev, data)
{
  ///////////main function.
  int index = 0;
  int havedata = 0;
  ////two arrays are used as buffers with size of 12 = buffer size
  int array1[buffersize];
  int array2[buffersize];
  static struct etimer timer;
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  while(havedata == 0 ) //////have data is a condation which is used in order to get 12 readings from the sensors.
  {
    etimer_set(&timer, CLOCK_CONF_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev=PROCESS_EVENT_TIMER);

        index = 0 ;
        while( index < buffersize){
            array1[index] = d1(getlight());
            index++;
            leds_off(LEDS_ALL);
            etimer_reset(&timer);
        }
        ////////////when 12 data have been collected and copied to two arrays the condation of have data is set to 1 in order to stop collecting data
        int c;
        for(c=0;c<buffersize;c++){
            array2[c] = array1[c];
        }

        havedata = 1;
        printarray(array1,buffersize);
       short stdvalue =  std(array2,mean(array2));///use the collected array to get the std and mean values and return is the std value
       /////////////when happens when the std values is with in certain threshold.
       if(stdvalue < 20){
        printf("STAGE1:mean = %d \n",mean(array2));
       }else if(stdvalue>=20 && stdvalue<=100){
          stage2(array2);
       }else if(stdvalue>100){
          stage3(array2);
       }
       ////in order to make this programe recussive havedata is equal to 0
       havedata=0;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
