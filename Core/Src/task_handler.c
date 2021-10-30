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
          curr_state = state_rtc;
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
      xQueueSend(g_queue_print, &invalid_msg_menu, portMAX_DELAY);
    }

    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); 
  }
}

void task_rtc_handler(void* parameter)
{
  for(;;)
  {
	  // waiting notification from menu task
	  xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
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
