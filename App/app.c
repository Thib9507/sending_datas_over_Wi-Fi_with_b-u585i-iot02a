#include "bsp.h"
#include "mx_wifi.h"
#include "mx_wifi_io.h"
#include "mx_wifi_bare_os.h"

// define the wifi module secondaries pins (not the spi pins)

#define MXCHIP_SPI              Wifi_SPIHandle
#define MXCHIP_FLOW_Pin         GPIO_PIN_15
#define MXCHIP_FLOW_GPIO_Port   GPIOG
#define MXCHIP_FLOW_EXTI_IRQn   EXTI15_IRQn
#define MXCHIP_RESET_Pin        GPIO_PIN_15
#define MXCHIP_RESET_GPIO_Port  GPIOF
#define MXCHIP_NSS_Pin          GPIO_PIN_12
#define MXCHIP_NSS_GPIO_Port    GPIOB
#define MXCHIP_NOTIFY_Pin       GPIO_PIN_14
#define MXCHIP_NOTIFY_GPIO_Port GPIOD
#define MXCHIP_NOTIFY_EXTI_IRQn EXTI14_IRQn



#ifdef MX_WIFI_API_DEBUG
#define DEBUG_LOG(M, ...)  printf((M), ##__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(M, ...)  /*;*/
#endif /* MX_WIFI_API_DEBUG */

/*void SPI2_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&Wifi_SPIHandle);
}*/

/**
  * @brief This function handles EXTI Line13 interrupt. (Notify_pin of spi) need it to change the state and to speak to/ listen from the module
  */
void EXTI14_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MXCHIP_NOTIFY_Pin);
}

/**
  * @brief This function handles EXTI Line15 interrupt. (flow pin of spi) need it to change the state and to speak to/ listen from the module
  */
void EXTI15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MXCHIP_FLOW_Pin);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void Wifi_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MXCHIP_RESET_GPIO_Port, MXCHIP_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MXCHIP_NSS_GPIO_Port, MXCHIP_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : MXCHIP_FLOW_Pin */
  GPIO_InitStruct.Pin = MXCHIP_FLOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MXCHIP_FLOW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MXCHIP_RESET_Pin */
  GPIO_InitStruct.Pin = MXCHIP_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MXCHIP_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : MXCHIP_NSS_Pin */
  GPIO_InitStruct.Pin = MXCHIP_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MXCHIP_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MXCHIP_NOTIFY_Pin */
  GPIO_InitStruct.Pin = MXCHIP_NOTIFY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MXCHIP_NOTIFY_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI14_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI14_IRQn);

  HAL_NVIC_SetPriority(EXTI15_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_IRQn);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    /* MXCHIP flow pin notification */
    case MXCHIP_FLOW_Pin:
    {
      mxchip_WIFI_ISR(MXCHIP_FLOW_Pin);
      break;
    }

    /* MXCHIP notify pin notification */
    case (MXCHIP_NOTIFY_Pin):
    {
      mxchip_WIFI_ISR(MXCHIP_NOTIFY_Pin);
      break;
    }

  default:
    {
      break;
    }
  }
}







//empty structure to feed mxwifi_probe function in hw_start
typedef struct
{
	void *probe_attribute;
}probe;


// function hw_start from the IOT project but adapted (consider the structure above
static int32_t hw_start(/*net_if_handle_t *pnetif */probe *probe_object)
{
  int32_t ret = /*NET_ERROR_DEVICE_ERROR*/-13;

  if (mxwifi_probe(/*&pnetif->pdrv->context = 0 ?*/ &probe_object->probe_attribute) == 0) // function to indicate witch function should be use for the SPI
  {
    DEBUG_LOG("%s\n", "MX_WIFI IO [OK]");

    if (wifi_obj_get()->Runtime.interfaces == 0U)
    {
       // WiFi module hardware reboot.
      DEBUG_LOG("%s\n", "MX_WIFI REBOOT(HW) ...");
      ret = MX_WIFI_HardResetModule(wifi_obj_get());         // Not an error, it is mandatory
    }
    else
    {
      ret = MX_WIFI_STATUS_OK;
    }

    if (MX_WIFI_STATUS_OK != ret)
    {
      ret = /*NET_ERROR_DEVICE_ERROR*/-13;
    }
    else
    {
      /* Wait for Mxchip WiFi reboot. */

      /* Initialize the WiFi module. */
      if (MX_WIFI_STATUS_OK != MX_WIFI_Init(wifi_obj_get())) // function to init the WiFi module communication and to verify it with 2 requests : ask version and ask MAC adress
      {
        ret = /*NET_ERROR_INTERFACE_FAILURE*/-17;
      }
      else
      {
        /*DEBUG_LOG("%s\n", "MX_WIFI_Init [OK]");
         //Retrieve the WiFi module information.
        (void)strncpy(pnetif->DeviceName, (const char *)wifi_obj_get()->SysInfo.Product_Name, sizeof(pnetif->DeviceName));
        (void)strncpy(pnetif->DeviceID, (const char *)wifi_obj_get()->SysInfo.Product_ID, sizeof(pnetif->DeviceID));
        (void)strncpy(pnetif->DeviceVer, (const char *)wifi_obj_get()->SysInfo.FW_Rev, sizeof(pnetif->DeviceVer));
*/
        //(void)MX_WIFI_GetMACAddress(wifi_obj_get(), pnetif->macaddr.mac);

        /* Set bypass mode. */
    	  /*
#if (defined(MX_WIFI_NETWORK_BYPASS_MODE) && (MX_WIFI_NETWORK_BYPASS_MODE == 1))
        if (MX_WIFI_STATUS_OK != MX_WIFI_Network_bypass_mode_set(wifi_obj_get(), 1,
                                                                 net_mx_wifi_netlink_input_callback, pnetif))
        {
          NET_DBG_ERROR("*** set mx_wifi module bypass mode failed!\n");
          ret = NET_ERROR_MODULE_INITIALIZATION;
        }
        else
#endif /* (MX_WIFI_NETWORK_BYPASS_MODE == 1) */
        {
          ret = /*NET_OK*/0;
        }
      }
    }
  }

    return ret;
}










void app_main( void) {
    /* Initialize bsp resources */
    bsp_init();

    MX_WIFI_STATUS_T a;


//a   = MX_WIFI_Connect(MX_WIFIObject_t *Obj, const mx_char_t *SSID, const mx_char_t *Password, MX_WIFI_SecurityType_t SecType);

    const mx_char_t *SSID = "iPhone_de_Thibault";
    const mx_char_t *Password = "88888888";

    uint8_t *mac =1;

    // initialisation of the wifi module secondaries pins (not the spi pins)
    {
    	Wifi_IO_Init();
    }

    // initialisation of the module request
    {
		int32_t ret_hw_start;

		probe probe_object; // // ~ pnetif for hw_start

		ret_hw_start= hw_start(&probe_object);
    }


    MX_WIFI_STATUS_T MX_WIFI_GetVersion(MX_WIFIObject_t *Obj, uint8_t *version, uint32_t size);



    //PendingRequest.req_id = MIPC_REQ_ID_RESET_VAL;
    // a= MX_WIFI_Init(wifi_obj_get());

    //int32_t mx_wifi_hci_init(hci_send_func_t low_level_send)

    a = MX_WIFI_GetsoftapMACAddress(wifi_obj_get(), mac);

    a = MX_WIFI_Connect(wifi_obj_get(), SSID, Password, MX_WIFI_SEC_WPA_AES);

    if (a==MX_WIFI_STATUS_OK)    {
        HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_7);
    }
    else {
    HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_6);
    }





    while(1){

    }

    return;
}
