#include <Arduino.h>

// #include <ETH.h>
#include <WiFiUdp.h>
#include "SPIFFS.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "master_ip";
const char* PARAM_INPUT_2 = "listening_port";
const char* PARAM_INPUT_3 = "writing_port";
const char* PARAM_INPUT_4 = "reset";

// STA账号，密码
char* ssid = "1111C_2.4G";
char* password = "ssrgroup";
// 初始化wifi为AP模式并固定ip 192.168.5.1
void init_AP(void);

//Variables to save values from HTML form
String ip_n;
String listen_port_n;
String write_port_n;
String random_n;

// File paths to save input values permanently
const char* ip_Path = "/ip.txt"; // ETH的目标IP
const char* listen_port_Path = "/listen_port.txt"; // ETH目标IP的监听端口
const char* write_port_Path = "/write_port.txt"; // ETH目标IP的被写端口
const char* randomPath = "/random.txt"; // 01-无需random 00-需要random

// 继电器四个端口 相关
#define myBeep  12
#define myLed_R 32
#define myLed_G 33
#define myLed_B 14
static bool status0 = false, status1 = false, status2 = false, status3 = false;

// 板载系统指示灯 相关
static bool led_status = true;
#define Led_R 2
// #define Led_G 5
// #define Led_B 16
#define SerialDebug true  // set to true to get Serial output for debugging

// UDP 相关
WiFiUDP udp; // 用于UDP监听
WiFiUDP udp_write; // 用于UDP写
unsigned int localUdpPort = 24539;  //  port to send
unsigned int listenPort = 23030; //  port to listen
uint8_t ip_1, ip_2, ip_3, ip_4; 
IPAddress Listen_IP1(10, 168, 1, 127); // IP to send
IPAddress Listen_IP2(192, 168, 5, 3);
IPAddress Listen_IP3(192, 168, 5, 4);
#define BUFFER_LENGTH 64
static bool listen_command = false; // listen_command == true 代表 接收到数据但未处理  listen_command == false 代表未接收到数据或者接收到已处理
char incomingPacket[BUFFER_LENGTH]; 
int len = 0;

// Initialize SPIFFS
void initSPIFFS(void);
// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path);
// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message);
// 分割IP为 ip_1 ~ ip_4
void fenge(String zifuchuan,String fengefu);

// // ETH 连接状态判断
// void WiFiEvent(WiFiEvent_t event);

// ETH 联网测试
void testClient(const char * host, uint16_t port);

// UDP 发送函数（仅仅向192.168.5.2~4发送串口数据）
void UDP_send(WiFiUDP Udp, IPAddress ip, uint16_t port, char* words);

// UDP 透传发送数据到三个指定的目标IP和端口
void UDP_send_string_to_target(char* words);

static bool eth_connected = false;

void decode_command(); // 解析得到每个状态 status0, status1, status2, status3

// 任务相关
void LED_BEEP_Task(void *ptParam);//LED_BEEP_Task 任务主体(控制继电器动作)
void UDP_Listen_Task(void *ptParam);//UDP_Listen_Task 任务主体(监听服务器指令)
void System_led_blink(void *ptParam);//System_led_blink 任务主体(兼容debug功能)

