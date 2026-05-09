#pragma once
#include "Config.h"

// Mang the va so luong hien tai
extern CardInfo cards[MAX_CARDS];
extern int      cardCount;

// Nap du lieu tu NVS khi khoi dong
void loadCardsFromNVS();

// Luu toan bo mang cards[] vao NVS
void saveCardsToNVS();

// Tim the theo UID, tra ve index hoac -1
int findCardIndexByUID(const String& uid);

// Tim the theo bien so, tra ve index hoac -1
int findCardIndexByPlate(const String& plate);

// Dem xe dang duoc danh dau la trong bai
int countCarsInside();

// Kiem tra lenh giua the va cam bien IR
bool trackingMismatch();

// Tao ban ghi moi hoac cap nhat bien so (tra ve false neu loi)
bool saveOrUpdateCard(const String& uid,
                      const String& plate,
                      String& err);

// Sua bien so cho mot UID cu the
bool updateCardPlate(const String& uid,
                     const String& plate,
                     String& err);

// Doi UID the cu sang UID the moi
bool replaceCardUID(const String& oldUID,
                    const String& newUID,
                    String& err);

// Dat trang thai trong / ngoai bai cho mot the
bool setInsideState(const String& uid,
                    bool inside,
                    String& err);
