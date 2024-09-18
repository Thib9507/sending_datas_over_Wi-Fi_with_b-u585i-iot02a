#include "bsp.h"
#include "mx_wifi.h"
#include "mx_wifi_io.h"
#include "mx_wifi_bare_os.h"




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
    /*DEBUG_LOG("%s\n", "MX_WIFI IO [OK]");

    if (wifi_obj_get()->Runtime.interfaces == 0U)
    {
       // WiFi module hardware reboot.
      DEBUG_LOG("%s\n", "MX_WIFI REBOOT(HW) ...");
      ret = MX_WIFI_HardResetModule(wifi_obj_get());
    }
    else
    {*/
      ret = MX_WIFI_STATUS_OK;
    //}

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
