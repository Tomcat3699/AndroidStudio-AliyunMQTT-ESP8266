// 引入wifi 模块，并实例化，不同的芯片这里不同
#include <ESP8266WiFi.h>
static WiFiClient espClient;  

//引入DS18b20温度传感器
#include <OneWire.h>
#include <DallasTemperature.h>

// 引入阿里云 IoT SDK
#include <AliyunIoTSDK.h>
#include <ArduinoJson.h>

//设置产品和设备的信息，从阿里云设备信息里查看,自行修改
#define PRODUCT_KEY "k0xxxxxxxPJ"
#define DEVICE_NAME "esp8266_dev"
#define DEVICE_SECRET "2a7xxxxx3daxxx9fxxxxx60xxxxxxx99"
#define REGION_ID "cn-shanghai"//地域，华东2（上海）

//esp8266设备连接的WiFi用户名和密码
#define WIFI_SSID "xxxxxxxxx"
#define WIFI_PASSWD "xxxxxxxxx"

//Arduino根目录下的libraries里的头文件
#include <ArduinoJson.h>  

//步进电机
#include<Servo.h>
Servo myservo;

//#define ONE_WIRE_BUS 2    //2对应开发板上的D9
#define ONE_WIRE_BUS 13     //13对应开发板上的D7

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// put your setup code here, to run once:
void setup() {

  //初始化串口
  Serial.begin(115200);//初始化串口波特率是115200

  myservo.attach(2,500,2500);  //2对应开发板上的D9

  myservo.write(90);  //舵机初始给个角度90，在0-90,90-180之间正反转即可
  
  //ESP8266设备连接WiFi
  wifiInit(WIFI_SSID, WIFI_PASSWD);

  sensors.begin();

  // 初始化 iot，需传入 wifi 的 client，和设备产品信息
  AliyunIoTSDK::begin(espClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);
  
  //绑定属性回调，云服务下发的数据会进入回调，用于监听特定数据的下发
  //（参数1：在产品-功能定义中，led为某属性中的标识符）
  AliyunIoTSDK::bindData("led",powerCallback);
  //这里可以自行扩展，写法类似
  
}

// put your main code here, to run repeatedly:                                             
unsigned long lastMsMain = 0;
const long interval = 10000;   //发布消息频率                                  
void loop() {
  AliyunIoTSDK::loop();  //在主程序loop 中调用，检查连接和定时发送信息
  
  sensors.requestTemperatures(); 
  
  if (millis() - lastMsMain >= interval)
    {
      lastMsMain = millis(); 

      
      //发送一个数据到云平台，参数1是在设备产品中定义的物联网模型的 id，即在产品-功能定义里某属性的标识符
      AliyunIoTSDK::send("temp", sensors.getTempCByIndex(0));  
      //这里可以自行扩展，写法类似

    }
}


//ESP8266设备连接WiFi函数
void wifiInit(const char *ssid, const char *passphrase)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passphrase);
    WiFi.setAutoConnect (true);
    WiFi.setAutoReconnect (true);
    while (WiFi.status() != WL_CONNECTED)//如果没有连接上WiFi，就重复尝试，直到连接成功
    {
        delay(2000);
        Serial.println("WiFi not Connect");
    }
    Serial.println("Connected to AP");
}



void powerCallback(JsonVariant p)
{
    int PowerSwitch = p["led"]["value"];
    if (PowerSwitch == 1)
    {
        //看是否会响应云平台的内容
        Serial.println("Switch=1  ");
        rotateClockwise(35);  //舵机旋转角度，自行修改以适配
        AliyunIoTSDK::send("led", 1);
    }
    else
    {
        //是否会响应平台的内容
        Serial.println("Switch=0");      
        rotateCounterclockwise(28);  //舵机旋转角度，自行修改以适配
        AliyunIoTSDK::send("led", 0);
    }  
}

void rotateClockwise(int degrees) {
  int currentPos = myservo.read();
  int targetPos = currentPos + degrees;

  if (targetPos > 180) {
    targetPos = 180;
  }

  myservo.write(targetPos);
  delay(1500);
  myservo.write(90);
}

void rotateCounterclockwise(int degrees) {
  int currentPos = myservo.read();
  int targetPos = currentPos - degrees;

  if (targetPos < 0) {
    targetPos = 0;
  }

  myservo.write(targetPos);
    delay(1500);
  myservo.write(90);
}

