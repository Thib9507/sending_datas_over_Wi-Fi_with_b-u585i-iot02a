#include "bsp.h"
#include "mx_wifi.h"
#include "mx_wifi_io.h"
#include "mx_address.h"

#include <stdio.h>
#include <string.h>

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


#define NET_IPPROTO_TCP         6


#ifdef MX_WIFI_API_DEBUG
#define DEBUG_LOG(M, ...)  printf((M), ##__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(M, ...)  /*;*/
#endif /* MX_WIFI_API_DEBUG */

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

typedef char            char_t;

//empty structure to feed mxwifi_probe function in hw_start
typedef struct
{
	void *probe_attribute;
}probe;


typedef enum { // Structure to indicate the json to send
    AUTOTEST = 0,
	KEEPALIVE = 1,
	NOTHING = 2
} Mode;


typedef enum { // structure to indicate the app status
    APP_OK = 0,
	HW_START_FAILED = -1,
	WIFI_CONNECTION_FAILED = -2,
	IP_REQUEST_FAILED = -3,
	SOCKET_CREATION_FAILED = -4,
	SOCKET_CONNECTION_FAILED = -5,
	POST_REQUEST_SENDING_FAILED =-6,
	POST_REQUEST_RECEIVING_FAILED=-7,
	SOCKET_CLOSING_FAILED=-8
} App_status;


// function hw_start adapted from the initial project (consider the structure above)
static int32_t hw_start(/*net_if_handle_t *pnetif */probe *probe_object)
{
  int32_t ret = /*NET_ERROR_DEVICE_ERROR*/-13;

  if (mxwifi_probe(&probe_object->probe_attribute) == 0) // function to indicate witch function should be use for the SPI
  {
    DEBUG_LOG("%s\n", "MX_WIFI IO [OK]");

    if (wifi_obj_get()->Runtime.interfaces == 0U)
    {
       // WiFi module hardware reboot.
      DEBUG_LOG("%s\n", "MX_WIFI REBOOT(HW) ...");
      ret = MX_WIFI_HardResetModule(wifi_obj_get());         // Not an error, it is mandatory for the init
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
      /* Initialize the WiFi module. */
      if (MX_WIFI_STATUS_OK != MX_WIFI_Init(wifi_obj_get())) // function to init the WiFi module communication and to verify it with 2 requests : ask version and ask MAC adress
      {
        ret = /*NET_ERROR_INTERFACE_FAILURE*/-17;
      }
      else
      {
          ret = /*NET_OK*/0;
      }
    }
  }
    return ret;
}


// Setting of the application

// network info
const mx_char_t *SSID = "xxxx"; // code replace by xxxx (security issue)
const mx_char_t *Password = "xxxx"; // code replace by xxxx (security issue)


Mode currentMode = KEEPALIVE; // Choose if you want to send a keepalive, an autotest or nothing



