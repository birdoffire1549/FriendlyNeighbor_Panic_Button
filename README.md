# FriendlyNeighbor Panic Button

## Background
This project was inspired by the fact that one of my next-door neighbors is a friendly old lady. Usually she is taken care of by her Daughter and Son-in-law which typically live with her. Well, as of lately her care-takers have been spending a lot of time out of the state and sometimes ask my wife and I if we can call her mom and check on her once a day while they are out of town. Generally this isn't a problem but I got the idea that what if the lady could have a device that would act as a kind of panic button. That way if there was an emergency and she couldn't call for help, perhaps she could activate the Panic Button Device and it could notify some selected people of her need for help or assistance. That is what this project is.

## Getting Started 
This project is a WiFi enabled Panic Button device.

Once the device is built and then this firmware is flashed to the device, it will bootup into Configuration Mode. In configuration mode, the device broadcasts a secured wireless network that one can connect to for configuring the device. 

Typically the ssid that is broadcast will look something like this: `Panic_Button_A3224E`. The ssid for this device will start with the text `Panic_Button_` and then a 6 digit semi-unique device id will follow. 

To connect to the device's WiFi use the password: `P@ssw0rd123`

Then pull up the device's configuration page in a web-browser using the address: `https://192.168.1.1/admin`.

This will cause an Authentication Popup to display where you can log-in using the username `admin` and the password `P@ssw0rd123`, to authenticate. After that you will see a settings page where the only input is a text-field which a JSON Document in it. You may then change the settings in the JSON Document and then click the 'submit' button to cause the new settings to take affect. 

If the settings are good then the devie will simply restart and you will see `Connecting...` on the device's display. Once the device connects to the desired Network the display will read, `System Ready.`. Once that is desplayed on the device, then it is ready for use.

## Device Details
### Post Firmware Install
After the device's firmware is installed and the device is powered up for the first time, the screen will read, `Setup Required!`, and the red attention LED will be solidly lit indicating the device needs attention. At this point one must connect to the device's WiFi to configure the device.

### Device Configuration
#### WiFi Connection in Configuration Mode
When the device is displaying the message `Setup Required!`, one can look for a wireless network with an SSID simmilar to `Panic_Button_A3224E`. The SSID is made of the static text 'Panic_Button_' followed by a semi unique device id which should be different for every set of hardware that the firmware is flashed onto. When connecting to the SSID use the Password: `P@ssw0rd123`. This should allow you to connect to the device's wifi.

#### Setup via Admin Page
After connected to the device's WiFi the device can be configured using its Admin Page. To connect to the admin page use the following address in a web-browser: `https://192.168.1.1/admin`. This will cause an authentication popup, which you can use the user `admin` and password `P@ssw0rd123` to connect to.

Once connected you will see the configuration for the device displayed as a JSON Document. Simply edit the contents of the JSON Document then click 'Submit' at the bottom of the page. This will cause the device to be configured.

Samply Factory Settings JSON:
```
{
    "ssid": "SET_ME",
    "pwd": "SET_ME",
    "smtp_host": "SET_ME",
    "smtp_port": 465,
    "smtp_user": "SET_ME",
    "smtp_pwd": "SET_ME",
    "from_name": "FriendlyNeighbor PanicButton",
    "from_email": "no-reply@panic-button.com",
    "owner": "SET_ME",
    "message": "Please send help ASAP!",
    "panic_level": 5,
    "recipients": "test@email.com"
}
```

##### ssid
This is the SSID of the network the device should connect to for access to the Internet.
##### pwd
This is the Password used to connect to the wireless network.
##### smtp_host
This is the hostname of the smtp server you want to connect to. Example: `smtp.gmail.com`
##### smtp_port
This is the port the smtp service is answering on.
##### smtp_user
This is the user id that is needed to authenticate with your smtp server.
##### smtp_pwd
This is the password that is needed to authenticate with your smtp server.
##### from_name
This is the name that the email message will appear to be from.
##### from_email
This is the email address that the email message will appear to be from.
##### owner
This is the name of the person who is the intended user of the Panic Button Device.
##### message
This is the message that will be sent when the Panic Button is activated.
##### panic_level
This is the severity of the message. See below for more information.
| Level | Description | 
| ---- | ---- |
| 1 | TEST | 
| 2 | INFORMATIONAL | 
| 3 | WARNING |
| 4 | CRITICAL |
| 5 | EMERGENCY |
##### recipients
This is a list of recipients where each address is to separated by a semicolin ';'.

### Basic Operation
When the device is ready to use the display should read `System Ready.`. This means the system is ready for use and all is expected to function properly. 

#### Activate Panic Mode
To activate the Panic Button Device, press and hold the Panic Button. The device will count backwards from 3 and then activate Panic Mode if the button remains pressed during the countdown. This is to ensure the device isn't triggered by the button accedently being pressed breifly or bumped. When the device is in Panic Mode the dispaly will read `Alerts Sent!`, and the attention LED will flash.

#### Cancel Panic Mode
After Panic Mode has been activated it can be canceled by holding down the cancel button while the device counts backwards from 3 on the display. Canceling Panic Mode sends out new messages stating that the Panic Mode was canceled. If all goes well the device's display should eventually return to a message of `System Ready!`, and the attention LED should be off.

#### Potential Issues
When the device is in Ready Mode, it will once a minute check-in with the SMTP server to ensure it has access to the SMTP server. If at anytime it doesn't have accesss to the server the message `Internet Down?` will be displayed with the attention LED on. The device will continue to check for access to be restored once a minute and if it is restored it will go back into Ready Mode.

Another issue it can have is that some or all of the recipients might not have messages go through when activating Panic Mode. If at least some of the recipients go through then the screen will show `Partial Send!` and the led will flash, this indicates that the device is in Panic Mode and at least some of the intended recipients were notified. If none of the recipients were able to be sent messages then the device will show `Send Error!!!`, and the attention LED will remain solidly lit.

If power to the device goes out while in Panic Mode, when power is restored the device will bootup in Panic Mode so that it can be canceled if desired.

## Device Limitations
The ESP8266 is actually a quite limited chip in terms of its available memory, and the RAM in particular. Originally I had code written which would have provided a rich settings page but due to the memory limitations I had to scale that way back to a single field which contained a JSON Document for the configurations. The HTML was just too big otherwise and pages didn't show reliably.

Also due to memory limits, I decided to minimize the number of configurable settings the device would have. This lead to static default passwords for connecting to and accessing the device. Since these passwords don't change they aren't the best security but they do help some, because the device's WiFi and WebServer shutdown once the device is actually configured. Therefore a configured device cannot have its settings modified or viewed by way of being hacked, so the weak initial password use isn't a super huge deal. If one wants to reconfigure a device that was already configured they must perform a factory reset on the device, which wipes and defaults all of the device's configuration settings.

## Device Schematic

![Schematic of the FriendlyNeighbor Panic Button Device](https://github.com/birdoffire1549/FriendlyNeighbor_Panic_Button/blob/9410a65af17257e3acdc20b8ad1a5bfb0b4692c5/FriendlyNeighbor%20Panic%20Button_schem.png)
