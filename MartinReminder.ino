#include <M5Unified.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>
#include <vector>

#define WIFI_SSID       "VM1299355"
#define WIFI_PASSWORD   "rznaf9jAucyyxsht"
#define FIREBASE_HOST   "https://martinreminder-278e1-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyBF-Ls1eoZH_WiAcM7WqNR4LEr1KnWDI"
#define FIREBASE_USER_EMAIL    "testkieronrose@gmail.com"
#define FIREBASE_USER_PASSWORD "Beamish1407!!"

FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig fbCfg;

bool firebaseReady = false;

struct Item { String key, text; bool done; };
std::vector<Item> items;

void drawList() {
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(20, 20);
  M5.Display.println("Martin's Reminder List");

  struct tm ti;
  if (getLocalTime(&ti)) {
    char buf[40];
    strftime(buf, sizeof(buf), "%A %d %B %Y", &ti);
    M5.Display.setCursor(20, 60);
    M5.Display.println(buf);
  }

  M5.Display.setTextSize(2);
  int y = 120;
  if (items.empty()) {
    M5.Display.setCursor(20, y);
    M5.Display.println("(No items found)");
    return;
  }

  for (auto &it : items) {
    M5.Display.setCursor(40, y);
    M5.Display.printf("[%c] %s", it.done ? '\xFC' : ' ', it.text.c_str());
    y += M5.Display.fontHeight() + 10;
  }

  M5.Display.fillRoundRect(800, 20, 120, 50, 10, TFT_LIGHTGREY);
  M5.Display.setCursor(820, 35);
  M5.Display.println("Refresh");
}

void fetchToday() {
  if (!firebaseReady) return;

  char dateBuf[11];
  strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", localtime(nullptr));
  String path = "reminders/" + String(dateBuf);

  Serial.print("GET ");
  Serial.println(path);

  if (!Firebase.RTDB.getJSON(&fbdo, path)) {
    Serial.print("GET fail: ");
    Serial.println(fbdo.errorReason());
    items.clear();
    drawList();
    return;
  }

  items.clear();
  FirebaseJson &json = fbdo.jsonObject();
  size_t n = json.iteratorBegin();
  for (size_t i = 0; i < n; i++) {
    String key, val; int type;
    json.iteratorGet(i, type, key, val);
    FirebaseJson j2; j2.setJsonData(val);
    FirebaseJsonData t, d;
    j2.get(t, "text"); j2.get(d, "done");
    if (t.success && d.success)
      items.push_back({ key, t.stringValue, d.boolValue });
  }
  json.iteratorEnd();

  drawList();
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Display.setRotation(1);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.p
