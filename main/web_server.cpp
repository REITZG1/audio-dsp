#include "web_server.h"
#include "global_vars.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <esp_http_server.h>
#include <cstring>
#include <cstdio>

static const char *TAG = "web";
static httpd_handle_t server = NULL;

// --- Embedded web UI ---
static const char* HTML_INDEX = R"rawliteral(
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Audio Mixer</title>
<style>
body{font-family:sans-serif;margin:20px;background:#1a1a2e;color:#eee}
h1{color:#e94560}
.section{background:#16213e;padding:15px;margin:10px 0;border-radius:8px}
.section h2{margin:0 0 10px 0;color:#0f3460;font-size:16px}
.row{display:flex;justify-content:space-between;align-items:center;padding:4px 0;border-bottom:1px solid #1a1a2e}
.row label{flex:1}
.row .val{text-align:right;margin:0 10px;min-width:40px}
input[type=range]{flex:2;height:4px;accent-color:#e94560}
button{background:#e94560;color:#fff;border:none;padding:8px 20px;border-radius:4px;cursor:pointer;margin:2px}
button.off{background:#555}
.status{display:inline-block;width:12px;height:12px;border-radius:50%;margin-right:6px}
.status.on{background:#0f0}
.status.off{background:#f00}
</style></head><body>
<h1>DSP Audio Mixer</h1>
<div id="status"></div>
<script>
let state={};
function updateStatus(){
 fetch('/api/status').then(r=>r.json()).then(d=>{
  state=d;let h='';
  h+='<div class="section"><h2>System</h2>';
  h+='<div class="row"><label>Sample Rate</label><span>'+d.sr+' Hz</span></div>';
  h+='<div class="row"><label>Channels</label><span>'+d.ch+'</span></div>';
  h+='<div class="row"><label>WiFi</label><span class="status '+(d.wifi?'on':'off')+'"></span><span>'+d.ip+'</span></div>';
  h+='<div class="row"><label>Mute</label><button id="btnMute" class="'+(d.mute?'':'off')+'" onclick="toggleMute()">'+(d.mute?'MUTED':'Active')+'</button></div>';
  h+='</div>';
  h+='<div class="section"><h2>Volume & Balance</h2>';
  h+=slider('Input Level','il',d.il,-50,90,1);
  h+=slider('Output Level','ol',d.ol,-70,10,1);
  h+=slider('Balance','bal',d.bal,0,100,1);
  h+=slider('Clipper','clip',d.clip,-40,40,1);
  h+='</div>';
  h+='<div class="section"><h2>Processing</h2>';
  h+=slider('Pre Emphasis','pre',d.pre,-30,30,1);
  h+=slider('Post Emphasis','post',d.post,-30,30,1);
  h+=slider('Echo','echo',d.echo,0,1,.01);
  h+=slider('Q Factor','qf',d.qf,.1,10,.1);
  h+=slider('Step By','step',d.step,-90,-3,1);
  h+='<div class="row"><label>Compressor</label><button id="btnComp" class="'+(d.comp?'':'off')+'" onclick="toggleComp()">'+(d.comp?'ON':'OFF')+'</button></div>';
  h+='<div class="row"><label>Band Sync</label><button id="btnSync" class="'+(d.sync?'':'off')+'" onclick="toggleSync()">'+(d.sync?'ON':'OFF')+'</button></div>';
  h+='<div class="row"><label>Limiter</label><button id="btnLim" class="'+(d.res1?'':'off')+'" onclick="toggleLim()">'+(d.res1?'ON':'OFF')+'</button></div>';
  h+='</div>';
  h+='<div class="section"><h2>Bands ('+d.nb+' bands)</h2>';
  for(let i=0;i<d.nb;i++){
   h+=slider('Band '+(i+1)+' EQ','eq'+i,d.eq[i],-12,12,.5);
   h+=slider('Band '+(i+1)+' Gain','gn'+i,d.gn[i],0,80,1);
   h+=slider('Protection','prot'+i,d.prot[i],0,30,1);
  }
  h+='</div>';
  h+='<div class="section"><button onclick="saveConfig()">Save Config</button><button onclick="reboot()">Reboot</button></div>';
  document.getElementById('status').innerHTML=h;
 }).catch(()=>setTimeout(updateStatus,2000));
}
function slider(label,key,val,min,max,step){
 return '<div class="row"><label>'+label+'</label><input type="range" min="'+min+'" max="'+max+'" step="'+step+'" value="'+val+'" oninput="setVal(\''+key+'\',this.value)" onchange="setVal(\''+key+'\',this.value)"><span class="val">'+Number(val).toFixed((step<1)?1:0)+'</span></div>';
}
function setVal(key,val){
 if(key.startsWith('eq')){let i=parseInt(key.slice(2));fetch('/api/set',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({k:'eq',i:i,v:parseFloat(val)})});}
 else if(key.startsWith('gn')){let i=parseInt(key.slice(2));fetch('/api/set',{method:'POST',body:JSON.stringify({k:'gn',i:i,v:parseFloat(val)})});}
 else if(key.startsWith('prot')){let i=parseInt(key.slice(4));fetch('/api/set',{method:'POST',body:JSON.stringify({k:'prot',i:i,v:parseFloat(val)})});}
 else{fetch('/api/set',{method:'POST',body:JSON.stringify({k:key,v:parseFloat(val)})});}
 // Update display
 let spans=document.querySelectorAll('.val');
 spans.forEach(s=>{let p=s.parentElement;let inp=p.querySelector('input');if(inp&&inp==document.activeElement)s.textContent=Number(val).toFixed((parseFloat(inp.step)<1)?1:0);});
}
function toggleMute(){fetch('/api/set',{method:'POST',body:JSON.stringify({k:'mute',v:state.mute?0:1})}).then(()=>updateStatus());}
function toggleComp(){fetch('/api/set',{method:'POST',body:JSON.stringify({k:'comp',v:state.comp?0:1})}).then(()=>updateStatus());}
function toggleSync(){fetch('/api/set',{method:'POST',body:JSON.stringify({k:'sync',v:state.sync?0:1})}).then(()=>updateStatus());}
function toggleLim(){fetch('/api/set',{method:'POST',body:JSON.stringify({k:'res1',v:state.res1?0:1})}).then(()=>updateStatus());}
function saveConfig(){fetch('/api/save',{method:'POST'}).then(()=>alert('Saved!'));}
function reboot(){fetch('/api/reboot',{method:'POST'}).then(()=>setTimeout(function(){location.reload()},5000));}
updateStatus();setInterval(updateStatus,3000);
</script></body></html>
)rawliteral";

// --- REST API handlers ---

static esp_err_t api_status_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");

  // Build JSON using ArduinoJson (already in project)
  DynamicJsonDocument doc(4096);

  doc["sr"] = SampleRateFreq;
  doc["ch"] = channelCount;
  doc["wifi"] = true;

  // Get IP
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("STA_DEF");
  if (netif) {
    esp_netif_ip_info_t ip;
    if (esp_netif_get_ip_info(netif, &ip) == ESP_OK) {
      char ipstr[16];
      snprintf(ipstr, sizeof(ipstr), IPSTR, IP2STR(&ip.ip));
      doc["ip"] = ipstr;
    } else {
      doc["ip"] = "0.0.0.0";
    }
  } else {
    doc["ip"] = "0.0.0.0";
  }

  doc["il"] = InputLevel;
  doc["ol"] = OutputLevel;
  doc["bal"] = Balance;
  doc["clip"] = Clipper;
  doc["comp"] = Compressor;
  doc["sync"] = BandSync;
  doc["mute"] = Mute;
  doc["res1"] = Reserved1;
  doc["nb"] = NumBands;
  doc["pre"] = PreEmphasis;
  doc["post"] = PostEmphasis;
  doc["step"] = StepBy;
  doc["echo"] = Echo;
  doc["qf"] = FiltersQFactor;

  JsonArray eq = doc.createNestedArray("eq");
  JsonArray gn = doc.createNestedArray("gn");
  JsonArray prot = doc.createNestedArray("prot");

  for (int i = 0; i < ALL_NUM_BANDS; i++) {
    if (i < MAX_NUM_BANDS) eq.add(Equalizer[i]);
    gn.add(Gain[i]);
    prot.add(Protection[i]);
  }

  String output;
  serializeJson(doc, output);

  httpd_resp_sendstr(req, output.c_str());
  return ESP_OK;
}

static esp_err_t api_set_handler(httpd_req_t *req) {
  char buf[256];
  int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
  if (ret <= 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No data");
    return ESP_FAIL;
  }
  buf[ret] = '\0';

  DynamicJsonDocument doc(256);
  DeserializationError err = deserializeJson(doc, buf);
  if (err) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad JSON");
    return ESP_FAIL;
  }

  const char *k = doc["k"];
  float v = doc["v"] | 0.0f;
  int idx = doc["i"] | -1;

  auto setBandVal = [&](float arr[], float minv, float maxv) {
    int i = idx;
    if (i >= 0 && i < ALL_NUM_BANDS) {
      arr[i] = constrain(v, minv, maxv);
    }
  };

  if (strcmp(k, "il") == 0) { InputLevel = constrain(v, MIN_INPUT_LEVEL, MAX_INPUT_LEVEL); }
  else if (strcmp(k, "ol") == 0) { OutputLevel = constrain(v, MIN_OUTPUT_LEVEL, MAX_OUTPUT_LEVEL); }
  else if (strcmp(k, "bal") == 0) { Balance = constrain(v, MIN_BALANCE, MAX_BALANCE); }
  else if (strcmp(k, "clip") == 0) { Clipper = constrain(v, MIN_CLIPPER, MAX_CLIPPER); }
  else if (strcmp(k, "comp") == 0) { Compressor = v > 0.5f; }
  else if (strcmp(k, "sync") == 0) { BandSync = v > 0.5f; }
  else if (strcmp(k, "mute") == 0) { Mute = v > 0.5f; }
  else if (strcmp(k, "res1") == 0) { Reserved1 = v > 0.5f; }
  else if (strcmp(k, "pre") == 0) { PreEmphasis = constrain(v, MIN_PRE_EMPHASIS, MAX_PRE_EMPHASIS); }
  else if (strcmp(k, "post") == 0) { PostEmphasis = constrain(v, MIN_POST_EMPHASIS, MAX_POST_EMPHASIS); }
  else if (strcmp(k, "step") == 0) { StepBy = constrain(v, MIN_STEP_BY, MAX_STEP_BY); }
  else if (strcmp(k, "echo") == 0) { Echo = constrain(v, MIN_ECHO, MAX_ECHO); }
  else if (strcmp(k, "qf") == 0) { FiltersQFactor = constrain(v, MIN_FILTERS_Q_FACTOR, MAX_FILTERS_Q_FACTOR); }
  else if (strcmp(k, "eq") == 0) { setBandVal(Equalizer, MIN_EQ_BAND, MAX_EQ_BAND); }
  else if (strcmp(k, "gn") == 0) { setBandVal(Gain, MIN_GAIN, MAX_GAIN); }
  else if (strcmp(k, "prot") == 0) { setBandVal(Protection, MIN_PROTECTION, MAX_PROTECTION); }

  commitConfig();
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, "{\"ok\":true}");
  return ESP_OK;
}

static esp_err_t api_save_handler(httpd_req_t *req) {
  saveConfig();
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, "{\"ok\":true}");
  return ESP_OK;
}

static esp_err_t api_reboot_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, "{\"ok\":true}");
  esp_restart();
  return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_sendstr(req, HTML_INDEX);
  return ESP_OK;
}

static const httpd_uri_t uri_index = {
  .uri = "/", .method = HTTP_GET, .handler = index_handler
};

static const httpd_uri_t uri_api_status = {
  .uri = "/api/status", .method = HTTP_GET, .handler = api_status_handler
};

static const httpd_uri_t uri_api_set = {
  .uri = "/api/set", .method = HTTP_POST, .handler = api_set_handler
};

static const httpd_uri_t uri_api_save = {
  .uri = "/api/save", .method = HTTP_POST, .handler = api_save_handler
};

static const httpd_uri_t uri_api_reboot = {
  .uri = "/api/reboot", .method = HTTP_POST, .handler = api_reboot_handler
};

esp_err_t startWebServer() {
  if (server) return ESP_OK;

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 10;
  config.stack_size = 8192;
  config.lru_purge_enable = true;

  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return ESP_FAIL;
  }

  httpd_register_uri_handler(server, &uri_index);
  httpd_register_uri_handler(server, &uri_api_status);
  httpd_register_uri_handler(server, &uri_api_set);
  httpd_register_uri_handler(server, &uri_api_save);
  httpd_register_uri_handler(server, &uri_api_reboot);

  ESP_LOGI(TAG, "Web server started on port %d", config.server_port);
  return ESP_OK;
}

esp_err_t stopWebServer() {
  if (server) {
    httpd_stop(server);
    server = NULL;
  }
  return ESP_OK;
}

// --- WiFi Initialization ---
esp_err_t initWiFi() {
  ESP_LOGI(TAG, "Initializing WiFi (ESP32-C6 via SDIO)...");

  // NVS init
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Network stack
  ESP_ERROR_CHECK(esp_netif_init());
  ret = esp_event_loop_create_default();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_ERROR_CHECK(ret);
  }

  // STA netif
  esp_netif_create_default_wifi_sta();

  // WiFi init (this triggers esp_hosted SDIO initialization)
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
    return ret;
  }

  // Event handler
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
    WIFI_EVENT, ESP_EVENT_ANY_ID,
    [](void*, esp_event_base_t, int32_t event_id, void*) {
      if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
      } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WiFi disconnected, reconnecting...");
      }
    }, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
    IP_EVENT, IP_EVENT_STA_GOT_IP,
    [](void*, esp_event_base_t, int32_t, void* data) {
      ip_event_got_ip_t *event = (ip_event_got_ip_t*)data;
      ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }, NULL, &instance_got_ip));

  // Station mode
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

  // Read SSID/pass from config (or use defaults)
  String ssid = readFile("/wifi.txt");
  String pass = "";
  if (ssid.isEmpty()) {
    ssid = "dsp-mixer";
    pass = "mixer1234";
    ESP_LOGW(TAG, "No /wifi.txt found, using AP mode: '%s'", ssid.c_str());
  }

  if (pass.isEmpty()) {
    // Try to read password from same file (format: SSID\\nPASS)
    int newline = ssid.indexOf('\n');
    if (newline > 0) {
      pass = ssid.substring(newline + 1);
      pass.trim();
      ssid = ssid.substring(0, newline);
      ssid.trim();
    }
  }

  wifi_config_t wifi_config = {};

  // If no SSID configured, use AP mode
  if (ssid == "dsp-mixer" && pass == "mixer1234") {
    // AP mode as fallback
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = {};
    strncpy((char*)ap_config.ap.ssid, "AudioMixer-P4", sizeof(ap_config.ap.ssid) - 1);
    strncpy((char*)ap_config.ap.password, "mixer1234", sizeof(ap_config.ap.password) - 1);
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = 4;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_LOGI(TAG, "AP mode: SSID 'AudioMixer-P4', password 'mixer1234'");
  } else {
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    if (!pass.isEmpty()) {
      strncpy((char*)wifi_config.sta.password, pass.c_str(), sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_LOGI(TAG, "STA mode: SSID '%s'", ssid.c_str());
  }

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WiFi init complete");
  return ESP_OK;
}
