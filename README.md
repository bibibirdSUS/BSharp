# BSharp (B#) 程式語言

BSharp 是一種簡潔、優雅且功能強大的腳本語言。它採用縮排式語法（Indentation-based），旨在提供極佳的可讀性與直觀的編程體驗。

## 特色

- **縮排式區塊結構**：不使用 `{}`，透過縮排定義程式碼區塊。
- **強大的比較運算子鏈結**：支援如 `a < b < c` 的表達方式。
- **動態步長 For 迴圈**：步長可以是表達式，在每次迭代時動態計算。
- **自動作用域管理**：變數生命週期自動管理，支援巢狀作用域。
- **內建函式庫**：提供基本的輸入輸出與數學運算功能。

## 安裝與編譯

本專案使用 CMake 作為構建系統。

### 前置需求

- C++20 編譯器 (如 GCC, Clang 或 MSVC)
- CMake 3.20 或更高版本

### 編譯步驟

1. 複製此專案
2. 建立構建目錄並編譯：

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

編譯完成後，你可以在 `build` 目錄下找到 `BSharp.exe`。

## 使用方法

### 執行腳本

```bash
./BSharp.exe your_script.bs
```

### 進入 REPL 模式

```bash
./BSharp.exe
```

在 REPL 模式下，輸入空行即可執行當前輸入的內容。輸入 `exit` 退出。

## 語法範例

### 變數與運算

```bsharp
x = 10
y = 20
print(x + y * 2)

# 比較運算子鏈結
if 10 < x <= 20 do
    print("x is between 10 and 20")
```

### 迴圈

```bsharp
# For 迴圈
for i = 0 ~ 10 do
    print(i)

# While 迴圈
i = 0
while i < 5 do
    print(i)
    i += 1
```

### 函式

```bsharp
fn add(a, b) = a + b

fn fib(n) do
    if n <= 1 do
        return n
    return fib(n - 1) + fib(n - 2)

print(fib(10))
```

## 說明文件

更多詳細資訊請參考 [Wiki](wiki/index.html)。
