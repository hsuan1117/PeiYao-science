#include <SoftwareSerial.h>
/* Copyright (c) 2021 Hsuan,Wei
 * For more details, see https://www.sivir.pw , https://hsuan.app
 * Released under the MIT license.
 * */

/**
 * 旋轉延時 <br>
 *
 * 值越小轉越快，太小會轉不動
 * */
const int ROTATE_DELAY = 10;

/**
 * 亮度感測器 <br>
 *
 * <li><b>8</b> CT ==> uart_tx(mode 1)</li>
 * <li><b>9</b> DR ==> uart_rx(mode 1)</li>
 * 接口和arduino的 rx tx 要相反
 *
 * @param rx : int 接收資料端口
 * @param tx : int 重送資料端口
 *
 * @see <a href="https://www.arduino.cc/en/Reference/SoftwareSerialConstructor">說明文件</a>
 **/
SoftwareSerial light(10, 9);

/**
 * 存放取得的資料
 * */
byte data[32];

/**
 * TODO:
 *  - 轉一圈，偵測最大值
 *  - 手動輸入角度，轉到該腳開始檢測亮度
 * */
namespace ArduinoToolKit {
    enum class LogType {
        Info, Error, Debug
    };

    void log(LogType type, string msg) {
        switch (type) {
            case LogType::Info:
                Serial.println("[Info] " + msg);
                break;
            case LogType::Error:
                Serial.println("[Error] " + msg);
                break;
            case LogType::Debug:
                Serial.println("[Error] " + msg);
                break;
            default:
                Serial.println("[Unknown] " + msg);
        }

    }
}

class GY39 {

private:
    static bool verify_data(int length, int start,*byte frame) {
        byte btmp = 0;
        for (int i = 0; i < length; ++i) {
            btmp += *frame[start + i];
        }
        if (btmp == *frame[start + length - 1])
            return true;
        return false;
    }

public:
    static const byte FRAME_FLAG = 0x5A;
    static const byte FRAME_DATATYPE_LIGHT = 0x15;
    static const byte FRAME_DATATYPE_WET = 0x45;
    static const byte FRAME_DATATYPE_IIC = 0x55;

    /***
     *
     *
     * @see
     *  <a href="https://www.taiwaniot.com.tw/wp-content/uploads/woocommerce_uploads/2016/10/GY39_manual.pdf" >GY39 說明文件</a>
     * @return
     */
    static double calculate(*byte frame) {
        int start;
        delay(10);

        //尋找資料起始點
        for (int i = 0; i < 26; ++i) {
            if (*frame[i] == FRAME_FLAG && *frame[i + 1] == FRAME_FLAG) {
                Serial.println("[Info] Got sensor data.");
                start = i;
                break;
            }
        }

        delay(100);
        if (*frame[start + 2] == FRAME_DATATYPE_LIGHT) {
            Serial.println("[Info] DataType: Light");
            verify_data(8, start);

            Serial.println("[Info] Light = ");
            double lux = (*frame[start + 4] << 24) + (*frame[start + 5] << 16) + (*frame[start + 6] << 8) + (*frame[start + 7]);
            Serial.print(lux / 100);
            return lux;

        } else {
            Serial.println("[Error] Method not allowed.");
        }
    }

    void read_data(*byte frame) {
        for (int i = 0; i < 25; ++i) {
            frame[i] = light.read();
            Serial.print(*frame[i]);
            Serial.print(" ");
        }
    }


};


void stepForward();

/**
 * 初始化Arduino板設定
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

    digitalWrite(11, HIGH);
    digitalWrite(8, LOW);


    // 亮度感測器模式
    byte mode[3] = {0xA5, 0x81, 0x26};
    for (int i = 0; i < 3; ++i) {
        light.print(mode[i]);
    }
}

void loop() {
    delay(1000);

    while (Serial.available()) {
        char c;
        int input;
        while((c = Serial.read() && c != '!')) {
            // 輸入方式: 數字!
            input += c-'0';
        }

        int M = -1;
        for (int i = 1; i <= input; i++) {
            stepForward();
            GY39::read_data(*data);
            M = max(M,GY39::calculate());
        }


        Serial.print("\n");

        double val = GY39::calculate();
        Serial.print("Gear: ");
        Serial.print(input);
        Serial.print("\n");

        Serial.print("Lux: ");
        Serial.print(abs(val));
        Serial.print("\n");

        Serial.print("Lux Max: ");
        Serial.print(M);
        Serial.print("\n");

        delay(10000);
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
                     {1, 0, 0, 1}};
    for (int i = 0; i < 8; ++i) {
        digitalWrite(A0, arr[i][0]);
        digitalWrite(A1, arr[i][1]);
        digitalWrite(A2, arr[i][2]);
        digitalWrite(A3, arr[i][3]);
        delay(ROTATE_DELAY);
    }
}