// freertos任务创建
void freertos_task_create(void)
{
    // xTaskCreatePinnedToCore(LED_BEEP_Task, "LED_BEEP_Task", 1024 * 8, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(UDP_Listen_Task, "UDP_Listen_Task", 1024 * 8, NULL, 2, NULL, 1);
    vTaskDelay(1000); //让UDP数据监听程序提前先运行一秒获取第一笔数据
    xTaskCreatePinnedToCore(System_led_blink, "System_led_blink", 1024 * 8, NULL, 2, NULL, 1);
    Serial.println("System has created 3 tasks[LED_BEEP_Task-10Hz, UDP_Listen_Task-200Hz, System_led_blink-1Hz]");
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) { 
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

  pinMode(Led_R, OUTPUT);
  digitalWrite(Led_R, HIGH); // start with led off
  // pinMode(Led_G, OUTPUT);
  // digitalWrite(Led_G, HIGH); // start with led off
  // pinMode(Led_B, OUTPUT);
  // digitalWrite(Led_B, HIGH); // start with led off 

  delay(1000);

  // spiffs 初始化
  initSPIFFS();

  // 加载参数
  ip_n = readFile(SPIFFS, ip_Path);
  listen_port_n = readFile(SPIFFS, listen_port_Path);
  write_port_n = readFile(SPIFFS, write_port_Path);
  Serial.println(ip_n);
  Serial.println(listen_port_n);
  Serial.println(write_port_n);
  fenge(ip_n, ".");

  localUdpPort = write_port_n.toInt();  //  port to send
  listenPort = listen_port_n.toInt();  //  port to listen

  // // ETH & udp listen on
  // WiFi.onEvent(WiFiEvent);
  // ETH.begin();
  

  // web server 用于修改IP，监听端口，状态回传端口号
  // 初始化 Wifi-AP
  // init_AP();
  udp.begin(listenPort);

  // // server setting
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //     request->send(SPIFFS, "/wifimanager.html", "text/html");
  // });

  // server.serveStatic("/", SPIFFS, "/");

  // server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
  //     int params = request->params();
  //     Serial.print("We have number of params is: ");
  //     Serial.println(params);
  //     for(int i=0;i<params;i++){
  //     AsyncWebParameter* p = request->getParam(i);
  //     if(p->isPost()){
  //         // HTTP POST ip_n value
  //         if (p->name() == PARAM_INPUT_1) {
  //         ip_n = p->value().c_str();
  //         Serial.print("Target IP set to: ");
  //         Serial.println(ip_n);
  //         // Write file to save value
  //         writeFile(SPIFFS, ip_Path, ip_n.c_str());
  //         }
  //         // HTTP POST listen_port_n value
  //         if (p->name() == PARAM_INPUT_2) {
  //         listen_port_n = p->value().c_str();
  //         Serial.print("Listen_port_n set to: ");
  //         Serial.println(listen_port_n);
  //         // Write file to save value
  //         writeFile(SPIFFS, listen_port_Path, listen_port_n.c_str());
  //         }
  //         // HTTP POST write_port_n value
  //         if (p->name() == PARAM_INPUT_3) {
  //         write_port_n = p->value().c_str();
  //         Serial.print("Write_port_n set to: ");
  //         Serial.println(write_port_n);
  //         // Write file to save value
  //         writeFile(SPIFFS, write_port_Path, write_port_n.c_str());
  //         }
  //     }
  //     }
  //     request->send(200, "text/plain", "Done. ESP-ETH system will restart, connect to your ETH with following ip, listen_port and write_port : " + ip_n + " " + listen_port_n + " " +  write_port_n);
  //     ESP.restart();
  // });

  // server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
  // request->send(SPIFFS, "/wifimanager.html", "text/html");
  // });
  // server.begin();

  // Task load
  freertos_task_create();
}


void loop()
{
  // if (eth_connected) {
  //   testClient("baidu.com", 80);
  // }
  // delay(10000);

  // UDP_send_string_to_target("hello world!"); // 需要发的内容可以替换"hello world!"
  // Serial.println("hello world!");
  // delay(500);

  // int packetLength = udp.parsePacket(); 
  // if(packetLength){
  //     int len = udp.read(incomingPacket, BUFFER_LENGTH);
  //     if (len > 0){
  //         incomingPacket[len] = 0;
  //         Serial.printf("%s\n", incomingPacket);
  //     }
  // }
}

// void WiFiEvent(WiFiEvent_t event)
// {
//   switch (event) {
//     case ARDUINO_EVENT_ETH_START:
//       Serial.println("ETH Started");
//       //set eth hostname here
//       ETH.setHostname("esp32-eth-zenan");
//       break;
//     case ARDUINO_EVENT_ETH_CONNECTED:
//       Serial.println("ETH Connected");
//       break;
//     case ARDUINO_EVENT_ETH_GOT_IP:
//       Serial.print("ETH MAC: ");
//       Serial.print(ETH.macAddress());
//       Serial.print(", IPv4: ");
//       Serial.print(ETH.localIP());
//       if (ETH.fullDuplex()) {
//         Serial.print(", FULL_DUPLEX");
//       }
//       Serial.print(", ");
//       Serial.print(ETH.linkSpeed());
//       Serial.println("Mbps");
//       eth_connected = true;
//       break;
//     case ARDUINO_EVENT_ETH_DISCONNECTED:
//       Serial.println("ETH Disconnected");
//       eth_connected = false;
//       break;
//     case ARDUINO_EVENT_ETH_STOP:
//       Serial.println("ETH Stopped");
//       eth_connected = false;
//       break;
//     default:
//       break;
//   }
// }

// void testClient(const char * host, uint16_t port)
// {
//   Serial.print("\nconnecting to ");
//   Serial.println(host);

//   WiFiClient client;
//   if (!client.connect(host, port)) {
//     Serial.println("connection failed");
//     return;
//   }
//   client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
//   while (client.connected() && !client.available());
//   while (client.available()) {
//     Serial.write(client.read());
//   }

//   Serial.println("closing connection\n");
//   client.stop();
// }


