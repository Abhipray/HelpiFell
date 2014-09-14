#include<pebble.h>
#define BUF_SIZE 10
#define THRESH 25000000
  
static uint32_t accel_hist[BUF_SIZE];

extern void send_message(int fall); //Defined in app_message.c
extern void launch_fall_window(void);

void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Process 10 events - every 1 second
    APP_LOG(APP_LOG_LEVEL_DEBUG, "accel_data_handler");

    //Add to buffer
    uint32_t min = accel_hist[0], max = accel_hist[0], i = 0;
      
    for(i = 0 ;i < num_samples; i++)
    {
      accel_hist[i] = (uint32_t)((data[i].x)*(data[i].x) + (data[i].y)*(data[i].y) + (data[i].z)*(data[i].z));
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "x:%d y:%d z:%d", data[i].x, data[i].y, data[i].z); 
    }
  
    for(i = 0 ;i < num_samples; i++)
    {
      if(accel_hist[i] < min)
        min = accel_hist[i];
      else if(accel_hist[i] > max)
        max = accel_hist[i];
    }
    
    if(max - min > THRESH)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ARE YOU OK?"); 
      //Reset accelerometer history
      for(i = 0; i < num_samples; i++)
        accel_hist[i] = 0;
      launch_fall_window();          
    }
}