#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

BaseType_t ExtractCmd(command_t *cmd);
void ProcessCmd(command_t *cmd);
BaseType_t verify_rtc_info(RTC_TimeTypeDef * time, RTC_DateTypeDef * date);
curr_state = state_menu;

rtc_state_t rtc_state = cfg_unf;
RTC_TimeTypeDef TIME;
RTC_DateTypeDef DATE;

/* USER CODE BEGIN 4 */
void task_led_handler(void* parameter)
{
  const char* msg_led = "------------------\n"
                        "      LED       \n"
                        "------------------\n"
                        "(none, e1, e2, e3, e4)\n"
                        "Enter your choice here: ";

  const char* msg_inv = "Invalid command\n";
  
  uint32_t cmd_addr;
  command_t *cmd;

  for(;;)
  {
    // wait for the notification from menu task
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    // send message to host
    xQueueSend(g_queue_print, &msg_led, portMAX_DELAY);

    // wait for the led command from Print task
    xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);

    // extract led command
    cmd = (command_t*)cmd_addr;
    if (cmd->len <= 4)
    {
      if (!strcmp((char*)cmd->payload, "none"))
      {
        LedEffectStop();
      }
      else if (!strcmp((char*)cmd->payload, "e1"))
      {
        LedEffect(1);
      }
      else if(!strcmp((char*)cmd->payload, "e2"))
      {
        LedEffect(2);
      }
      else if (!strcmp((char*)cmd->payload, "e3"))
      {
        LedEffect(3);
      }
      else if (!strcmp((char*)cmd->payload, "e4"))
      {
        LedEffect(4);
      }
      else
      {
        xQueueSend(g_queue_print, &msg_inv, portMAX_DELAY);
      }
    }

    curr_state = state_menu;

    // back to menu task
    xTaskNotify(task_menu, 0, eNoAction);
  }
}

void task_menu_handler(void* parameter)
{
  const char* msg_menu = "------------------\n"
                          "      MENU       \n"
                          "------------------\n"
                          "Led effect: 0\n"
                          "Date and timer: 1\n"
                          "Exit: 2\n"
                          "Enter your choice here: ";
  
  const char* invalid_msg_menu = "Invalid command\n";

  uint32_t cmd_addr;
  command_t *cmd;
  uint8_t option;

  for(;;)
  {
    xQueueSend(g_queue_print, &msg_menu, portMAX_DELAY);

    // wait for the command
    xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
    cmd = (command_t*)cmd_addr;

    if (cmd->len == 1)
    {
      // process commnad
      option = cmd->payload[0] - 48;
      switch (option)
      {
        case 0:
        {
          curr_state = state_led;
          xTaskNotify(task_led,0, eNoAction);
        }
        break;

        case 1:
        {
          curr_state = state_rtc_menu;
          xTaskNotify(task_rtc, 0, eNoAction);
        }
        break;

        case 2:
        {
          break;
        }
        
        default:
          break;
      }
    }
    else
    {
      // send invalid message to host
      xQueueSend(g_queue_print, &invalid_msg_menu, portMAX_DELAY); // need to understand why &invlid_msg_menu
    }

    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); 
  }
}

