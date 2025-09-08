#define BLYNK_TEMPLATE_ID "TMPL62E4kN51k"
#define BLYNK_TEMPLATE_NAME "Fire Alert"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>


// ====== THÔNG TIN MẠNG ======
char ssid[] = "OPPO Reno4";        // Tên WiFi
char pass[] = "thanh@86868686";    // Mật khẩu WiFi

// ====== BLYNK AUTH TOKEN ======
char auth[] = "76VxtAkWbYiKCTtxftYWRhyPsrDcaz-Z"; // Lấy từ blynk.cloud

// ====== UART GIAO TIẾP VỚI STM32 ======
// Dùng Serial2 với chân 16 (RX), 17 (TX)
#define STM32_RX 16
#define STM32_TX 17

void setup() {
  Serial.begin(9600); // Debug
  Serial2.begin(9600, SERIAL_8N1, STM32_RX, STM32_TX); // UART giao tiếp STM32
  Serial.println("Hello");

  Blynk.begin(auth, ssid, pass); // Kết nối Blynk + WiFi
  Serial.println("Đã ket noi wifi ");
}

void loop() {
  Blynk.run();
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();  // Xóa khoảng trắng/thừa
    Serial.println("Đã nhận UART từ STM32: " + msg);

    if (msg == "FIRE") {
      Serial.println(">> GỬI CẢNH BÁO LÊN BLYNK!");
      Blynk.logEvent("fire_alert", "🔥 Hệ thống phát hiện cháy!");
    }
  }
}
