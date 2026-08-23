#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===================== WiFi =====================
// Set these locally before uploading. Do not publish real credentials.
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* AP_SSID = "ESP-Canvas";
const char* AP_PASS = "12345678";

// ===================== OLED =====================
#define OLED_W 128
#define OLED_H 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);
WebServer server(80);
WebSocketsServer ws(81);

bool renderNeeded = false;
unsigned long lastRender = 0;

void drawLine(int x0, int y0, int x1, int y1, int size) {
  x0 = constrain(x0, 0, OLED_W - 1);
  x1 = constrain(x1, 0, OLED_W - 1);
  y0 = constrain(y0, 0, OLED_H - 1);
  y1 = constrain(y1, 0, OLED_H - 1);

  if (size <= 1) {
    display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
    renderNeeded = true;
    return;
  }

  int r = size / 2;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while (true) {
    display.fillCircle(x0, y0, r, SSD1306_WHITE);
    if (x0 == x1 && y0 == y1) break;

    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }

  renderNeeded = true;
}

void drawPoint(int x, int y, int size) {
  x = constrain(x, 0, OLED_W - 1);
  y = constrain(y, 0, OLED_H - 1);
  display.fillCircle(x, y, size / 2, SSD1306_WHITE);
  renderNeeded = true;
}

void command(char* s) {
  char* type = strtok(s, ",");
  if (!type) return;

  if (!strcmp(type, "CLR")) {
    display.clearDisplay();
    renderNeeded = true;
    return;
  }

  if (!strcmp(type, "P")) {
    char* a = strtok(NULL, ",");
    char* b = strtok(NULL, ",");
    char* c = strtok(NULL, ",");
    if (a && b && c) drawPoint(atoi(a), atoi(b), atoi(c));
    return;
  }

  if (!strcmp(type, "L")) {
    char* a = strtok(NULL, ",");
    char* b = strtok(NULL, ",");
    char* c = strtok(NULL, ",");
    char* d = strtok(NULL, ",");
    char* e = strtok(NULL, ",");
    if (a && b && c && d && e) {
      drawLine(atoi(a), atoi(b), atoi(c), atoi(d), atoi(e));
    }
  }
}

void wsEvent(uint8_t client, WStype_t type, uint8_t* data, size_t len) {
  if (type == WStype_CONNECTED) {
    ws.sendTXT(client, "READY");
  } else if (type == WStype_TEXT) {
    if (len >= 900) return;

    char buf[900];
    memcpy(buf, data, len);
    buf[len] = '\0';

    char* save = nullptr;
    char* token = strtok_r(buf, ";", &save);
    while (token) {
      command(token);
      token = strtok_r(nullptr, ";", &save);
    }
  }
}

const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 OLED Draw Pad</title>
<style>
body{margin:0;padding:20px;background:#0b0e11;color:white;font-family:Arial;text-align:center}
main{max-width:650px;margin:auto}
canvas{width:100%;max-width:600px;aspect-ratio:2/1;background:#000;image-rendering:pixelated;touch-action:none;border:1px solid #333}
#status{margin:10px;color:#888}.live{color:#6ee7d8!important}
.controls{margin-top:15px}button{margin-top:12px;width:100%;padding:12px;background:#171717;color:#ffb454;border:1px solid #444;border-radius:8px}
</style>
</head>
<body>
<main>
<h3>ESP32 OLED DRAW PAD</h3>
<div id="status">Connecting...</div>
<canvas id="pad" width="128" height="64"></canvas>
<div class="controls">
<label>Pen: <input id="size" type="range" min="1" max="8" value="2"><span id="val">2</span></label>
<button id="clear">Clear OLED</button>
</div>
</main>
<script>
const canvas=document.getElementById("pad"),ctx=canvas.getContext("2d"),status=document.getElementById("status"),size=document.getElementById("size"),val=document.getElementById("val"),clear=document.getElementById("clear");
ctx.fillStyle="#000";ctx.fillRect(0,0,128,64);
let pen=2,drawing=false,lastX=0,lastY=0,socket,queue=[];
size.oninput=()=>{pen=+size.value;val.textContent=pen};
function connect(){status.textContent="Connecting...";status.classList.remove("live");socket=new WebSocket("ws://"+location.hostname+":81/");socket.onopen=()=>{status.textContent="WebSocket LIVE";status.classList.add("live")};socket.onclose=()=>{status.textContent="Reconnecting...";status.classList.remove("live");setTimeout(connect,1000)}}
connect();
function send(s){queue.push(s)}
setInterval(()=>{if(socket&&socket.readyState===WebSocket.OPEN&&queue.length){socket.send(queue.join(";"));queue=[]}},20);
function point(x,y){const r=canvas.getBoundingClientRect();return[Math.max(0,Math.min(127,Math.round((x-r.left)/r.width*128))),Math.max(0,Math.min(63,Math.round((y-r.top)/r.height*64))) ]}
function dot(x,y,s){ctx.fillStyle="#fff";ctx.beginPath();ctx.arc(x,y,Math.max(.5,s/2),0,Math.PI*2);ctx.fill()}
canvas.onpointerdown=e=>{canvas.setPointerCapture(e.pointerId);[lastX,lastY]=point(e.clientX,e.clientY);drawing=true;dot(lastX,lastY,pen);send(`P,${lastX},${lastY},${pen}`)};
canvas.onpointermove=e=>{if(!drawing)return;const[x,y]=point(e.clientX,e.clientY);if(x===lastX&&y===lastY)return;ctx.strokeStyle="#fff";ctx.lineWidth=pen;ctx.lineCap="round";ctx.beginPath();ctx.moveTo(lastX,lastY);ctx.lineTo(x,y);ctx.stroke();send(`L,${lastX},${lastY},${x},${y},${pen}`);lastX=x;lastY=y};
canvas.onpointerup=canvas.onpointercancel=e=>{drawing=false;try{canvas.releasePointerCapture(e.pointerId)}catch(_) {}};
clear.onclick=()=>{ctx.fillStyle="#000";ctx.fillRect(0,0,128,64);send("CLR")};
</script>
</body>
</html>
)HTML";

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(21, 22);
  Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 DRAW PAD");
  display.println();
  display.println("Connecting WiFi...");
  display.display();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) delay(250);

  display.clearDisplay();
  display.setCursor(0, 0);

  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi connected");
    display.println();
    display.println("Open:");
    display.println(WiFi.localIP());
  } else {
    display.println("WiFi failed");
    display.println();
    display.println("Hotspot:");
    display.println(AP_SSID);
    display.println(WiFi.softAPIP());
  }
  display.display();

  server.on("/", []() { server.send_P(200, "text/html", PAGE); });
  server.begin();

  ws.begin();
  ws.onEvent(wsEvent);
}

void loop() {
  server.handleClient();
  ws.loop();

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
  }

  if (renderNeeded && millis() - lastRender >= 20) {
    display.display();
    renderNeeded = false;
    lastRender = millis();
  }

  delay(1);
}
