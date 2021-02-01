#include <SoftwareSerial.h>
#include <Stepper.h>
/* File: GetMax
 * Copyright (c) 2021 Hsuan,Wei
   For more details, see https://www.sivir.pw , https://hsuan.app
   Released under the MIT license.
 * */

/**
   旋轉延時 <br>

   值越小轉越快，太小會轉不動
 * */
const int ROTATE_DELAY = 2;
const int MOTOR_STEP   = 2048;



/**
   亮度感測器 <br>

   <li><b>8</b> CT ==> uart_tx(mode 1)</li>
   <li><b>9</b> DR ==> uart_rx(mode 1)</li>
   接口和arduino的 rx tx 要相反

   @param rx : int 接收資料端口
   @param tx : int 重送資料端口

   @see <a href="https://www.arduino.cc/en/Reference/SoftwareSerialConstructor">說明文件</a>
 **/
SoftwareSerial light(10, 9);
Stepper motor(MOTOR_STEP,A0,A1,A2,A3);
/**
   存放取得的資料
 * */
byte data[32];

/**
   TODO:
    - 轉一圈，偵測最大值
    - 手動輸入角度，轉到該腳開始檢測亮度
 * */
class GY39 {

  private:
    static bool verify_data(int length, int start) {
      byte btmp = 0;
      for (int i = 0; i < length; ++i) {
        btmp += data[start + i];
      }
      if (btmp == data[start + length - 1])
        return true;
      return false;
    }

  public:
    static const byte FRAME_FLAG = 0x5A;
    static const byte FRAME_DATATYPE_LIGHT = 0x15;
    static const byte FRAME_DATATYPE_WET = 0x45;
    static const byte FRAME_DATATYPE_IIC = 0x55;

    /***


       @see
        <a href="https://www.taiwaniot.com.tw/wp-content/uploads/woocommerce_uploads/2016/10/GY39_manual.pdf" >GY39 說明文件</a>
       @return
    */
    static double calculate() {
      int start;
      //delay(10);

      //尋找資料起始點
      for (int i = 0; i < 26; ++i) {
        if (data[i] == FRAME_FLAG && data[i + 1] == FRAME_FLAG) {
          Serial.println("[Info] Got sensor data.");
          start = i;
          break;
        }
      }

      //delay(100);
      if (data[start + 2] == FRAME_DATATYPE_LIGHT) {
        Serial.println("[Info] DataType: Light");
        verify_data(8, start);

        Serial.print("[Info] Light = ");
        double lux = (data[start + 4] << 24) + (data[start + 5] << 16) + (data[start + 6] << 8) + (data[start + 7]);
        Serial.println(abs(lux / 100.0));
        return abs(lux / 100.0);

      } else {
        Serial.println("[Error] Method not allowed.");
      }
    }

    void read_data() {
      for (int i = 0; i < 25; ++i) {
        data[i] = light.read();
        Serial.print(data[i]);
        Serial.print(" ");
      }
      Serial.print('\n');
    }


};
GY39 Device;

void stepForward();

/**
   初始化Arduino板設定
 * */
void setup() {
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(11, OUTPUT);

  /// 初始化Arduino 序列阜
  Serial.begin(115200);
  light.begin(9600);

  motor.setSpeed(5);
  digitalWrite(11, HIGH);
  digitalWrite(8, LOW);

  Serial.print("Start");
  // 亮度感測器模式
  byte mode[3] = {0xA5, 0x81, 0x26};
  for (int i = 0; i < 3; ++i) {
    light.print(mode[i]);
  }
}
int iipt;
void loop() {
  
  delay(500);
  //while (Serial.available()) {
  iipt = Serial.readStringUntil('!').toInt();
  if (iipt != 0) {

    Serial.print("[Debug] ");
    Serial.println(iipt);

    double M = -1.0;
    double m = 0x3f3f3f3f;
    double gear = -1;
    for (int i = 1; i <= 2048; i++) {
      Serial.print("[Info] Time: ");
      Serial.println(millis());
      //stepForward();
      
      Device.read_data();
      double result = Device.calculate();
      if(result > M){
          gear = i;
      }
      M = max(M, result);
      if(result > 0.0){
        m = min(m, result);
      }
      
      motor.step(1);
    }


    Serial.print("\n");

    Serial.print("Gear: ");
    Serial.print(gear);
    Serial.print("\n");

    Serial.print("Lux min: ");
    Serial.print(m);
    Serial.print("\n");

    Serial.print("Lux Max: ");
    Serial.print(M);
    Serial.print("\n");

  }
}

void stepForward() {
  int arr[8][4] = {{1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
  };
  for (int i = 0; i < 8; ++i) {
    digitalWrite(A0, arr[i][0]);
    digitalWrite(A1, arr[i][1]);
    digitalWrite(A2, arr[i][2]);
    digitalWrite(A3, arr[i][3]);
    delay(ROTATE_DELAY);
  }
}
