# esp8266-weather

This is a project that I started when I discoved the little ESP8266 wifi capable arduino boards.  You could probably just do away with the DHT-22 humidity sensor if you want to save 10 bux.  I used it because at the time the BMP-180 didn't have a very accurate pressure sensor, that has changed with the newer BME280 chips. 

I've also included a 3d print case design for sliding in a pcb board with the components on it.  I originally glued this onto a solar panel/usb battery.  But the solar panel sucked, so it was basically just a battery. I plan to build my own panel and charger system eventually and add that to the project.

You can download the free Splunk version which limits the amount of data you collect each day, but it's plenty for doing weather data. Download this at splunk.com.  Or you can just use a Wunderground.com account to make your own weather station.  Enter in your account info into the .ino file along with your wifi SSID and Password.

The .ino code switches the ESP into low power mode and wakes up every five minutes to take readings.  Tweak the sleep number to adjust the interval.

I'll keep trying to improve the instructions and other stuff here as I get time.

Added another Arduino code file for saving to a local Python Flask app.

Enjoy,

-Keith
