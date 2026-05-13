#include "WebUI.h"
#include "Config.h"
#include "Utils.h"
#include "CardDB.h"
#include "UserDB.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include "RFIDHandler.h"
#include <WiFi.h>
#include <WebServer.h>

String apIP = "192.168.4.1";

static WebServer server(80);

// ============================================================
// HTML - TRANG DANG NHAP
// ============================================================
static const char PAGE_LOGIN[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Dang nhap bai do xe</title>
  <style>
    body{font-family:Arial,sans-serif;background:#f4f6f9;margin:0;color:#1f2937;}
    .loginBox{max-width:420px;margin:60px auto;background:white;border-radius:16px;padding:24px;box-shadow:0 2px 14px rgba(0,0,0,.12);}
    h1{font-size:24px;margin:0 0 8px;color:#0f172a;}
    .muted{color:#64748b;font-size:14px;line-height:1.5;}
    label{display:block;margin-top:14px;font-weight:700;}
    input{width:100%;box-sizing:border-box;padding:11px 12px;border:1px solid #cbd5e1;border-radius:10px;font-size:15px;margin-top:6px;}
    button{width:100%;margin-top:18px;padding:11px 12px;border:none;border-radius:10px;background:#2563eb;color:white;font-weight:700;font-size:15px;cursor:pointer;}
    .hint{background:#eff6ff;border:1px solid #bfdbfe;border-radius:12px;padding:12px;margin-top:16px;font-size:14px;line-height:1.6;}
  </style>
</head>
<body>
  <div class="loginBox">
    <h1>Dang nhap he thong</h1>
    <div class="muted">Chon tai khoan phu hop de truy cap web quan ly bai do xe.</div>
    <form method="POST" action="/login">
      <label>Tai khoan</label>
      <input name="username" type="text" placeholder="admin hoac user" required autofocus>
      <label>Mat khau</label>
      <input name="password" type="password" placeholder="Nhap mat khau" required>
      <button type="submit">Dang nhap</button>
    </form>
    <div class="hint">
      <b>Tai khoan demo:</b><br>
      Admin: <b>admin / admin123</b><br>
      User mac dinh: <b>user / user123</b><br>
      Admin co the tao them tai khoan user tren web.
    </div>
  </div>
</body>
</html>
)rawliteral";

static const char PAGE_LOGIN_FAIL[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Dang nhap that bai</title>
  <style>
    body{font-family:Arial,sans-serif;background:#f4f6f9;margin:0;color:#1f2937;}
    .loginBox{max-width:420px;margin:60px auto;background:white;border-radius:16px;padding:24px;box-shadow:0 2px 14px rgba(0,0,0,.12);}
    h1{font-size:24px;margin:0 0 8px;color:#0f172a;}
    .err{color:#dc2626;font-weight:700;margin:12px 0;}
    a{display:inline-block;margin-top:12px;color:#2563eb;font-weight:700;text-decoration:none;}
  </style>
</head>
<body>
  <div class="loginBox">
    <h1>Dang nhap that bai</h1>
    <div class="err">Sai tai khoan hoac mat khau.</div>
    <a href="/">Quay lai trang dang nhap</a>
  </div>
</body>
</html>
)rawliteral";

// ============================================================
// HTML - TRANG CHINH
// ============================================================
static const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Bai do xe thong minh</title>
  <style>
    body{font-family:Arial,sans-serif;background:#f4f6f9;margin:0;color:#1f2937;}
    .wrap{max-width:1250px;margin:16px auto;padding:12px;}
    .title{font-size:28px;font-weight:700;margin-bottom:8px;}
    .topbar{display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap;background:#fff;border-radius:14px;padding:10px 14px;margin-bottom:12px;box-shadow:0 2px 10px rgba(0,0,0,0.06);}
    .logoutLink{color:#2563eb;font-weight:700;text-decoration:none;}
    .hidden{display:none!important;}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px;}
    .slotGrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px;margin-top:12px;}
    .card{background:#fff;border-radius:14px;padding:14px;box-shadow:0 2px 10px rgba(0,0,0,0.08);}
    .big{font-size:26px;font-weight:700;margin-top:6px;}
    .slot{border-radius:14px;padding:14px;text-align:center;font-weight:700;border:2px solid #ddd;background:#fff;}
    .slot-empty{background:#eafaf0;border-color:#27ae60;color:#1f7a43;}
    .slot-full{background:#fdecec;border-color:#e74c3c;color:#b63125;}
    .row{display:flex;gap:8px;flex-wrap:wrap;align-items:center;}
    .actions{display:flex;gap:6px;flex-wrap:wrap;}
    input[type=text],input[type=number],input[type=password],select{padding:9px 10px;font-size:14px;border:1px solid #cbd5e1;border-radius:10px;min-width:120px;box-sizing:border-box;}
    button{padding:9px 12px;border:none;border-radius:10px;background:#2563eb;color:#fff;font-weight:700;cursor:pointer;}
    button.alt{background:#475569;}
    button.warnBtn{background:#d97706;}
    button.dangerBtn{background:#dc2626;}
    table{width:100%;border-collapse:collapse;margin-top:10px;}
    th,td{padding:8px;border-bottom:1px solid #e5e7eb;text-align:left;font-size:14px;vertical-align:top;}
    .muted{color:#64748b;}
    .ok{color:#059669;font-weight:700;}
    .warn{color:#d97706;font-weight:700;}
    .err{color:#dc2626;font-weight:700;}
    .badge{display:inline-block;padding:4px 8px;border-radius:999px;font-size:12px;font-weight:700;}
    .badge-in{background:#dcfce7;color:#166534;}
    .badge-out{background:#e2e8f0;color:#334155;}
    .insideList{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:10px;}
    .insideItem{border:1px solid #dbeafe;background:#f8fbff;border-radius:12px;padding:10px;}
    .insidePlate{font-size:18px;font-weight:700;}
    .small{padding:7px 10px;font-size:12px;}
    .mono{font-family:Consolas,monospace;word-break:break-all;}
    .money{font-weight:700;color:#0f766e;}
    .tableWrap{overflow-x:auto;}
    .parkingMapWrap{overflow-x:auto;}
    .parkingMap{display:grid;grid-template-columns:repeat(7,72px);grid-template-rows:repeat(5,72px);gap:8px;margin-top:12px;min-width:max-content;}
    .mapCell{border-radius:14px;display:flex;align-items:center;justify-content:center;text-align:center;font-weight:700;font-size:16px;border:2px solid #d1d5db;user-select:none;line-height:1.2;}
    .mapEmpty{background:#eafaf0;border-color:#27ae60;color:#166534;}
    .mapFull{background:#fdecec;border-color:#e74c3c;color:#991b1b;}
    .mapRoad{background:#e5e7eb;border-style:dashed;color:#475569;font-size:13px;}
    .legendRow{display:flex;gap:14px;flex-wrap:wrap;margin-top:12px;}
    .legendItem{display:flex;align-items:center;gap:7px;font-size:14px;}
    .legendBox{width:18px;height:18px;border-radius:6px;border:2px solid #cbd5e1;}
    .legendEmpty{background:#eafaf0;border-color:#27ae60;}
    .legendFull{background:#fdecec;border-color:#e74c3c;}
    .legendRoad{background:#e5e7eb;border-style:dashed;}
  </style>
</head>
<body>
  <div class="wrap">
    <div class="title">Bai do xe thong minh</div>
    <div class="topbar">
      <div>Dang nhap: <b id="roleName">--</b></div>
      <a class="logoutLink" href="/logout">Dang xuat</a>
    </div>

    <div class="grid">
      <div class="card"><div>O trong</div><div class="big" id="freeCount">--</div></div>
      <div class="card"><div>O co xe</div><div class="big" id="usedCount">--</div></div>
      <div class="card"><div>Xe trong bai</div><div class="big" id="insideCount">--</div></div>
      <div class="card"><div>IP</div><div class="big" id="ipText">192.168.4.1</div></div>
      <div class="card adminOnly"><div>Phi moi lan ra</div><div class="big" id="exitFee">20.000d</div></div>
    </div>

    <div class="card adminOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">Dang ky the moi</h3>
      <p>UID dang cho: <span id="pendingUid" class="warn">Chua co</span></p>
      <div class="row">
        <input id="plateInput" type="text" maxlength="15" placeholder="VD: 30A-12345">
        <select id="ownerInput"><option value="">Chua gan chu xe</option></select>
        <button type="button" onclick="savePlate(); return false;">Luu bien so + gan user</button>
        <button type="button" class="alt" onclick="clearPending(); return false;">Xoa UID cho</button>
      </div>
      <p id="saveMsg"></p>
    </div>

    <div class="card adminOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">The quet gan nhat</h3>
      <p>UID: <b id="lastUid" class="mono">--</b></p>
      <p>Bien so: <b id="lastPlate">--</b></p>
      <p>So du: <b id="lastBalance">--</b></p>
      <p>Cong: <b id="gateState">--</b></p>
      <p id="trackWarn" class="muted" style="margin-bottom:0;"></p>
    </div>

    <div class="card" style="margin-top:12px;">
      <h3 style="margin-top:0;">Ban do bai do xe</h3>
      <div class="parkingMapWrap">
        <div id="parkingMap" class="parkingMap"></div>
      </div>
      <div class="legendRow">
        <div class="legendItem"><div class="legendBox legendEmpty"></div><span>Con trong</span></div>
        <div class="legendItem"><div class="legendBox legendFull"></div><span>Da co xe</span></div>
        <div class="legendItem"><div class="legendBox legendRoad"></div><span>Loi di</span></div>
      </div>
    </div>

    <div class="card userOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">Xe / the cua tai khoan toi</h3>
      <p class="muted">Chi hien thi the va bien so da duoc admin gan cho tai khoan dang nhap.</p>
      <div class="tableWrap">
        <table>
          <thead><tr><th>STT</th><th>UID</th><th>Bien so</th><th>So du</th><th>Trang thai</th></tr></thead>
          <tbody id="myCardTable"></tbody>
        </table>
      </div>
    </div>

    <div class="card adminOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">Xe dang o trong bai</h3>
      <div id="insideCars" class="insideList"></div>
    </div>

    <div class="card tableWrap adminOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">Quan ly tai khoan nguoi dung</h3>
      <p class="muted">Admin tao tai khoan user rieng. User chi xem trang thai va ban do bai xe.</p>
      <div class="row">
        <input id="newUsername" type="text" maxlength="16" placeholder="Tai khoan user">
        <input id="newPassword" type="password" maxlength="20" placeholder="Mat khau">
        <button type="button" onclick="createUserAccount(); return false;">Tao tai khoan</button>
      </div>
      <p id="userMsg"></p>
      <table>
        <thead><tr><th>STT</th><th>Tai khoan</th><th>Quyen</th><th>Thao tac</th></tr></thead>
        <tbody id="userTable"></tbody>
      </table>
    </div>

    <div class="card tableWrap adminOnly" style="margin-top:12px;">
      <h3 style="margin-top:0;">Danh sach the da luu</h3>
      <p class="muted">
        - Nap tien: nhap so tien VND roi bam nap.<br>
        - Moi lan xe ra se tru 20.000 VND.<br>
        - Neu so du khong du 20.000 VND thi khong mo cong ra.<br>
        - Muon doi the thi quet the moi truoc de co UID dang cho.
      </p>
      <table>
        <thead>
          <tr>
            <th>STT</th><th>UID</th><th>Bien so</th>
            <th>Chu xe / tai khoan user</th><th>So du</th>
            <th>Nap tien</th><th>Trang thai</th><th>Thao tac</th>
          </tr>
        </thead>
        <tbody id="cardTable"></tbody>
      </table>
    </div>
  </div>

  <script>
    const el = id => document.getElementById(id);

    function esc(v) {
      return String(v??"").replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;").replace(/'/g,"&#39;");
    }

    function formatMoney(v) {
      return Number(v||0).toLocaleString("vi-VN") + " d";
    }

    async function postForm(url, data) {
      const body = new URLSearchParams();
      Object.keys(data).forEach(k => body.append(k, data[k]));
      const r = await fetch(url, { method:"POST", headers:{"Content-Type":"application/x-www-form-urlencoded"}, body: body.toString() });
      return { ok: r.ok, text: await r.text() };
    }

    function isEditingCardTable() {
      const a = document.activeElement;
      if (!a || !a.id) return false;
      return a.id.startsWith("moneyEdit_") || a.id.startsWith("plateEdit_") || a.id.startsWith("ownerEdit_") || a.id === "ownerInput";
    }

    function applyRole(role, loginUser) {
      const admin = role === "admin";
      document.querySelectorAll(".adminOnly").forEach(el => el.classList.toggle("hidden", !admin));
      document.querySelectorAll(".userOnly").forEach(el => el.classList.toggle("hidden", admin));
      el("roleName").textContent = admin ? "Admin" : ("Nguoi dung" + (loginUser ? " - " + loginUser : ""));
    }

    function renderUserOptions(users, selected) {
      let h = '<option value="">Chua gan chu xe</option>';
      if (users) for (let u of users)
        h += `<option value="${esc(u.username)}" ${u.username===selected?"selected":""}>${esc(u.username)}</option>`;
      return h;
    }

    function renderParkingMap(slots) {
      const map = el("parkingMap");
      if (!map) return;
      const pos = [
        {slot:6,row:1,col:2},{slot:5,row:1,col:3},{slot:4,row:1,col:4},
        {slot:3,row:1,col:5},{slot:2,row:1,col:6},{slot:1,row:1,col:7},
        {slot:7,row:2,col:1},{slot:8,row:3,col:1},{slot:9,row:4,col:1},{slot:10,row:5,col:1}
      ];
      let h = "";
      for (let r=1;r<=5;r++) for (let c=1;c<=7;c++) {
        const cell = pos.find(p=>p.row===r&&p.col===c);
        if (cell) {
          const occ = slots[cell.slot-1]===1;
          h += `<div class="mapCell ${occ?"mapFull":"mapEmpty"}" title="O ${cell.slot} - ${occ?"Da co xe":"Con trong"}">${occ?"XE<br>":""}O ${cell.slot}</div>`;
        } else {
          h += `<div class="mapCell mapRoad" title="Loi di">LOI DI</div>`;
        }
      }
      map.innerHTML = h;
    }

    async function loadStatus() {
      try {
        const skipCardTable = isEditingCardTable();
        const resp = await fetch("/status");
        if (resp.status === 401) { window.location.href = "/"; return; }
        const data = await resp.json();

        applyRole(data.role, data.login_user);
        el("freeCount").textContent   = data.free;
        el("usedCount").textContent   = data.occupied;
        el("insideCount").textContent = data.inside_count;
        el("ipText").textContent      = data.ip;

        if (data.role === "admin") {
          el("exitFee").textContent     = formatMoney(data.exit_fee);
          el("pendingUid").textContent  = data.pending_uid || "Chua co";
          el("lastUid").textContent     = data.last_uid    || "--";
          el("lastPlate").textContent   = data.last_plate  || "--";
          el("lastBalance").textContent = formatMoney(data.last_balance);
          el("gateState").textContent   = data.gate_open ? "DANG MO" : "DANG DONG";

          el("trackWarn").innerHTML = data.tracking_mismatch
            ? `<span class="warn">Canh bao: the trong bai = ${data.inside_count}, cam bien IR = ${data.occupied}.</span>`
            : `<span class="ok">Danh sach the dang trong bai khop voi cam bien IR.</span>`;

          const ownerInput = el("ownerInput");
          if (ownerInput && document.activeElement !== ownerInput)
            ownerInput.innerHTML = renderUserOptions(data.users, ownerInput.value || "");

          let uh = "";
          if (data.users && data.users.length > 0)
            for (let i=0; i<data.users.length; i++) {
              const u = data.users[i];
              const uj = JSON.stringify(u.username);
              uh += `<tr><td>${i+1}</td><td class="mono">${esc(u.username)}</td><td><span class="badge badge-out">USER</span></td><td><button type="button" class="small dangerBtn" onclick='deleteUserAccount(${uj}); return false;'>Xoa tai khoan</button></td></tr>`;
            }
          el("userTable").innerHTML = uh || '<tr><td colspan="4">Chua co tai khoan user nao.</td></tr>';

          let ih = "";
          for (let c of data.inside_cars)
            ih += `<div class="insideItem"><div class="insidePlate">${esc(c.plate)}</div><div class="muted mono">UID: ${esc(c.uid)}</div><div class="money">So du: ${formatMoney(c.balance)}</div></div>`;
          el("insideCars").innerHTML = ih || '<div class="muted">Chua co xe nao trong bai.</div>';

          if (!skipCardTable) {
            let th = "";
            for (let i=0; i<data.cards.length; i++) {
              const c = data.cards[i];
              const uj = JSON.stringify(c.uid);
              const badge = c.inside ? '<span class="badge badge-in">TRONG BAI</span>' : '<span class="badge badge-out">NGOAI BAI</span>';
              th += `<tr>
                <td>${i+1}</td>
                <td class="mono">${esc(c.uid)}</td>
                <td><input id="plateEdit_${i}" type="text" maxlength="15" value="${esc(c.plate)}"></td>
                <td><select id="ownerEdit_${i}">${renderUserOptions(data.users, c.owner||"")}</select></td>
                <td class="money">${formatMoney(c.balance)}</td>
                <td><div class="row"><input id="moneyEdit_${i}" type="number" min="1000" step="1000" placeholder="VD 50000"><button type="button" class="small" onclick='topUp(${uj},${i}); return false;'>Nap tien</button></div></td>
                <td>${badge}</td>
                <td><div class="actions">
                  <button type="button" class="small" onclick='updatePlate(${uj},${i}); return false;'>Sua bien so/user</button>
                  <button type="button" class="small alt" onclick='replaceCard(${uj}); return false;'>Doi the</button>
                  <button type="button" class="small warnBtn" onclick='toggleInside(${uj},${c.inside?"false":"true"}); return false;'>${c.inside?"Danh dau ra":"Danh dau vao"}</button>
                  <button type="button" class="small dangerBtn" onclick='deleteCard(${uj}); return false;'>Xoa the</button>
                </div></td>
              </tr>`;
            }
            el("cardTable").innerHTML = th || '<tr><td colspan="8">Chua co the nao</td></tr>';
          }
        }

        if (data.role === "user") {
          let mh = "";
          if (data.my_cards && data.my_cards.length > 0)
            for (let i=0; i<data.my_cards.length; i++) {
              const c = data.my_cards[i];
              const badge = c.inside ? '<span class="badge badge-in">TRONG BAI</span>' : '<span class="badge badge-out">NGOAI BAI</span>';
              mh += `<tr><td>${i+1}</td><td class="mono">${esc(c.uid)}</td><td>${esc(c.plate)}</td><td class="money">${formatMoney(c.balance)}</td><td>${badge}</td></tr>`;
            }
          el("myCardTable").innerHTML = mh || '<tr><td colspan="5">Tai khoan nay chua duoc gan the/xe nao.</td></tr>';
        }

        renderParkingMap(data.slots);
      } catch(e) { console.log(e); }
    }

    async function savePlate() {
      const plate = el("plateInput").value.trim();
      if (!plate) { el("saveMsg").innerHTML = '<span class="warn">Hay nhap bien so truoc.</span>'; return; }
      const owner = el("ownerInput") ? el("ownerInput").value : "";
      const r = await postForm("/savePlate", { plate, owner });
      el("saveMsg").innerHTML = r.text;
      if (r.ok) el("plateInput").value = "";
      loadStatus();
    }

    async function clearPending() { await fetch("/clearPending"); el("saveMsg").innerHTML=""; loadStatus(); }

    async function updatePlate(uid, i) {
      const plate = document.getElementById(`plateEdit_${i}`).value.trim();
      if (!plate) { el("saveMsg").innerHTML = '<span class="warn">Bien so khong duoc rong.</span>'; return; }
      const ownerEl = document.getElementById(`ownerEdit_${i}`);
      const owner = ownerEl ? ownerEl.value : "";
      el("saveMsg").innerHTML = (await postForm("/updatePlate", { uid, plate, owner })).text;
      loadStatus();
    }

    async function topUp(uid, i) {
      const amount = document.getElementById(`moneyEdit_${i}`).value.trim();
      if (!amount || Number(amount) <= 0) { el("saveMsg").innerHTML = '<span class="warn">Nhap so tien hop le.</span>'; return; }
      const r = await postForm("/topUp", { uid, amount });
      el("saveMsg").innerHTML = r.text;
      if (r.ok) document.getElementById(`moneyEdit_${i}`).value = "";
      loadStatus();
    }

    async function replaceCard(old_uid) {
      el("saveMsg").innerHTML = (await postForm("/replaceCard", { old_uid })).text;
      loadStatus();
    }

    async function toggleInside(uid, inside) {
      el("saveMsg").innerHTML = (await postForm("/setInside", { uid, inside: inside?"1":"0" })).text;
      loadStatus();
    }

    async function deleteCard(uid) {
      if (!confirm("Xoa the nay khoi he thong?")) return;
      el("saveMsg").innerHTML = (await postForm("/deleteCard", { uid })).text;
      loadStatus();
    }

    async function createUserAccount() {
      const username = el("newUsername").value.trim();
      const password = el("newPassword").value.trim();
      if (!username || !password) { el("userMsg").innerHTML = '<span class="warn">Nhap du tai khoan va mat khau.</span>'; return; }
      const r = await postForm("/createUser", { username, password });
      el("userMsg").innerHTML = r.text;
      if (r.ok) { el("newUsername").value=""; el("newPassword").value=""; }
      loadStatus();
    }

    async function deleteUserAccount(username) {
      if (!confirm("Xoa tai khoan user: " + username + "?")) return;
      el("userMsg").innerHTML = (await postForm("/deleteUser", { username })).text;
      loadStatus();
    }

    loadStatus();
    setInterval(loadStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

// ============================================================
// PHAN QUYEN WEB
// ============================================================
static String getCookieValue(const String& name) {
  String cookie = server.header("Cookie");
  String key    = name + "=";
  int start = cookie.indexOf(key);
  if (start < 0) return "";
  start += key.length();
  int end = cookie.indexOf(';', start);
  if (end < 0) end = cookie.length();
  String value = cookie.substring(start, end);
  value.trim();
  return value;
}

static String currentRole() {
  String token = getCookieValue("sp_token");
  if (token == ADMIN_TOKEN) return "admin";
  String username = usernameFromUserToken(token);
  if (username != "" && findUserIndexByUsername(username) >= 0) return "user";
  return "";
}

static bool isLoggedIn() { return currentRole() != ""; }
static bool isAdmin()    { return currentRole() == "admin"; }

static String currentUsername() {
  String token = getCookieValue("sp_token");
  if (token == ADMIN_TOKEN) return String(ADMIN_USER);
  return usernameFromUserToken(token);
}

static void sendForbidden() {
  server.send(403, "text/html; charset=utf-8",
    "<span class='err'>Tai khoan nay khong co quyen thuc hien thao tac.</span>");
}

// ============================================================
// JSON
// ============================================================
static String buildStatusJSON(bool adminView) {
  int insideCount = countCarsInside();
  String loginName = currentUsername();

  String json = "{";
  json += "\"role\":\"" + String(adminView ? "admin" : "user") + "\",";
  json += "\"login_user\":\"" + jsonEscape(loginName) + "\",";
  json += "\"free\":" + String(freeCount) + ",";
  json += "\"occupied\":" + String(10 - freeCount) + ",";
  json += "\"inside_count\":" + String(insideCount) + ",";
  json += "\"tracking_mismatch\":"; json += trackingMismatch() ? "true" : "false"; json += ",";
  json += "\"distance\":" + String(lastDistance, 1) + ",";
  json += "\"ip\":\"" + jsonEscape(apIP) + "\",";

  if (adminView) {
    json += "\"last_uid\":\""     + jsonEscape(lastUID)   + "\",";
    json += "\"last_plate\":\""   + jsonEscape(lastPlate) + "\",";
    json += "\"last_balance\":"   + String(lastBalance)   + ",";
    json += "\"pending_uid\":\""  + jsonEscape(pendingUID) + "\",";
  } else {
    json += "\"last_uid\":\"\",\"last_plate\":\"\",\"last_balance\":0,\"pending_uid\":\"\",";
  }

  json += "\"gate_open\":"; json += gateIsOpen ? "true" : "false"; json += ",";
  json += "\"exit_fee\":" + String(EXIT_FEE) + ",";

  json += "\"slots\":[";
  for (int i = 0; i < 10; i++) {
    json += slotOccupied[i] ? "1" : "0";
    if (i < 9) json += ",";
  }
  json += "],";

  json += "\"inside_cars\":[";
  if (adminView) {
    bool first = true;
    for (int i = 0; i < cardCount; i++) {
      if (!cards[i].inside) continue;
      if (!first) json += ",";
      first = false;
      json += "{\"uid\":\"" + jsonEscape(cards[i].uid) + "\",\"plate\":\"" + jsonEscape(cards[i].plate) + "\",\"owner\":\"" + jsonEscape(cards[i].owner) + "\",\"balance\":" + String(cards[i].balance) + "}";
    }
  }
  json += "],";

  json += "\"cards\":[";
  if (adminView) {
    for (int i = 0; i < cardCount; i++) {
      json += "{\"uid\":\"" + jsonEscape(cards[i].uid) + "\",\"plate\":\"" + jsonEscape(cards[i].plate) + "\",\"owner\":\"" + jsonEscape(cards[i].owner) + "\",\"inside\":";
      json += cards[i].inside ? "true" : "false";
      json += ",\"balance\":" + String(cards[i].balance) + "}";
      if (i < cardCount - 1) json += ",";
    }
  }
  json += "],";

  json += "\"users\":[";
  if (adminView) {
    for (int i = 0; i < userCount; i++) {
      json += "{\"username\":\"" + jsonEscape(users[i].username) + "\"}";
      if (i < userCount - 1) json += ",";
    }
  }
  json += "],";

  json += "\"my_cards\":[";
  if (!adminView) {
    bool first = true;
    for (int i = 0; i < cardCount; i++) {
      if (cards[i].owner != loginName) continue;
      if (!first) json += ",";
      first = false;
      json += "{\"uid\":\"" + jsonEscape(cards[i].uid) + "\",\"plate\":\"" + jsonEscape(cards[i].plate) + "\",\"inside\":";
      json += cards[i].inside ? "true" : "false";
      json += ",\"balance\":" + String(cards[i].balance) + "}";
    }
  }
  json += "]";

  json += "}";
  return json;
}

// ============================================================
// ROUTE HANDLERS
// ============================================================
static void handleRoot() {
  if (!isLoggedIn()) {
    server.send_P(200, "text/html; charset=utf-8", PAGE_LOGIN);
    return;
  }
  server.send_P(200, "text/html; charset=utf-8", PAGE_INDEX);
}

static void handleLogin() {
  if (!server.hasArg("username") || !server.hasArg("password")) {
    server.send_P(400, "text/html; charset=utf-8", PAGE_LOGIN_FAIL);
    return;
  }
  String username = server.arg("username"); username.trim();
  String password = server.arg("password"); password.trim();

  if (username == ADMIN_USER && password == ADMIN_PASS) {
    server.sendHeader("Set-Cookie", String("sp_token=") + ADMIN_TOKEN + "; Path=/; SameSite=Lax");
    server.sendHeader("Location", "/");
    server.send(303, "text/plain; charset=utf-8", "");
    return;
  }

  String userNorm = normalizeUsername(username);
  if (findUserIndexByCredentials(userNorm, password) >= 0) {
    server.sendHeader("Set-Cookie", String("sp_token=") + makeUserToken(userNorm) + "; Path=/; SameSite=Lax");
    server.sendHeader("Location", "/");
    server.send(303, "text/plain; charset=utf-8", "");
    return;
  }

  server.send_P(401, "text/html; charset=utf-8", PAGE_LOGIN_FAIL);
}

static void handleLogout() {
  server.sendHeader("Set-Cookie", "sp_token=; Path=/; Max-Age=0; SameSite=Lax");
  server.sendHeader("Location", "/");
  server.send(303, "text/plain; charset=utf-8", "");
}

static void handleStatus() {
  if (!isLoggedIn()) {
    server.send(401, "application/json; charset=utf-8", "{\"error\":\"unauthorized\"}");
    return;
  }
  server.send(200, "application/json; charset=utf-8", buildStatusJSON(isAdmin()));
}

static void handleSavePlate() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (pendingUID == "") {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Chua co UID nao dang cho.</span>"); return;
  }
  if (!server.hasArg("plate")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu bien so.</span>"); return;
  }
  String plate = normalizePlate(server.arg("plate"));
  String owner = server.hasArg("owner") ? normalizeUsername(server.arg("owner")) : "";
  if (plate.length() < 5) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Bien so qua ngan.</span>"); return;
  }
  String err = "";
  if (!saveOrUpdateCard(pendingUID, plate, owner, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  int idx = findCardIndexByUID(pendingUID);
  lastUID     = pendingUID;
  lastPlate   = plate;
  lastBalance = (idx >= 0) ? cards[idx].balance : 0;
  pendingUID  = "";
  setCardNotice("Da luu bien so", plate, 1800);
  Serial.print("Da luu bien so "); Serial.print(plate);
  Serial.print(" cho UID "); Serial.println(lastUID);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da luu bien so thanh cong.</span>");
}

static void handleUpdatePlate() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("uid") || !server.hasArg("plate")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID hoac bien so.</span>"); return;
  }
  String uid   = normalizeUID(server.arg("uid"));
  String plate = normalizePlate(server.arg("plate"));
  String owner = server.hasArg("owner") ? normalizeUsername(server.arg("owner")) : "";
  if (plate.length() < 5) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Bien so qua ngan.</span>"); return;
  }
  String err = "";
  if (!updateCardPlate(uid, plate, owner, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  if (lastUID == uid) lastPlate = plate;
  setCardNotice("Da sua bien so", plate, 1800);
  Serial.print("Cap nhat bien so cho UID "); Serial.print(uid); Serial.print(" -> "); Serial.println(plate);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da cap nhat bien so.</span>");
}

static void handleTopUp() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("uid") || !server.hasArg("amount")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID hoac so tien.</span>"); return;
  }
  String uid        = normalizeUID(server.arg("uid"));
  String amountText = server.arg("amount");
  amountText.replace(",", ""); amountText.replace(".", ""); amountText.trim();
  int amount = amountText.toInt();
  String err = "";
  if (!topUpBalance(uid, amount, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  int idx = findCardIndexByUID(uid);
  if (idx >= 0) {
    lastUID     = uid;
    lastPlate   = cards[idx].plate;
    lastBalance = cards[idx].balance;
    setCardNotice(cards[idx].plate, "Nap +" + moneyToText(amount), 1800);
  }
  Serial.print("Nap tien cho UID "); Serial.print(uid); Serial.print(" +"); Serial.println(amount);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Nap tien thanh cong.</span>");
}

static void handleReplaceCard() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("old_uid")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID the cu.</span>"); return;
  }
  if (pendingUID == "") {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Hay quet the moi truoc de tao UID dang cho.</span>"); return;
  }
  String oldUID = normalizeUID(server.arg("old_uid"));
  String newUID = normalizeUID(pendingUID);
  String err = "";
  if (!replaceCardUID(oldUID, newUID, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  int idx     = findCardIndexByUID(newUID);
  lastUID     = newUID;
  lastPlate   = (idx >= 0) ? cards[idx].plate : "";
  lastBalance = (idx >= 0) ? cards[idx].balance : 0;
  pendingUID  = "";
  setCardNotice("Da doi the", lastPlate, 1800);
  Serial.print("Da doi the "); Serial.print(oldUID); Serial.print(" -> "); Serial.println(newUID);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da doi sang the moi.</span>");
}

static void handleSetInside() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("uid") || !server.hasArg("inside")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu tham so cap nhat trang thai.</span>"); return;
  }
  String uid    = normalizeUID(server.arg("uid"));
  bool   inside = (server.arg("inside") == "1");
  String err    = "";
  if (!setInsideState(uid, inside, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  int idx = findCardIndexByUID(uid);
  if (idx >= 0) {
    lastUID     = uid;
    lastPlate   = cards[idx].plate;
    lastBalance = cards[idx].balance;
    setCardNotice(cards[idx].plate, inside ? "Danh dau vao" : "Danh dau ra", 1800);
  }
  Serial.print("Cap nhat tay trang thai the "); Serial.print(uid);
  Serial.print(" -> "); Serial.println(inside ? "TRONG BAI" : "NGOAI BAI");
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da cap nhat trang thai xe.</span>");
}

static void handleDeleteCard() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("uid")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu UID can xoa.</span>"); return;
  }
  String uid = normalizeUID(server.arg("uid"));
  String err = "";
  int idx = findCardIndexByUID(uid);
  String plateBefore = (idx >= 0) ? cards[idx].plate : "";
  if (!deleteCardByUID(uid, err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  if (lastUID == uid)    { lastUID = ""; lastPlate = ""; lastBalance = 0; }
  if (pendingUID == uid) pendingUID = "";
  setCardNotice("Da xoa the", plateBefore, 1800);
  Serial.print("Da xoa the UID "); Serial.println(uid);
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da xoa the khoi he thong.</span>");
}

static void handleCreateUser() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("username") || !server.hasArg("password")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu tai khoan hoac mat khau.</span>"); return;
  }
  String err = "";
  if (!createUserAccount(server.arg("username"), server.arg("password"), err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  Serial.print("Da tao tai khoan user: "); Serial.println(normalizeUsername(server.arg("username")));
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da tao tai khoan user thanh cong.</span>");
}

static void handleDeleteUser() {
  if (!isAdmin()) { sendForbidden(); return; }
  if (!server.hasArg("username")) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>Thieu ten tai khoan can xoa.</span>"); return;
  }
  String err = "";
  if (!deleteUserAccountByName(server.arg("username"), err)) {
    server.send(400, "text/html; charset=utf-8", "<span class='warn'>" + err + "</span>"); return;
  }
  Serial.print("Da xoa tai khoan user: "); Serial.println(normalizeUsername(server.arg("username")));
  server.send(200, "text/html; charset=utf-8", "<span class='ok'>Da xoa tai khoan user.</span>");
}

static void handleClearPending() {
  if (!isAdmin()) { sendForbidden(); return; }
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

  const char* headerKeys[] = {"Cookie"};
  server.collectHeaders(headerKeys, 1);

  server.on("/",            handleRoot);
  server.on("/login",       HTTP_POST, handleLogin);
  server.on("/logout",      handleLogout);
  server.on("/status",      handleStatus);
  server.on("/savePlate",   HTTP_POST, handleSavePlate);
  server.on("/updatePlate", HTTP_POST, handleUpdatePlate);
  server.on("/topUp",       HTTP_POST, handleTopUp);
  server.on("/replaceCard", HTTP_POST, handleReplaceCard);
  server.on("/setInside",   HTTP_POST, handleSetInside);
  server.on("/deleteCard",  HTTP_POST, handleDeleteCard);
  server.on("/createUser",  HTTP_POST, handleCreateUser);
  server.on("/deleteUser",  HTTP_POST, handleDeleteUser);
  server.on("/clearPending",           handleClearPending);
  server.begin();
}

void handleWebClients() {
  server.handleClient();
}
