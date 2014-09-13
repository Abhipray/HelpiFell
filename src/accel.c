#include<accel.h>
#define BUF_SIZE 5
#define THRESH 25000000
  
static uint32_t accel_hist[BUF_SIZE];
static uint32_t accel_index = 0;

extern void send_message(void); //Defined in app_message.c

void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Process 10 events - every 1 second
    char buf[80];
    snprintf(buf, 80, "x:%d y:%d z:%d\n", data->x, data->y, data->z);
    APP_LOG(APP_LOG_LEVEL_DEBUG, buf);  
    accel_hist[accel_index] = (data->x)*(data->x) + (data->y)*(data->y) + (data->y)*(data->y);
    accel_index++;
    uint32_t min = accel_hist[0], max = accel_hist[0], i = 0;
      
    for(;i < BUF_SIZE; i++)
    {
      if(accel_hist[i] < min)
        min = accel_hist[i];
      else if(accel_hist[i] > max)
        max = accel_hist[i];
    }
    if(accel_index == BUF_SIZE)
      accel_index = 0;
    if(max - min > THRESH)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ARE YOU OK?"); 
      send_message();
    }
}