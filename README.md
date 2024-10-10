# sending_datas_over_Wi-Fi_with_b-u585i-iot02a

This project aims to send datas on a server using Wi-Fi with the STMicroelectronics evaluation kit "b-u585i-iot02a". To do it this project simply sends a post request to the server.

To execute the project correctly you need to : 
  - Add the parameter of the network that you want to connect (at the top of the file app.c)
  - Fill in the "Authorization: Basic" for each post request to get identify by the server (in each post request written in app.c)
  - Enter the port and the IP adress of your server (just before "MX_WIFI_Socket_connect" function in app.c)

You can choose between 2 differents post request :
  - A keepalive request : contains few informations about the device directly displayed on the server
  - An autotest request : contains an HTML code that you can vizualise

  For an Autotest, the date of your autotest is the date filled in "created" (in the Post request). Take care to edit it for each request that you send (edit also correctly the UTC) to find it easier on the server.
  By default, the content of the post request is the send hour of your request (obtain automatically with javascript : different of the value in "created")
  
