#include <WiFiS3.h>
#include <Arduino_JSON.h>
#include <ArduinoHttpClient.h>
#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 150
CRGB leds[NUM_LEDS];

//wifi info
const char* ssid = "E3000";
const char* password = "savannah03";

//location & api key
String openWeatherMapApiKey = "647979d02fba11b09dbbf17f34b2c5b8";
//String city = "San%20Francisco";
String city = "New%20Haven%20County";
//String city = "Dubai";

String countryCode = "";

//Timer to control the interval between API requests
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

WiFiClient wifiClient;
HttpClient httpClient(wifiClient, "api.openweathermap.org", 80);

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);

  Serial.begin(9600);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 10 seconds (timerDelay variable).");

}

void loop() {
  //Send an HTTP GET request every timerDelay interval
  if ((millis() - lastTime) > timerDelay) {

    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=imperial";

      String jsonBuffer = httpGETRequest(serverPath.c_str());

      JSONVar data = JSON.parse(jsonBuffer);

      //Check if JSON parsing was successful
      if (JSON.typeof(data) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      //Extract variables
      float F_temperature = (double)data["main"]["temp"];
      float F_humidity = (double)data["main"]["humidity"];
      float F_wind = (double)data["wind"]["speed"];
      int cloudPercentage = data["clouds"]["all"];
      float visability = 0.0;
      if (data.hasOwnProperty("visibility")) {
        visability = (double)data["visibility"];
      }
      float rain_1hour = 0.0;
      if (data.hasOwnProperty("rain") && data["rain"].hasOwnProperty("1h")) {
        rain_1hour = (double)data["rain"]["1h"];
      }
      const char* weather_type = data["weather"][0]["main"]; 


      //convert floats to ints for led indexing
      int temperature = static_cast<int>(F_temperature);
      int humidity = static_cast<int>(F_humidity);
      int wind = static_cast<int>(F_wind);

      //print out extracted variables
      Serial.println(temperature);
      Serial.println(humidity);
      Serial.println(wind);
      Serial.println(cloudPercentage);
      Serial.println(rain_1hour);
      Serial.println(visability);

      //start with every led off
      fill_solid(leds, NUM_LEDS, CRGB::Black);

      //display values on board
      temp_display(temperature);
      wind_display(wind);
      Visability_display(visability);
      cloud_display(cloudPercentage);
      rain_display(rain_1hour, weather_type);

    } 
    
    else {
      Serial.println("WiFi Disconnected");
    }

    lastTime = millis();
  }
}


String httpGETRequest(const char* serverPath) {
  String payload = "{}";

  //Send GET request
  httpClient.get(serverPath);

  int statusCode = httpClient.responseStatusCode();
  if (statusCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(statusCode);
    payload = httpClient.responseBody();
  } else {
    Serial.print("Error code: ");
    Serial.println(statusCode);
  }

  httpClient.stop();
  return payload;
}


void cloud_display(int cloudPercentage){

  Serial.print("Cloud percentage: ");
  Serial.println(cloudPercentage);

  //turn on sun LEDs
  for(int i=0; i<7; i++){
    leds[i] = CRGB(100,150,0);
  }

  if(cloudPercentage > 10 && cloudPercentage <= 30){
    for(int i=7; i<19; i++){
      leds[i] = CRGB(10,10,40);
    }
  }

  else if(cloudPercentage > 30 && cloudPercentage <= 60){
    for(int i=7; i<35; i++){
      leds[i] = CRGB(10,10,40);
    }
  }

  else if(cloudPercentage > 60 && cloudPercentage <= 90){
    for(int i=7; i<35; i++){
      leds[i] = CRGB(10,10,40);
    }
    for(int i=53; i<64; i++){
      leds[i] = CRGB(10,10,40);
    }
  }

  else if(cloudPercentage > 90){
    for(int i=7; i<35; i++){
      leds[i] = CRGB(10,10,40);
    }
    for(int i=53; i<64; i++){
      leds[i] = CRGB(10,10,40);
    }
    for(int i=72; i<83; i++){
      leds[i] = CRGB(10,10,40);
    }
    //disable sun
    for(int i=0; i<7; i++){
      leds[i] = CRGB(0,0,0);
    }
  }

  FastLED.show();
}


