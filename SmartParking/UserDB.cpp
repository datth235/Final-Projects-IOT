#include "UserDB.h"
#include "Utils.h"
#include "CardDB.h"   // de reset owner khi xoa user
#include <Preferences.h>

UserAccount users[MAX_USERS];
int         userCount = 0;

static Preferences prefs;

// ============================================================
// NVS
// ============================================================
void loadUsersFromNVS() {
  prefs.begin("userdb", false);

  bool initialized = prefs.getBool("init", false);
  if (!initialized) {
    prefs.putBool("init", true);
    prefs.putInt("count", 1);
    prefs.putString("u0", DEFAULT_USER);
    prefs.putString("p0", DEFAULT_PASS);
  }

  userCount = prefs.getInt("count", 0);
  if (userCount < 0)        userCount = 0;
  if (userCount > MAX_USERS) userCount = MAX_USERS;

  for (int i = 0; i < userCount; i++) {
    String ku = "u" + String(i);
    String kp = "p" + String(i);
    users[i].username = normalizeUsername(prefs.getString(ku.c_str(), ""));
    users[i].password = prefs.getString(kp.c_str(), "");
  }

  prefs.end();
}

void saveUsersToNVS() {
  prefs.begin("userdb", false);
  prefs.putBool("init", true);
  prefs.putInt("count", userCount);

  for (int i = 0; i < MAX_USERS; i++) {
    String ku = "u" + String(i);
    String kp = "p" + String(i);
    if (i < userCount) {
      prefs.putString(ku.c_str(), users[i].username);
      prefs.putString(kp.c_str(), users[i].password);
    } else {
      prefs.remove(ku.c_str());
      prefs.remove(kp.c_str());
    }
  }

  prefs.end();
}

// ============================================================
// VALIDATION
// ============================================================
bool isValidUsername(const String& username) {
  if (username.length() < 3 || username.length() > 16) return false;
  if (username == ADMIN_USER) return false;
  for (int i = 0; i < (int)username.length(); i++) {
    char c = username.charAt(i);
    bool ok = (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-';
    if (!ok) return false;
  }
  return true;
}

bool isValidPassword(const String& password) {
  if (password.length() < 4 || password.length() > 20) return false;
  for (int i = 0; i < (int)password.length(); i++) {
    char c = password.charAt(i);
    if (c == ';' || c == '=' || c == ' ') return false;
  }
  return true;
}

// ============================================================
// TIM KIEM
// ============================================================
int findUserIndexByUsername(const String& usernameRaw) {
  String username = normalizeUsername(usernameRaw);
  for (int i = 0; i < userCount; i++)
    if (users[i].username == username) return i;
  return -1;
}

int findUserIndexByCredentials(const String& usernameRaw, const String& passwordRaw) {
  String username = normalizeUsername(usernameRaw);
  String password = passwordRaw;
  password.trim();
  for (int i = 0; i < userCount; i++)
    if (users[i].username == username && users[i].password == password) return i;
  return -1;
}

// ============================================================
// CRUD
// ============================================================
bool createUserAccount(const String& usernameRaw, const String& passwordRaw, String& err) {
  String username = normalizeUsername(usernameRaw);
  String password = passwordRaw;
  password.trim();

  if (!isValidUsername(username)) {
    err = "Tai khoan chi dung a-z, 0-9, _ hoac -, dai 3-16 ky tu va khong duoc la admin.";
    return false;
  }
  if (!isValidPassword(password)) {
    err = "Mat khau dai 4-20 ky tu, khong dung dau cach, dau ; hoac =.";
    return false;
  }
  if (findUserIndexByUsername(username) >= 0) {
    err = "Tai khoan user nay da ton tai.";
    return false;
  }
  if (userCount >= MAX_USERS) {
    err = "Da dat gioi han so tai khoan user.";
    return false;
  }

  users[userCount].username = username;
  users[userCount].password = password;
  userCount++;
  saveUsersToNVS();
  return true;
}

bool deleteUserAccountByName(const String& usernameRaw, String& err) {
  String username = normalizeUsername(usernameRaw);
  int idx = findUserIndexByUsername(username);

  if (idx < 0) {
    err = "Khong tim thay tai khoan user.";
    return false;
  }

  for (int i = idx; i < userCount - 1; i++)
    users[i] = users[i + 1];
  userCount--;
  saveUsersToNVS();

  // Reset owner o mang cards cho tat ca the da gan user nay
  resetCardOwner(username);
  return true;
}

// ============================================================
// TOKEN
// ============================================================
String makeUserToken(const String& usernameRaw) {
  return String(USER_TOKEN_PREFIX) + normalizeUsername(usernameRaw);
}

String usernameFromUserToken(const String& token) {
  String prefix = USER_TOKEN_PREFIX;
  if (!token.startsWith(prefix)) return "";
  return token.substring(prefix.length());
}
