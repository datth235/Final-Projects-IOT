#include "WebUI.h"
#include "Config.h"
#include "Utils.h"
#include "CardDB.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include "RFIDHandler.h"
#include <WiFi.h>
#include <WebServer.h>

String apIP = "192.168.4.1";

static WebServer server(80);

// ============================================================
// HTML
// ============================================================
static const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Bai do xe thong minh</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f4f6f9; margin: 0; color: #1f2937; }
    .wrap { max-width: 1100px; margin: 16px auto; padding: 12px; }
    .title { font-size: 28px; font-weight: 700; margin-bottom: 12px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 12px; }
    .slotGrid { display: grid; grid-template-columns: repeat(auto-fit, minmax(120px, 1fr)); gap: 10px; margin-top: 12px; }
    .card { background: #fff; border-radius: 14px; padding: 14px; box-shadow: 0 2px 10px rgba(0,0,0,.08); }
    .big { font-size: 28px; font-weight: 700; margin-top: 6px; }
    .slot { border-radius: 14px; padding: 14px; text-align: center; font-weight: 700; border: 2px solid #ddd; background: #fff; }
    .slot-empty { background: #eafaf0; border-color: #27ae60; color: #1f7a43; }
    .slot-full  { background: #fdecec; border-color: #e74c3c; color: #b63125; }
    .row { display: flex; gap: 8px; flex-wrap: wrap; align-items: center; }
    .actions { display: flex; gap: 6px; flex-wrap: wrap; }
    input[type=text] { padding: 9px 10px; font-size: 14px; border: 1px solid #cbd5e1; border-radius: 10px; min-width: 180px; }
    button { padding: 9px 12px; border: none; border-radius: 10px; background: #2563eb; color: #fff; font-weight: 700; cursor: pointer; }
    button.alt     { background: #475569; }
    button.warnBtn { background: #d97706; }
    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { padding: 8px; border-bottom: 1px solid #e5e7eb; text-align: left; font-size: 14px; vertical-align: top; }
    .muted { color: #64748b; }
    .ok   { color: #059669; font-weight: 700; }
    .warn { color: #d97706; font-weight: 700; }
    .badge { display: inline-block; padding: 4px 8px; border-radius: 999px; font-size: 12px; font-weight: 700; }
    .badge-in  { background: #dcfce7; color: #166534; }
    .badge-out { background: #e2e8f0; color: #334155; }
    .insideList { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 10px; }
    .insideItem { border: 1px solid #dbeafe; background: #f8fbff; border-radius: 12px; padding: 10px; }
    .insidePlate { font-size: 18px; font-weight: 700; }
    .small { padding: 7px 10px; font-size: 12px; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="title">Bai do xe thong minh</div>
    <div class="grid">
      <div class="card"><div>O trong</div><div class="big" id="freeCount">--</div></div>
      <div class="card"><div>O co xe</div><div class="big" id="usedCount">--</div></div>
      <div class="card"><div>Xe trong bai</div><div class="big" id="insideCount">--</div></div>
      <div class="card"><div>Khoang cach cong</div><div class="big" id="distance">--</div></div>
      <div class="card"><div>IP</div><div class="big" id="ipText">192.168.4.1</div></div>
    </div>
    <div class="card" style="margin-top:12px;">
      <h3 style="margin-top:0;">Dang ky / doi the / sua bien so</h3>
      <p class="muted">Dua xe vao gan cong, cam bien sieu am phai do duoc &lt;= 5.0 cm moi cho quet the. Quet the moi de tao UID dang cho, sau do luu bien so hoac bam Doi the trong bang duoi.</p>
      <p>UID dang cho: <span id="pendingUid" class="warn">Chua co</span></p>
      <div class="row">
        <input id="plateInput" type="text" maxlength="15" placeholder="VD: 30A-12345">
        <button onclick="savePlate()">Luu bien so</button>
        <button class="alt" onclick="clearPending()">Xoa UID cho</button>
      </div>
      <p id="saveMsg"></p>
    </div>
    <div class="card" style="margin-top:12px;">
      <h3 style="margin-top:0;">The quet gan nhat</h3>
      <p>UID: <b id="lastUid">--</b></p>
      <p>Bien so: <b id="lastPlate">--</b></p>
      <p>Cong: <b id="gateState">--</b></p>
      <p id="trackWarn" class="muted" style="margin-bottom:0;"></p>
    </div>
    <div class="slotGrid" id="slotGrid"></div>
    <div class="card" style="margin-top:12px;">
      <h3 style="margin-top:0;">Xe dang o trong bai</h3>
      <div id="insideCars" class="insideList"></div>
    </div>
    <div class="card" style="margin-top:12px;">
      <h3 style="margin-top:0;">Danh sach the da luu</h3>
      <p class="muted">Muon doi the thi quet the moi truoc de co UID dang cho. Nut Danh dau vao / ra dung de sua tay khi danh sach the va cam bien IR bi lech.</p>
      <table>
        <thead><tr><th>STT</th><th>UID</th><th>Bien so</th><th>Trang thai</th><th>Thao tac</th></tr></thead>
        <tbody id="cardTable"></tbody>
      </table>
    </div>
  </div>
  <script>
    const el = id => document.getElementById(id);
    function esc(v) {
      return String(v??"").replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;").replace(/'/g,"&#39;");
    }
    async function postForm(url, data) {
      const body = new URLSearchParams();
      Object.keys(data).forEach(k => body.append(k, data[k]));
      const r = await fetch(url, { method:"POST", headers:{"Content-Type":"application/x-www-form-urlencoded"}, body: body.toString() });
      return { ok: r.ok, text: await r.text() };
    }
    async function loadStatus() {
      try {
        const data = await (await fetch("/status")).json();
        el("freeCount").textContent   = data.free;
        el("usedCount").textContent   = data.occupied;
        el("insideCount").textContent = data.inside_count;
        el("ipText").textContent      = data.ip;
        el("distance").textContent    = data.distance < 0 ? "---" : data.distance.toFixed(1) + " cm";
        el("pendingUid").textContent  = data.pending_uid || "Chua co";
        el("lastUid").textContent     = data.last_uid   || "--";
        el("lastPlate").textContent   = data.last_plate || "--";
        el("gateState").textContent   = data.gate_open  ? "DANG MO" : "DANG DONG";
        el("trackWarn").innerHTML = data.tracking_mismatch
          ? `<span class="warn">Canh bao: the trong bai = ${data.inside_count}, cam bien IR = ${data.occupied}. Neu sai, sua tay bang nut Danh dau vao/ra.</span>`
          : `<span class="ok">Danh sach the dang trong bai khop voi cam bien IR.</span>`;
        let sh = "";
        for (let i=0; i<data.slots.length; i++) {
          const f = data.slots[i]===1;
          sh += `<div class="slot ${f?"slot-full":"slot-empty"}"><div>O ${i+1}</div><div>${f?"DA CO XE":"CON TRONG"}</div></div>`;
        }
        el("slotGrid").innerHTML = sh;
        let ih = "";
        for (let c of data.inside_cars)
          ih += `<div class="insideItem"><div class="insidePlate">${esc(c.plate)}</div><div class="muted">UID: ${esc(c.uid)}</div></div>`;
        el("insideCars").innerHTML = ih || '<div class="muted">Chua co xe nao duoc danh dau la dang trong bai.</div>';
        let th = "";
        for (let i=0; i<data.cards.length; i++) {
          const c = data.cards[i], uj = JSON.stringify(c.uid);
          const badge = c.inside ? '<span class="badge badge-in">TRONG BAI</span>' : '<span class="badge badge-out">NGOAI BAI</span>';
          th += `<tr><td>${i+1}</td><td>${esc(c.uid)}</td><td><input id="plateEdit_${i}" type="text" maxlength="15" value="${esc(c.plate)}"></td><td>${badge}</td><td><div class="actions"><button class="small" onclick='updatePlate(${uj},${i})'>Sua bien so</button><button class="small alt" onclick='replaceCard(${uj})'>Doi the</button><button class="small warnBtn" onclick='toggleInside(${uj},${c.inside?"false":"true"})'>${c.inside?"Danh dau ra":"Danh dau vao"}</button></div></td></tr>`;
        }
        el("cardTable").innerHTML = th || '<tr><td colspan="5">Chua co the nao</td></tr>';
      } catch(e) { console.log(e); }
    }
    async function savePlate() {
      const plate = el("plateInput").value.trim();
      if (!plate) { el("saveMsg").innerHTML = '<span class="warn">Hay nhap bien so truoc.</span>'; return; }
      const r = await postForm("/savePlate", { plate });
      el("saveMsg").innerHTML = r.text;
      if (r.ok) el("plateInput").value = "";
      loadStatus();
    }
    async function clearPending() { await fetch("/clearPending"); el("saveMsg").innerHTML=""; loadStatus(); }
    async function updatePlate(uid, idx) {
      const plate = document.getElementById(`plateEdit_${idx}`).value.trim();
      if (!plate) { el("saveMsg").innerHTML = '<span class="warn">Bien so khong duoc rong.</span>'; return; }
      el("saveMsg").innerHTML = (await postForm("/updatePlate", { uid, plate })).text;
      loadStatus();
    }
    async function replaceCard(old_uid) { el("saveMsg").innerHTML = (await postForm("/replaceCard", { old_uid })).text; loadStatus(); }
    async function toggleInside(uid, inside) { el("saveMsg").innerHTML = (await postForm("/setInside", { uid, inside: inside?"1":"0" })).text; loadStatus(); }
    loadStatus();
    setInterval(loadStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

// ============================================================
// JSON BUILDER
// ============================================================
static String buildStatusJSON() {
  int insideCount = countCarsInside();
  String json = "{";
  json += "\"free\":"          + String(freeCount)        + ",";
  json += "\"occupied\":"      + String(10 - freeCount)   + ",";
  json += "\"inside_count\":"  + String(insideCount)      + ",";
  json += "\"tracking_mismatch\":"; json += trackingMismatch() ? "true" : "false"; json += ",";
  json += "\"distance\":"      + String(lastDistance, 1)  + ",";
  json += "\"ip\":\""          + jsonEscape(apIP)         + "\",";
  json += "\"last_uid\":\""    + jsonEscape(lastUID)       + "\",";
  json += "\"last_plate\":\""  + jsonEscape(lastPlate)     + "\",";
  json += "\"pending_uid\":\"" + jsonEscape(pendingUID)    + "\",";
  json += "\"gate_open\":";    json += gateIsOpen ? "true" : "false"; json += ",";

  json += "\"slots\":[";
  for (int i = 0; i < 10; i++) {
    json += slotOccupied[i] ? "1" : "0";
    if (i < 9) json += ",";
  }
  json += "],";

  json += "\"inside_cars\":[";
  bool firstInside = true;
  for (int i = 0; i < cardCount; i++) {
    if (!cards[i].inside) continue;
    if (!firstInside) json += ",";
    firstInside = false;
    json += "{\"uid\":\"" + jsonEscape(cards[i].uid) + "\",\"plate\":\"" + jsonEscape(cards[i].plate) + "\"}";
  }
  json += "],";

  json += "\"cards\":[";
  for (int i = 0; i < cardCount; i++) {
    json += "{\"uid\":\"" + jsonEscape(cards[i].uid) + "\",\"plate\":\"" + jsonEscape(cards[i].plate) + "\",\"inside\":";
    json += cards[i].inside ? "true" : "false";
    json += "}";
    if (i < cardCount - 1) json += ",";
  }
  json += "]}";

  return json;
}

// ============================================================
// ROUTE HANDLERS
// ============================================================
static void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE_INDEX);
}

static void handleStatus() {
  server.send(200, "application/json; charset=utf-8", buildStatusJSON());
}

static void handleSavePlate() {
  if (pendingUID == "") {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Chua co UID nao dang cho.</span>");
    return;
  }
  if (!server.hasArg("plate")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu bien so.</span>");
    return;
  }
  String plate = normalizePlate(server.arg("plate"));
  if (plate.length() < 5) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Bien so qua ngan.</span>");
    return;
  }
  String err = "";
  if (!saveOrUpdateCard(pendingUID, plate, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>");
    return;
  }
  lastUID    = pendingUID;
  lastPlate  = plate;
  pendingUID = "";
  setCardNotice("Da luu bien so", plate, 1800);
  Serial.print("Da luu bien so "); Serial.print(plate);
  Serial.print(" cho UID ");       Serial.println(lastUID);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da luu bien so thanh cong.</span>");
}

static void handleUpdatePlate() {
  if (!server.hasArg("uid") || !server.hasArg("plate")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID hoac bien so.</span>");
    return;
  }
  String uid   = normalizeUID(server.arg("uid"));
  String plate = normalizePlate(server.arg("plate"));
  if (plate.length() < 5) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Bien so qua ngan.</span>");
    return;
  }
  String err = "";
  if (!updateCardPlate(uid, plate, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>");
    return;
  }
  if (lastUID == uid) lastPlate = plate;
  setCardNotice("Da sua bien so", plate, 1800);
  Serial.print("Cap nhat bien so cho UID "); Serial.print(uid);
  Serial.print(" -> "); Serial.println(plate);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da cap nhat bien so.</span>");
}

static void handleReplaceCard() {
  if (!server.hasArg("old_uid")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID the cu.</span>");
    return;
  }
  if (pendingUID == "") {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Hay quet the moi truoc de tao UID dang cho.</span>");
    return;
  }
  String oldUID = normalizeUID(server.arg("old_uid"));
  String newUID = normalizeUID(pendingUID);
  String err    = "";
  if (!replaceCardUID(oldUID, newUID, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>");
    return;
  }
  int idx    = findCardIndexByUID(newUID);
  lastUID    = newUID;
  lastPlate  = (idx >= 0) ? cards[idx].plate : "";
  pendingUID = "";
  setCardNotice("Da doi the", lastPlate, 1800);
  Serial.print("Da doi the "); Serial.print(oldUID);
  Serial.print(" -> "); Serial.println(newUID);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da doi sang the moi.</span>");
}

static void handleSetInside() {
  if (!server.hasArg("uid") || !server.hasArg("inside")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu tham so cap nhat trang thai.</span>");
    return;
  }
  String uid    = normalizeUID(server.arg("uid"));
  bool   inside = (server.arg("inside") == "1");
  String err    = "";
  if (!setInsideState(uid, inside, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>");
    return;
  }
  int idx = findCardIndexByUID(uid);
  if (idx >= 0) {
    lastUID   = uid;
    lastPlate = cards[idx].plate;
    setCardNotice(cards[idx].plate, inside ? "Danh dau vao" : "Danh dau ra", 1800);
  }
  Serial.print("Cap nhat tay trang thai the "); Serial.print(uid);
  Serial.print(" -> "); Serial.println(inside ? "TRONG BAI" : "NGOAI BAI");
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da cap nhat trang thai xe.</span>");
}

static void handleClearPending() {
  pendingUID = "";
  setCardNotice("Da xoa UID cho", gateIsOpen ? "Cong dang mo" : "Cong dang dong", 1200);
  server.send(200, "text/plain; charset=utf-8", "OK");
}

// ============================================================
// PUBLIC API
// ============================================================
void setupWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  apIP = WiFi.softAPIP().toString();

  server.on("/",            handleRoot);
  server.on("/status",      handleStatus);
  server.on("/savePlate",   HTTP_POST, handleSavePlate);
  server.on("/updatePlate", HTTP_POST, handleUpdatePlate);
  server.on("/replaceCard", HTTP_POST, handleReplaceCard);
  server.on("/setInside",   HTTP_POST, handleSetInside);
  server.on("/clearPending",          handleClearPending);
  server.begin();
}

void handleWebClients() {
  server.handleClient();
}