int8_t app_main( void) {
    /* Initialize bsp resources */
    bsp_init();

    MX_WIFI_STATUS_T a;  // declare a variable to stock the state of the module

    Wifi_IO_Init(); // initialization of the wifi module secondaries pins (not the spi pins)


    // initialization of the module request
    {
		probe probe_object; // ~ pnetif to feed hw_start

		int32_t ret_hw_start = hw_start(&probe_object); // initialization of the SPI and the module

			if (ret_hw_start < 0){
				return POST_REQUEST_RECEIVING_FAILED;
			}
    }


    a = MX_WIFI_Scan(wifi_obj_get(), 0, NULL,0); // scan is mandatory before connecting request to connect correctly

    (wifi_obj_get())->NetSettings.DHCP_IsEnabled=1; // switch on the DHCP to get an IP address

    a = MX_WIFI_Connect(wifi_obj_get(), SSID, Password, MX_WIFI_SEC_WPA_AES);

    HAL_Delay(5000); // waiting for 5s to get connected correctly

		if (a != MX_WIFI_STATUS_OK){
			return WIFI_CONNECTION_FAILED ;
		}

    uint8_t module_IP[4]; // declare a vector to stock the IP address of the module

    a = MX_WIFI_GetIPAddress(wifi_obj_get(),&module_IP[0],MC_STATION);

		if (a != MX_WIFI_STATUS_OK){
			return IP_REQUEST_FAILED;
		}

    int32_t sock_fd = MX_WIFI_Socket_create(wifi_obj_get(), MX_AF_INET, MX_SOCK_STREAM, NET_IPPROTO_TCP); // create a socket with some parameters to use TCP protocol

		if (sock_fd < 0){
			return SOCKET_CREATION_FAILED ;
		}

	struct mx_sockaddr *server_addr = (struct mx_sockaddr*) malloc(sizeof(struct mx_sockaddr));
	server_addr->sa_family = MX_AF_INET;
	server_addr->sa_len=16; // Number of bytes of the structure
		server_addr->sa_data[0] = 0x1F;          // port of our application : 8080 (in hexadecimal on 2 Bytes)
		server_addr->sa_data[1] = 0x90;
		server_addr->sa_data[2] = 192;
		server_addr->sa_data[3] = 168;           // IP of our server
		server_addr->sa_data[4] = 20;
		server_addr->sa_data[5] = 71;
		server_addr->sa_data[6] = 0;
		server_addr->sa_data[7] = 0;
		server_addr->sa_data[8] = 0;
		server_addr->sa_data[9] = 0;          // following element = 0
		server_addr->sa_data[10] = 0;
		server_addr->sa_data[11] = 0;
		server_addr->sa_data[12] = 0;
		server_addr->sa_data[13] = 0;

	// Connection to the Server
	a = MX_WIFI_Socket_connect(wifi_obj_get(), sock_fd, (const struct mx_sockaddr *)server_addr, (int32_t)sizeof(struct mx_sockaddr_in));

		if (a != MX_WIFI_STATUS_OK){
			return SOCKET_CONNECTION_FAILED;
		}

	if (currentMode == KEEPALIVE) {
		// Post request for KEEPALIVE
		const char* post_request =
				"POST /SDM/public/v1/devices/alive HTTP/1.1\r\n"
				"Host: 192.168.20.71:8080\r\n"
				"Accept: application/json\r\n"
				"Content-Type: application/json; charset=utf-8\r\n"
				"Authorization: Basic xxxx\r\n" // code replace by xxxx (security issue)
				"Content-Length: 267\r\n"
				"\r\n"
				"{"
				"\"serialNumber\":\"99051190\","
				"\"applicationId\":\"2.16.756.5.25.4.6.2.1\","
				"\"deviceId\":\"TEST_PVL_TCI\","
				"\"versionId\":\"51104\","
				"\"hardwareId\":\"0\","
				"\"settingSet\":\"Batman\""
				"}";

		a = MX_WIFI_Socket_send(wifi_obj_get(), sock_fd, (const uint8_t *)post_request, strlen(post_request), 0); // function to send the post request

		// prepare the stuff for the receive function
		static unsigned char recv_buffer[100]; // create a buffer to stock the response
		memset((void*)recv_buffer, 0, sizeof(recv_buffer)); // Clear the buffer

		int32_t nb = MX_WIFI_Socket_recv(wifi_obj_get(), sock_fd, (uint8_t *)recv_buffer, 100, 0); // function to receive the response from the server

			if (nb < 0){
				return POST_REQUEST_RECEIVING_FAILED;
			}
	}

	else if (currentMode == AUTOTEST) {
		// Post request for AUTOTEST
		const char* post_request =
				"POST /SDM/public/v1/devices/test HTTP/1.1\r\n"
				"Host: 192.168.20.71:8080\r\n"
				"Accept: application/json\r\n"
				"Content-Type: application/json; charset=utf-8\r\n"
				"Authorization: Basic xxxx\r\n" // code replace by xxxx (security issue)

				"Content-Length: 404\r\n"
				"\r\n"
				"{"
				"\"serialNumber\":\"99051190\","
				"\"applicationId\":\"2.16.756.5.25.4.6.2.1\","
				"\"level\":\"INFO\","
				"\"created\":\"2024-10-09T15:14:57.789+01:00\","
                "\"content\": \"<!DOCTYPE html><html><body><div id=\\\"date-utc1\\\"></div><script>function dateUTC1() {const now = new Date();const utc1 = new Date(now.getTime() );document.getElementById(\\\"date-utc1\\\").innerText = utc1;}dateUTC1();</script></body></html>\"}' "
				"}";

		a = MX_WIFI_Socket_send(wifi_obj_get(), sock_fd, (const uint8_t *)post_request, strlen(post_request), 0); // function to send the post request

		// prepare the stuff for the receive function
		static unsigned char recv_buffer[500]; // create a buffer to stock the response
		memset((void*)recv_buffer, 0, sizeof(recv_buffer)); // Clear the buffer

		int32_t nb = MX_WIFI_Socket_recv(wifi_obj_get(), sock_fd, (uint8_t *)recv_buffer, 100, 0); // function to receive the response from the server

			if (nb < 0){
				return POST_REQUEST_RECEIVING_FAILED;
			}
	}


// a 2 nd send request always fail, maybe the socket is broke because of the previous fail request

   a = MX_WIFI_Socket_close(wifi_obj_get(), sock_fd); // Ajoute la fonction pour fermer le socket

	if (a != MX_WIFI_STATUS_OK){
		return SOCKET_CLOSING_FAILED;
	}


    return APP_OK;
}
