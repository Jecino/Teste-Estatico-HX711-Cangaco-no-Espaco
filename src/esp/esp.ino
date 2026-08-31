#include <WiFi.h>
#include <WebServer.h>


const char *ap_ssid = "Banco Estático CnE";
const char *ap_pass = "Cangas168!";

WebServer server(80);

float last = 0;

void updateSensor(){
  last = last + rand() % 100;
}

String getForceString(){
  updateSensor();
  char b[16];
  snprintf(b, sizeof(b), "%.3f N", last);
  return String(b);
}

void handleRoot(){
  String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Banco Estático</title>

</head>

<body>
  <div class="panel">
    <div class="force" id="force">--.- N</div>
  </div>

  <script>
    async function update(){
      try{
        const f = await (await fetch('/force')).text();

        force.textContent = f;
      } catch(e){}
    }
    update();
    setInterval(update, 500);
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Configurando o access point");
  WiFi.mode(WIFI_AP);
  
  WiFi.softAP(ap_ssid, ap_pass);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Meu IP: ");
  Serial.println(myIP);

  server.on("/",handleRoot);
  server.on("/force",[](){server.send(200,"text/plain",getForceString());});
  server.begin();
}

void loop() {
  server.handleClient();
  updateSensor();
}