void task_rtc_handler(void* parameter)
{
  const char* msg_rtc1 = "------------------\n"
                          "     RTC       \n"
                          "------------------\n";
  const char* msg_rtc2 =  "Config time: 0\n"
                          "Config date: 1\n"
                          "Enable reporting: 2\n"
                          "Exit: 3\n"
                          "Enter your choice here: ";  
  
  const char *msg_rtc_hh = "Hour: ";
  const char *msg_rtc_mm = "Minute: ";
  const char *msg_rtc_ss = "Second: ";

  const char *msg_rtc_dd = "Date: ";
  const char *msg_rtc_mo = "Month: ";
  const char *msg_rtc_dow = "Day of week: ";
  const char *msg_rtc_yr = "Year: ";

  const char *msg_conf = "Configuration success\n";
  const char *msg_rtc_report = "Enable reporting (y/n): ";

  const char *inv_msg = "Invalid message";

  for(;;)
  {
	  // waiting notification from menu task
	  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    xQueueSend(g_queue_print, &msg_rtc1, portMAX_DELAY);
    show_time_date();
    xQueueSend(g_queue_print, &msg_rtc2, portMAX_DELAY);

    command_t *cmd;
    while (curr_state != state_menu)
    {
      // handle command from user
      uint32_t msg_addr;
      xTaskNotifyWait(0 , 0, &msg_addr, portMAX_DELAY);
      cmd = (command_t*) msg_addr;
      switch (curr_state)
      {
        case state_rtc_menu:
        {
          if (cmd->len >= 1)
          {
            uint8_t menu_cmd = cmd->payload[0] - 48;
            switch (menu_cmd)
            {
              case 0:
              {
                curr_state = state_rtc_timeconfig;
              }
              break;
              
              case 1:
              {
                curr_state = state_rtc_dateconfig;
                xQueueSend(g_queue_print, &msg_rtc_dd, portMAX_DELAY);
              }
              break;
              
              case 2:
              {
                curr_state = state_rtc_report;
                xQueueSend(g_queue_print, &msg_rtc_report, portMAX_DELAY);
              }
              break;
              
              case 3:
              {
                curr_state = state_menu;
              }
              break;

              default:
              {
                curr_state = state_menu;
                xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
              }
              break;
            }
          }
        }
        break;
      
      case state_rtc_timeconfig:
      {
        switch(rtc_state)
        {
          case cfg_unf:
          {
            rtc_state = cfg_hh;
            xQueueSend(g_queue_print, &msg_rtc_hh, portMAX_DELAY);
          }
          break;

          case cfg_hh:
          {
              uint8_t hh = getnumber(cmd->payload, cmd->len);
              TIME.Hours = hh;
              rtc_state = cfg_mm;
              xQueueSend(g_queue_print, &msg_rtc_mm, portMAX_DELAY);
          }
          break;

          case cfg_mm:
          {
              uint8_t mm = getnumber(cmd->payload, cmd->len);
              TIME.Minutes = mm;
              rtc_state = cfg_ss;
              xQueueSend(g_queue_print, &msg_rtc_ss, portMAX_DELAY);
          }
          break;

          case cfg_ss:
          {
              uint8_t ss = getnumber(cmd->payload, cmd->len);
              TIME.Seconds = ss;
              rtc_state = cfg_mm;
              if ( verify_rtc_info(&TIME, NULL) == pdTRUE)
              {
                rtc_config_time(&TIME);
                xQueueSend(g_queue_print, &msg_conf, portMAX_DELAY);
                show_time_date();
              }
              else
              {
                xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
              }
              curr_state = state_menu;
              rtc_state = cfg_unf;
          }
          break;

          default:
          {
            xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
          }
          break;
        }
      }
      break;

      case state_rtc_dateconfig:
      {
        switch (rtc_state)
        {
          case cfg_unf:
          {
            rtc_state = cfg_date;
          }
          break;

          case cfg_date:
          {
            uint8_t date = getnumber(cmd->payload, cmd->len);
            DATE.Date = date;
            rtc_state = cfg_DoW;
          }
          break;

          case cfg_DoW:
          {
            uint8_t Dow = getnumber(cmd->payload, cmd->len);
            DATE.WeekDay = Dow;
            rtc_state = cfg_month;
          }
          break;

          case cfg_month:
          {
            uint8_t month = getnumber(cmd->payload, cmd->len);
            DATE.Month = month;
            rtc_state = cfg_month;
          }
          break;

          case cfg_year:
          {
            uint8_t year = getnumber(cmd->payload, cmd->len);
            DATE.Year = year;
            if (verify_rtc_info(NULL, &DATE) == pdTRUE)
            {
              rtc_config_date(&DATE);
              xQueueSend(g_queue_print, &msg_conf, portMAX_DELAY);
              curr_state = state_menu;
              rtc_state = cfg_unf;
            }
            else
            {
              xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
            }
          }
          break;

          default:
          {
            xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
          }
          break;
        }
      }
      break;

      default:
      {
        xQueueSend(g_queue_print, &inv_msg, portMAX_DELAY);
      }
      break;      
    }
  }
}
}


void task_print_handler(void* parameter)
{
  uint32_t *msg;
  for(;;)
  {
    xQueueReceive(g_queue_print, &msg, portMAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen((char*)msg), portMAX_DELAY);
  }
}