void rain_display(float rain_1hr, const char* weather_type){
  Serial.print("rain amt: ");
  Serial.println(rain_1hr);
  if(rain_1hr == 0.0){
    //dont light up anything
    
  }

  else if(rain_1hr < 0.3){
    //light up the smaller clouds rain
    for(int i=0; i<4; i++){

      //small clouds rain
      leds[64+i] = CRGB(10,10,15);
      leds[71-i] = CRGB(10,10,15);
  
      delay(1000);
      FastLED.show();
    }
  }

  else if(rain_1hr >= 0.3){

    for(int i=0; i<6; i++){
      //big clouds rain
      leds[35+i] = CRGB(10,10,15);
      leds[46-i] = CRGB(10,10,15);
      leds[47+i] = CRGB(10,10,15);

      //small clouds rain
      if(i < 4){
        leds[64+i] = CRGB(10,10,15);
        leds[71-i] = CRGB(10,10,15);
      }
      delay(1000);
      FastLED.show();
    }

  }

  if (strcmp(weather_type, "Thunderstorm") == 0) {
    Serial.println("Thunderstorms!");
    
    delay(1000);
    for(int i=64; i < 72; i++){
      leds[i] = CRGB(0,0,0);
    }
    FastLED.show();
    delay(1000);

    leds[64] = CRGB(100,100,0);
    FastLED.show();
    delay(1000);

    leds[70] = CRGB(100,100,0);
    FastLED.show();
    delay(1000);

    leds[66] = CRGB(100,100,0);
    FastLED.show();
    delay(1000);

    leds[68] = CRGB(100,100,0);
    FastLED.show();
  }
}


void temp_display(int temperature){

  if(temperature < 0){
    leds[93] = CRGB(0,25,0);
  }
  else if(temperature > 119){
    for(int i=93; i<=113; i++){
      leds[i] = CRGB(0,25,0);
    }
  }
  else{
    //Ones place
    for (int i = 93; i >= 93 - (temperature % 10); i--) {
      leds[i] = CRGB(0,25,0);
    }

    //Tens place
    int j=93;
    while(j <= (temperature/10) + 93){
      leds[j] = CRGB(0,25,0);
      j++;
    }
  }

  FastLED.show();
}


void wind_display(int wind){
  Serial.print("Wind value: ");
  Serial.println(wind);
  if (wind < 0) wind = 0;
  if (wind > 26) wind = 26;

  for(int i=118; i >= (118 - (wind/2)); i--){
      leds[i] = CRGB(0,25,0);
  }
  FastLED.show();
}


void Visability_display(float vis) {
  //Convert vis from meters to miles
  vis = vis * 0.000621371;
  Serial.print("new Visability val: ");
  Serial.println(vis);

  //handle visability less than 1 mile
  if (vis <= 1.0) {
    // Each LED in the range 119–122 represents 1/4 mile
    int ledsToLight = vis * 4; // Convert miles to quarter-mile LEDs
    for (int i = 119; i < (119 + ledsToLight); i++) {
      leds[i] = CRGB(0,25,0); // Set color for visibility
    }
  } 
  else {
    //light up the first 4 leds
    for (int i = 119; i <= 122; i++) {
      leds[i] = CRGB(0,25,0); // Set color for first mile
    }

    //light up correct leds representing 2 - 10
    int ledsToLight = min(static_cast<int>(vis) - 1, 9); // Limit to max 9 additional LEDs
    for (int i = 123; i <= (123 + ledsToLight); i++) {
      leds[i] = CRGB(0,25,0); // Set color for additional miles
    }
  }

  FastLED.show();
}

