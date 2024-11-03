# Bitmap Image Processing Library

簡單的 BMP 圖檔處理程式庫，用來配合 Visual Studio C++/CLI 專案使用，
只使用基礎的標準庫，以減少對於外部開發環境的依賴，
但目前輸出接口只對 C++/CLI 開發環境開放，純 C++ 環境只支援直接訪問物件成員，不支援輸出。

## 功能
- 讀取/寫入 BMP 檔案
- 支援 24/32 位元色彩
- 基本影像處理

## 系統需求
- C++11 或以上
- Windows 作業系統

## 使用方法

## 1. 讀取方式
```cpp
#include "Bitmap_cpp.hpp"

// 建構子讀取
Bitmap_cpp image("Bitmap FilePath");

// 使用 LoadBmp 函式
Bitmap_cpp image;
image.LoadBmp("Bitmap FilePath");
```

## 2. 輸出/轉換方式
```cpp
// 使用轉型運算子
Bitmap^ bmp = static_cast<Bitmap^>(image);

// 使用 toBitmap 函式
Bitmap^ bmp = image.toBitmap();
```