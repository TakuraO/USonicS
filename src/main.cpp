#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoGraphics.h>
#include <ARDUINO_LED_MATRIX.h> // Arduino UNO R4 WiFiのLEDマトリクス用

// プロトタイプ宣言
void LEDTextScroll(const char* txt);
void LEDTextDraw(const char* dist);
void showNear();
void showMiddle();
void showFar();
void sendHttpCommand(String distanceStatus);

// HC-SR04用のピン定義
const int trigPin = 9; // トリガーピン
const int echoPin = 10; // エコーピン

// LED用のピン定義
const int RedPin = 3; // RGBのRピン
const int GreenPin = 5; // RGBのGピン
const int BluePin = 6; // RGBのBピン

// Wi-Fi接続情報
//const char* ssid = "ssid";
//const char* password = "pass";

// HTTPサーバーの情報
//const char* serverAddress = "192.168.1.123"; // サーバーアドレス記載例
const char* serverAddress = "your_http_server.com"; // サーバーアドレス
const int port = 80; // サーバーポート番号
//const char* resource = "/your_endpoint"; // リソースパス記載例
const char* resource = "/api/control"; // リソースパス

// LEDマトリクス制御用オブジェクト
ArduinoLEDMatrix ledMatrix;

// WiFiClientとHttpClient
WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, serverAddress, port);

// 定数（閾値設定）
const int NEAR_DISTANCE = 10;
const int MID_DISTANCE = 50;

// 距離計測用の変数
long duration;
int distance;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LEDマトリクスの初期化
  ledMatrix.begin();
  LEDTextScroll("Wifi Connecting...");

  // Wi-Fi接続
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(50);
    Serial.print(".");
    LEDTextScroll("...");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
    LEDTextScroll("  Wifi connected");
  } else {
    Serial.println("\nWi-Fi connection failed");
    LEDTextScroll("  Wifi Failed");
  }
  delay(2000);
  ledMatrix.clear();
}

void loop() {
  // 距離を計測
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  if (distance <= NEAR_DISTANCE) {
    showNear(); 
    sendHttpCommand("近い");
    analogWrite(RedPin,255);
    analogWrite(GreenPin,0);
    analogWrite(BluePin,0);
  } else if (distance <= MID_DISTANCE) {
    showMiddle();
    sendHttpCommand("丁度良い");
    analogWrite(RedPin,150);
    analogWrite(GreenPin,50);
    analogWrite(BluePin,0);
  } else {
    showFar();
    sendHttpCommand("遠い");
    analogWrite(RedPin,0);
    analogWrite(GreenPin,255);
    analogWrite(BluePin,0);
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  String distStr = String(distance);
  LEDTextDraw(distStr.c_str());
  delay(5000);
}

void LEDTextScroll(const char* txt){
  ledMatrix.beginDraw();
  ledMatrix.stroke(0xFFFFFFFF);
  ledMatrix.textFont(Font_4x6);
  ledMatrix.beginText(0,1,0xFFFFFF0);
  ledMatrix.println(txt);
  ledMatrix.textScrollSpeed(100);
  ledMatrix.endText(SCROLL_LEFT);
  ledMatrix.endDraw();
 // delay(2000);
}

void LEDTextDraw(const char* dist){
  ledMatrix.beginDraw();
  ledMatrix.stroke(0xFFFFFFFF);
  ledMatrix.textFont(Font_5x7);
  ledMatrix.beginText(0,1,0xFFFFFF0);
  ledMatrix.println(dist);
  delay(1000);
  ledMatrix.endText();
  ledMatrix.endDraw();
 // delay(2000);
}

void showNear() {
  ledMatrix.clear();
  ledMatrix.print("Near");
//  ledMatrix.display();
}

void showMiddle() {
  ledMatrix.clear();
  ledMatrix.print("Middle");
//  ledMatrix.display();
}

void showFar() {
  ledMatrix.clear();
  ledMatrix.print("Far");
//  ledMatrix.display();
}

void sendHttpCommand(String distanceStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    // GETリクエスト用のURLパラメータを構築
//    String url = String(resource) + "?status=" + distanceStatus;
    String url = String(resource) + "?speech=" + distanceStatus;
//    String url = "http://192.168.1.123/api/control?speech=あいうえお";

    // GETリクエストを送信
    Serial.println(url);
    httpClient.get(url);
    // レスポンスのステータスコードと本文を取得
    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    // 結果をシリアルモニタに出力
    Serial.print("HTTP Response Code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.println("Wi-Fi not connected, unable to send GET request");
      }
}