// UDP 发送函数
void UDP_send(WiFiUDP Udp, IPAddress ip, uint16_t port, char* words)
{
  Udp.beginPacket(ip, port);
  Udp.printf(words);
  Udp.endPacket();
}

// UDP 透传发送数据到三个指定的目标IP和端口(因为测试时候只有一个设备所以我只打开一个的目标IP)
// 解析字符串数组为指定端口
void UDP_send_string_to_target(char* words)
{
    static IPAddress Target_IP(ip_1, ip_2, ip_3, ip_4); // IP to send

    UDP_send(udp_write, Target_IP, localUdpPort, words);
    // UDP_send(udp, Listen_IP2, localUdpPort, words);
    // UDP_send(udp, Listen_IP3, localUdpPort, words);
}

// 解析得到每个状态 status0, status1, status2, status3
// UDP server 协议案例（如：开开关关）: "AA1100BB"
void decode_command(){
  
  if(len > 0)
  {
    Serial.println("Acquire a solid commmand. Decoding...");
    if(incomingPacket[0]=='A'&&incomingPacket[1]=='A')
    {
      if(incomingPacket[6]=='B'&&incomingPacket[7]=='B')
      {
        char buffer[40];
        sprintf(buffer, "Decoding result is: %c-%c-%c-%c .", incomingPacket[2], incomingPacket[3],incomingPacket[4],incomingPacket[5]);
        Serial.println(buffer);

        if(incomingPacket[2] == '0')
          status0 = false;
        else if(incomingPacket[2] == '1')
          status0 = true;

        if(incomingPacket[3] == '0')
          status1 = false;
        else if(incomingPacket[3] == '1')
          status1 = true;

        if(incomingPacket[4] == '0')
          status2 = false;
        else if(incomingPacket[4] == '1')
          status2 = true;
        
        if(incomingPacket[5] == '0')
          status3 = false;
        else if(incomingPacket[5] == '1')
          status3 = true;

        len = 0;
        incomingPacket[len] = 0;
      }
    }
  }
}


// Initialize SPIFFS
void initSPIFFS(void) {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

// 分割IP为 ip_1 ~ ip_4
void fenge(String zifuchuan,String fengefu)
{
  int weizhi; //找查的位置
  String temps;//临时字符串
  int count = 0;
  do{
      weizhi = zifuchuan.indexOf(fengefu);//找到位置
      if(weizhi != -1)//如果位置不为空
      {
          temps=zifuchuan.substring(0,weizhi);//打印取第一个字符
          zifuchuan = zifuchuan.substring(weizhi+fengefu.length(), zifuchuan.length());
          //分隔后只取后面一段内容 以方便后面找查
      }
      else
      {  //上面实在找不到了就把最后的 一个分割值赋值出来以免遗漏
         if(zifuchuan.length() > 0)
          temps=zifuchuan; 
      }
      Serial.println(temps);//在这里执行分割出来的字符下面不然又清空了
      if(count==0)
       ip_1 = temps.toInt();
      else if(count==1)
       ip_2 = temps.toInt();
      else if(count==2)
       ip_3 = temps.toInt();
      else if(count==3)
       ip_4 = temps.toInt();
      count++;
      temps="";
   }
   while(weizhi >=0 && count<=3);   
}


// 初始化wifi为AP模式并固定ip为 192.168.5.1
void init_AP(void)
{
  // Setting the ESP as an access point
  Serial.println("Setting AP (Access Point)...");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  // 限制最大连接数为5
  if (!WiFi.softAP(ssid, password, 5)) {
    Serial.println("AP Failed");
    return;
  }

  delay(2000);
  // Set static IP
  IPAddress AP_LOCAL_IP(192, 168, 5, 1);
  IPAddress AP_GATEWAY_IP(192, 168, 5, 1);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK)) {
    Serial.println("AP Config Failed");
    return;
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(IP);
}

/******************************  任务调度相关函数  **********************************/
// System has created 3 tasks[LED_BEEP_Task-10Hz, UDP_Listen_Task-200Hz, System_led_blink-1Hz]
//LED_BEEP_Task 任务主体(控制继电器动作)
void LED_BEEP_Task(void *ptParam){  
  // 初始化当前tick时刻
  TickType_t xLastWakeTime = xTaskGetTickCount();
  // 定义单周期的tick数量
  const TickType_t xFrequency = 100; // 间隔 100 ticks = 0.1 seconds = 10Hz
  
  for(;;) //使用for更高效
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    if(listen_command == true)
    {
      decode_command(); // 解析得到每个状态并驱动beep和led status0, status1, status2, status3
      listen_command = false;
      if(status0 == true)
        digitalWrite(myLed_R, LOW); // on
      else if(status0 == false)
        digitalWrite(myLed_R, HIGH); // off
      if(status1 == true)
        digitalWrite(myLed_G, LOW); // on
      else if(status1 == false)
        digitalWrite(myLed_G, HIGH); // off
      if(status2 == true)
        digitalWrite(myLed_B, LOW); // on
      else if(status2 == false)
        digitalWrite(myLed_B, HIGH); // off
      if(status3 == true)
        digitalWrite(myBeep, LOW); // on
      else if(status3 == false)
        digitalWrite(myBeep, HIGH); // off
    }
  }
}

