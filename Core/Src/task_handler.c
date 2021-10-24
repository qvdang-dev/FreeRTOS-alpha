#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

BaseType_t ExtractCmd(command_t *cmd);
void ProcessCmd(command_t *cmd);
curr_state = state_menu;

/* USER CODE BEGIN 4 */
void task_led_handler(void* parameter)
{
  for(;;)
  {

  }
}

void task_menu_handler(void* parameter)
{
  for(;;)
  {
    
  }
}

void task_rtc_handler(void* parameter)
{
  for(;;)
  {
    
  }
}

void task_print_handler(void* parameter)
{
  for(;;)
  {
    
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

      case state_rtc:
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
