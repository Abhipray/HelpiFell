#include<pebble.h>
#define BUF_SIZE 100
#define THRESH 25000000
  


extern void send_message(int fall); //Defined in app_message.c
extern void launch_fall_window(void);

void accel_data_handler(AccelData *data, uint32_t num_samples) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "accel_data_handler");
  
    uint32_t min = 4000*4000, max = 0, i, tmp;
      
    for(i = 0 ;i < num_samples; i++)
    {
      tmp = (uint32_t)((data[i].x)*(data[i].x) + (data[i].y)*(data[i].y) + (data[i].z)*(data[i].z));
      if(tmp < min)
        min = tmp;
      if(tmp > max)
        max = tmp;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "x:%d y:%d z:%d", data[i].x, data[i].y, data[i].z); 
    }
     
    if(max - min > THRESH)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ARE YOU OK?"); 
      //Reset accelerometer history
      launch_fall_window();          
    }
}