//UDP_Listen_Task 任务主体(监听服务器指令)
void UDP_Listen_Task(void *ptParam){  
  // 初始化当前tick时刻
  TickType_t xLastWakeTime = xTaskGetTickCount();
  // 定义单周期的tick数量
  const TickType_t xFrequency = 5; // 间隔 5 ticks = 0.005 seconds = 200Hz
  
  for(;;) //使用for更高效
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    int packetLength = udp.parsePacket(); 
    if(packetLength){
      len = udp.read(incomingPacket, BUFFER_LENGTH);
      if (len > 0){
          incomingPacket[len] = 0;
          Serial.printf("%s\n", incomingPacket);
          listen_command = true;
      }
    }
  }
}

//System_led_blink 任务主体(兼容debug功能) 
void System_led_blink(void *ptParam)
{
  // 初始化当前tick时刻
  TickType_t xLastWakeTime = xTaskGetTickCount();
  // 定义单周期的tick数量
  const TickType_t xFrequency = 1000; // 间隔 1000 ticks = 1 seconds = 1Hz
  
  for(;;) //使用for更高效
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    // LED_Task 主任务
    led_status = !led_status;
    if(led_status==true)
    {
      digitalWrite(Led_R, LOW); 
      // digitalWrite(Led_G, HIGH);
      // digitalWrite(Led_B, LOW);
    }
    else
    {
      digitalWrite(Led_R, HIGH);
      // digitalWrite(Led_G, LOW); 
      // digitalWrite(Led_B, HIGH); 
    }
    // 串口&UDP发送-for debug
    if(SerialDebug == 1) {
      // Serial.println("System working...");
      // char buffer[80];
      // sprintf(buffer, "Recent status is: %s-%s-%s-%s .\r\n", status0 ? "on" : "off", status1 ? "on" : "off", status2 ? "on" : "off", status3 ? "on" : "off");
      // Serial.println(buffer);
      UDP_send_string_to_target("Waiting cmd!\r\n");
      }
  }
}


/******************************  任务调度相关函数  **********************************/

// #include <Arduino.h>

// #include <ETH.h>


// static bool eth_connected = false;

// void WiFiEvent(WiFiEvent_t event)
// {
//   switch (event) {
//     case ARDUINO_EVENT_ETH_START:
//       Serial.println("ETH Started");
//       //set eth hostname here
//       ETH.setHostname("esp32-ethernet");
//       break;
//     case ARDUINO_EVENT_ETH_CONNECTED:
//       Serial.println("ETH Connected");
//       break;
//     case ARDUINO_EVENT_ETH_GOT_IP:
//       Serial.print("ETH MAC: ");
//       Serial.print(ETH.macAddress());
//       Serial.print(", IPv4: ");
//       Serial.print(ETH.localIP());
//       if (ETH.fullDuplex()) {
//         Serial.print(", FULL_DUPLEX");
//       }
//       Serial.print(", ");
//       Serial.print(ETH.linkSpeed());
//       Serial.println("Mbps");
//       eth_connected = true;
//       break;
//     case ARDUINO_EVENT_ETH_DISCONNECTED:
//       Serial.println("ETH Disconnected");
//       eth_connected = false;
//       break;
//     case ARDUINO_EVENT_ETH_STOP:
//       Serial.println("ETH Stopped");
//       eth_connected = false;
//       break;
//     default:
//       break;
//   }
// }

// void testClient(const char * host, uint16_t port)
// {
//   Serial.print("\nconnecting to ");
//   Serial.println(host);

//   WiFiClient client;
//   if (!client.connect(host, port)) {
//     Serial.println("connection failed");
//     return;
//   }
//   client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
//   while (client.connected() && !client.available());
//   while (client.available()) {
//     Serial.write(client.read());
//   }

//   Serial.println("closing connection\n");
//   client.stop();
// }

// void setup()
// {
//   Serial.begin(115200);
//   WiFi.onEvent(WiFiEvent);
//   ETH.begin();
// }


// void loop()
// {
//   if (eth_connected) {
//     testClient("baidu.com", 80);
//   }
//   delay(10000);
// }