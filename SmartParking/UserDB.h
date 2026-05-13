#pragma once
#include "Config.h"

extern UserAccount users[MAX_USERS];
extern int         userCount;

// NVS
void loadUsersFromNVS();
void saveUsersToNVS();

// Validation
bool isValidUsername(const String& username);
bool isValidPassword(const String& password);

// Tim kiem
int  findUserIndexByUsername(const String& usernameRaw);
int  findUserIndexByCredentials(const String& usernameRaw, const String& passwordRaw);

// CRUD
bool createUserAccount(const String& usernameRaw, const String& passwordRaw, String& err);
bool deleteUserAccountByName(const String& usernameRaw, String& err);

// Token
String makeUserToken(const String& usernameRaw);
String usernameFromUserToken(const String& token);
