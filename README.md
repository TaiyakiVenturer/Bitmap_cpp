# Bitmap Image Processing Library

簡單的 BMP 圖檔處理程式庫，用來配合 Visual Studio C++/CLI專案使用。

## 功能
- 讀取/寫入 BMP 檔案
- 支援 24/32 位元色彩
- 基本影像處理

## 使用方式
```cpp
#include "Bitmap_cpp.hpp"

// 讀取圖片
Bitmap_cpp image("Bitmap FilePath");

// 或者
Bitmap_cpp image;
image.LoadBmp("Bitmap FilePath");
```