void task_command_handler(void* parameter)
{
  BaseType_t val = pdFAIL;
  command_t cmd;
  for(;;)
  {
    val = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    if (val == pdTRUE)
    {
      ProcessCmd(&cmd);
    }    
  }
}

BaseType_t ExtractCmd(command_t *cmd)
{
    // extract cmd from user_data

    // check len of queue
    UBaseType_t len = uxQueueMessagesWaiting(g_queue_data);
    if (!len)
    {
      return pdFAIL;
    }
    else
    {
      uint8_t index = 0;
      uint8_t data;
      do
      {
        if (xQueueReceive( g_queue_data, (void*)&data, 0))
        {
          cmd->payload[index++] = data;
        }
      } while (data != '\n');
      cmd->payload[index-1] = '\0';
      cmd->len = index - 1;
    }
    return pdTRUE;
}

void ProcessCmd(command_t *cmd)
{
  if(ExtractCmd(cmd) == pdTRUE)
  {
    switch(curr_state)
    {
      case state_menu:
      {
        xTaskNotify(task_menu, (uint32_t)cmd, eSetValueWithOverwrite);
      }
      break;

      case state_led:
      {
        xTaskNotify(task_led, (uint32_t)cmd, eSetValueWithOverwrite);
      }
      break;

      case state_cmd:
      {
        xTaskNotify(task_command, (uint32_t)cmd, eSetValueWithOverwrite);
      }
      break;

      case state_print:
      {
        xTaskNotify(task_print, (uint32_t)cmd, eSetValueWithOverwrite);
      }
      break;

      case state_rtc_menu:
      case state_rtc_timeconfig:
      case state_rtc_dateconfig:
      case state_rtc_report:
      {
        xTaskNotify(task_rtc, (uint32_t)cmd, eSetValueWithOverwrite);
      }
      break;
    }
  }
  else
  {
    // process cmd fail
  }
}

// used by the rtc task

void show_time_date(void)
{
  static char showdate[40];
  static char showtime[40];

  RTC_DateTypeDef rtc_date;
  RTC_TimeTypeDef rtc_time;

  memset(&rtc_date, 0, sizeof(rtc_date));
  memset(&rtc_time, 0, sizeof(rtc_time));

  HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);

  char* format;
  if (rtc_time.TimeFormat == RTC_HOURFORMAT12_AM) 
  {
    format = "AM";  
  }
  else
  {
    format = "PM";
  }

  sprintf((char*)showtime, "Current time day: %02d:%02d:%02d [%s]", rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds, format);
  xQueueSend(g_queue_print, &showtime, portMAX_DELAY);

  sprintf((char*)showdate, " %02d-%02d-%02d", rtc_date.Month, rtc_date.Date, 2000 + rtc_date.Year);
  xQueueSend(g_queue_print, &showdate, portMAX_DELAY);

}

void rtc_config_time(RTC_TimeTypeDef *time)
{
  time->TimeFormat = RTC_HOURFORMAT12_AM;
  time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  time->StoreOperation = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetTime(&hrtc, time, RTC_FORMAT_BIN);
}

void rtc_config_date(RTC_DateTypeDef *date)
{
  HAL_RTC_SetDate(&hrtc, date, RTC_FORMAT_BIN);
}

uint8_t getnumber(uint8_t *msg, uint8_t len)
{
  if (len == 0 )
  {
    return 0;
  }

  uint8_t num = 0;
  uint8_t exp = len-1;
  for (int i = 0; i < len; i++)
  {
    num = num + pow(10, exp--)*(msg[i] - 48);
  }
  return num;
}

BaseType_t verify_rtc_info(RTC_TimeTypeDef * time, RTC_DateTypeDef * date)
{
  if (time != NULL)
  {
    if ((time->Hours < 1 ) || (time->Hours > 12)
        || (time->Minutes < 0 ) || (time->Minutes > 59)
        || (time->Seconds < 0 ) || (time->Seconds > 59))
    {
      return pdFAIL;
    }
  }

  if (date != NULL)
  {
    if ((date->Date < 1) || (date->Date > 31)
        || (date->Month < 1) || (date->Month > 12)
        || (date->Year < 0) || (date->Year > 99)
        || (date->WeekDay < 0 || date->WeekDay >7))
    {
      return pdFAIL;
    } 
  }

  return pdTRUE;
}

