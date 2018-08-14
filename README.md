# nug-iot-event

<title> IOT NUG </title>
<html><body bgcolor=#fefefe>
<font face = arial color=#1199ff>
<h1> Getting setup for the IOT NUG! </h1>
<h2> Install Serial Driver for Wemos boards </h2>
<ul>
<li> <a href="https://wiki.wemos.cc/downloads">https://wiki.wemos.cc/downloads</a></li>
<li> Reboot </li>
</ul>

<h2> Install Arduino IDE </h2>
<ul>
<li>Download: <a href="https://www.arduino.cc/en/Main/Software?">https://www.arduino.cc/en/Main/Software?</a></li>
<li>Add this Board Manager URL via Preferences: http://arduino.esp8266.com/stable/package_esp8266com_index.json</li>
<li>Go to Tools->Board->Board Manager, find "ESP8266 Community" and install </li>
<li>Go to Sketch->Include Library->Manage Libraries-> find "Adafruit Unified Sensor","DHT sensor library" and "PubSubClient"</li>
</ul>

<h2> Configure Arduino IDE </h2>
<li>Tools->Boards->Wemos D1R2 & mini</li>
<li>Tools->Port-> TBD</li>
<li>Grab this sketch: <a href="NUG_IOT_Demo.ino">NUG_IOT_Demo.ino</a></li>
</ul>

<h2> Get onto the UI</h2>
<a href="http://iot.nug.team:1880/ui/#/1">NodeRED Dashboard</a>
<br>
<a href="http://iot.nug.team:3000">Grafana Dashboard</a>
</font>
</body>
</html>
