#define SSID        "ZYH" //改为你的热点名称, 不要有中文
#define PASSWORD    "Zyh@20000812"//改为你的WiFi密码Wi-Fi密码
#define DEVICEID    "577208800" //OneNet上的设备ID
String apiKey = "DQ4QOInXKfFaF=TPku5=tuCeFj0=";//与你的设备绑定的APIKey

/***/
#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   500             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      500             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include <ESP8266.h>
#include <I2Cdev.h>                                //调用库  
/*******温湿度*******/
#include <Microduino_SHT2x.h>
/*******光照*******/
#define  sensorPin_1  A0
#define IDLE_TIMEOUT_MS  100000000    // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
char buf[10];

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();
//3,传感器值的设置 
float sensor_sta;                    //传感器温度、湿度、光照   
char  sensor_sta_c[7] ;    //换成char数组传输

#define INTERVAL_OLED 1000
String mCottenData;
String jsonToSend;

int Sensor = 8; //接收传感器的信号

#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(&EspSerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

Tem_Hum_S2 TempMonitor;

void setup(void)     //初始化函数  
{       
  //初始化串口波特率  
    Wire.begin();
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.print(F("setup begin\r\n"));
    delay(50);
   pinMode(11, OUTPUT);  //发红色光
   pinMode(10, OUTPUT);  //发蓝色光
   pinMode(9, OUTPUT);   //发绿色光
   pinMode(6, OUTPUT);//蜂鸣器响
   pinMode(Sensor, INPUT); // 设置为输入
  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("to station + softap ok\r\n"));
  } else {
    Serial.print(F("to station + softap err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));

    Serial.print(F("IP:"));
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }

  Serial.print(F("setup end\r\n"));
    
  
}
void loop(void)     //循环函数  
{     int val = digitalRead(Sensor); // 读取引脚电平
                      
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {                         
                          getSensorData();
                           if (val==1)  // 若检测到有人，则val值为1
                     { 
                            digitalWrite(11,HIGH); digitalWrite(10,LOW); digitalWrite(9,LOW);digitalWrite(6,HIGH);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,HIGH); digitalWrite(9,LOW);digitalWrite(6,LOW);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,LOW); digitalWrite(9,HIGH);digitalWrite(6,HIGH);
                            delay(1000); 
                            digitalWrite(11,HIGH); digitalWrite(10,LOW); digitalWrite(9,LOW);digitalWrite(6,HIGH);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,HIGH); digitalWrite(9,LOW);digitalWrite(6,LOW);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,LOW); digitalWrite(9,HIGH);digitalWrite(6,HIGH);
                            delay(1000); 
                            digitalWrite(11,HIGH); digitalWrite(10,LOW); digitalWrite(9,LOW);digitalWrite(6,HIGH);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,HIGH); digitalWrite(9,LOW);digitalWrite(6,LOW);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,LOW); digitalWrite(9,HIGH);digitalWrite(6,HIGH);
                            delay(1000); 
                            digitalWrite(11,HIGH); digitalWrite(10,LOW); digitalWrite(9,LOW);digitalWrite(6,HIGH);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,HIGH); digitalWrite(9,LOW);digitalWrite(6,LOW);
                            delay(1000);                    
                            digitalWrite(11,LOW); digitalWrite(10,LOW); digitalWrite(9,HIGH);digitalWrite(6,HIGH);
                            delay(1000); 
                               }

                            else

                            {
                             digitalWrite(11,LOW); digitalWrite(10,LOW); digitalWrite(9,LOW);digitalWrite(6,LOW);}
                             delay(50);              //读串口中的传感器数据
    sensor_time = millis();
  }  

  
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {     if (val==1)           
   { updateSensorData(); }   //将数据上传到服务器的函数
   else
   {UpdateSensorData();}
    net_time1 = millis();
  }
  
}
void getSensorData(){  
    sensor_sta = digitalRead(Sensor);    
    dtostrf(sensor_sta, 2, 1, sensor_sta_c);
}
void UpdateSensorData(){ if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"State\":";
    dtostrf(sensor_sta,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
  }
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"State\":";
    dtostrf(sensor_sta,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
}

