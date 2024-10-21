# Sending_datas_over_Wi-Fi_with_b-u585i-iot02a

## Project Objective:

This project aims to transmit data to a server over Wi-Fi using the STMicroelectronics evaluation kit "b-u585i-iot02a" To achieve this, the project sends a POST request to the server.

## Project importation:

To import this project on your own b-u585i-iot02a, you need to:

**1)** *Download this project*

**2)** *Open it with STM32CubeIDE software (use File -> Import-> Existing projects into workspace to import the project in your workspace)*

**3)** *Add the parameter of the network that you want to connect (at the top of the file app.c)*

**4)** *Fill in the "Authorization: Basic" for each post request to get identify by the server (in each post request written in app.c)*

**5)** *Enter the port and the IP adress of your server (just before "MX_WIFI_Socket_connect" function in app.c)*

**6)** *Run the project*

## Project Workflow:

![image](https://github.com/user-attachments/assets/9d0815da-1bf4-47a0-ad2b-ef7d1ef5dfc9)



## Prerequisites for Execution:

Network Parameters: Specify the desired network parameters at the top of the app.c file.
Authorization: Provide the "Authorization: Basic" header for each POST request to authenticate with the server (in app.c).
Server Address: Enter the correct port and IP address of your server just before the MX_WIFI_Socket_connect function in app.c.

## POST Request Options:

Keepalive Request: Contains minimal device information that is directly displayed on the server.

Autotest Request: Includes an HTML code for visualization.

## Autotest Notes:

The "created" field in the POST request specifies the autotest date. Ensure it is updated for each request and the UTC is correctly adjusted for easier server identification.
By default, the POST request content is the send time of the request (obtained automatically with JavaScript), which differs from the "created" value.
