#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#pragma pack(push, 1)
struct pixel
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;

    pixel() : b(0), g(0), r(0), a(255) {}
};

struct bmp_header
{
    char signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
};

struct bmp_info_header {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};
#pragma pack(pop)

class Bitmap_cpp
{
public:
    bmp_header header;
    bmp_info_header info_header;
    vector<vector<pixel>> data;

    Bitmap_cpp() = default;
    ~Bitmap_cpp();
    Bitmap_cpp(string file_path);
    void LoadBmp(string file_path);

    bool empty() const { return data.empty(); }
    void Resize(int width, int height, float ori_x = 0.0, float ori_y = 0.0);
    void TurnGray();
    void InvertColor();
    void mix_with(const Bitmap_cpp& other, const double& ratio = 0.5);
    void ZoomIn_FirstOrder(int scale = 2);

    // Operators
    Bitmap_cpp operator+(const Bitmap_cpp& other);
    Bitmap_cpp operator-(const Bitmap_cpp& other);
    Bitmap_cpp operator*(const int& scaler);
    Bitmap_cpp operator/(const int& scaler);

    // Bitwise operators
    void and_with(const Bitmap_cpp& other);
    void or_with(const Bitmap_cpp& other);
    void xor_with(const Bitmap_cpp& other);

    #ifdef __cplusplus_cli
    Bitmap_cpp(System::String^ file_path);
    void LoadBmp(System::String^ file_path);
    operator System::Drawing::Bitmap^();
    System::Drawing::Bitmap^ toBitmap();
    #endif
};

void Bitmap_cpp::LoadBmp(string file_path)
{
    ifstream file(file_path, ios::binary);
    if (!file.is_open())
        throw runtime_error("Error: file not found");

    file.read(reinterpret_cast<char*>(&header), sizeof(bmp_header));
    if (header.signature[0] != 'B' || header.signature[1] != 'M')
        throw runtime_error("Error: file is not a Bitmap_cpp file");

    file.read(reinterpret_cast<char*>(&info_header), sizeof(bmp_info_header));

    switch(info_header.bit_count)
    {
        case 24:
        {
            int padding = (4 - (info_header.width * 3) % 4) % 4;
            data.resize(info_header.height);
            vector<unsigned char> row_data(info_header.width * 3);
            
            for (int y = 0; y < info_header.height; y++)
            {
                data[y].resize(info_header.width);
                file.read(reinterpret_cast<char*>(row_data.data()), info_header.width * 3);
                
                for (int x = 0; x < info_header.width; x++)
                {
                    data[y][x].b = row_data[x * 3];
                    data[y][x].g = row_data[x * 3 + 1];
                    data[y][x].r = row_data[x * 3 + 2];
                }
                
                if (padding > 0)
                    file.seekg(padding, ios::cur);
            }
            cout << "It's a 24-bit Bitmap_cpp file" << endl;
            break;
        }
        case 32:
        {
            data.resize(info_header.height);
            for (auto& row : data)
            {
                row.resize(info_header.width);
                file.read(reinterpret_cast<char*>(row.data()), info_header.width * 4);
            }
            cout << "It's a 32-bit Bitmap_cpp file" << endl;
            break;
        }
        default:
        {
            throw runtime_error("Error: unsupported bit count");
        }
    }
    file.close();
}

Bitmap_cpp::Bitmap_cpp(string file_path)
{
    LoadBmp(file_path);
}

Bitmap_cpp::~Bitmap_cpp()
{
    data.clear();
}

void Bitmap_cpp::Resize(int width, int height, float ori_x, float ori_y)
{
    width = min(width, info_header.width);
    height = min(height, info_header.height);
    if (info_header.width * ori_x + width > info_header.width)
        ori_x = 0.0;
    if (info_header.height * ori_y + height > info_header.height)
        ori_y = 0.0;

    int ori_x_ = info_header.width * ori_x;
    int ori_y_ = info_header.height * ori_y;
    vector<vector<pixel>> new_data(height, vector<pixel>(width));
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            new_data[y][x] = data[ori_y_ + y][ori_x_ + x];

    data = new_data;
    info_header.width = width;
    info_header.height = height;
}

void Bitmap_cpp::TurnGray()
{
    for (auto& row : data)
    {
        for (auto& p : row)
        {
            unsigned char gray = (p.r + p.g + p.b) / 3;
            p.r = p.g = p.b = gray;
        }
    }
}

void Bitmap_cpp::InvertColor()
{
    for (auto& row : data)
    {
        for (auto& p : row)
        {
            p.r = 255 - p.r;
            p.g = 255 - p.g;
            p.b = 255 - p.b;
        }
    }
}

void Bitmap_cpp::mix_with(const Bitmap_cpp& other, const double& ratio)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            data[y][x].r = data[y][x].r * (1 - ratio) + other.data[y][x].r * ratio;
            data[y][x].g = data[y][x].g * (1 - ratio) + other.data[y][x].g * ratio;
            data[y][x].b = data[y][x].b * (1 - ratio) + other.data[y][x].b * ratio;
        }
    }
}

void Bitmap_cpp::ZoomIn_FirstOrder(int scale)
{
    vector<vector<pixel>> new_data(info_header.height * scale, vector<pixel>(info_header.width * scale));
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            new_data[x * scale][y * scale] = data[x][y];

    for (int x = 0; x < (info_header.height * scale) - (scale - 1); x += scale)
    {
        for (int y = 0; y < (info_header.width * scale); y++)
        {
            if (x % scale == 0 && y % scale == 0)
                continue;
            if (y >= (info_header.width * scale) - (scale - 1))
            {
                new_data[x][y] = new_data[x][y - 1];
                continue;
            }
            
            int y0 = (y / scale) * scale;
            int y1 = min(y0 + scale, info_header.width * scale - 1);
            int w0 = y - y0;
            int w1 = y1 - y;

            new_data[x][y].r = (w1 * new_data[x][y0].r + w0 * new_data[x][y1].r) / (w0 + w1);
            new_data[x][y].g = (w1 * new_data[x][y0].g + w0 * new_data[x][y1].g) / (w0 + w1);
            new_data[x][y].b = (w1 * new_data[x][y0].b + w0 * new_data[x][y1].b) / (w0 + w1);
        }
    }

    for (int x = 0; x < (info_header.height * scale); x++)
    {
        for (int y = 0; y < (info_header.width * scale); y++)
        {
            if (x % scale == 0)
                continue;
            if (x >= (info_header.height * scale) - (scale - 1))
            {
                new_data[x][y] = new_data[x - 1][y];
                continue;
            }

            int x0 = (x / scale) * scale;
            int x1 = min(x0 + scale, info_header.height * scale - 1);
            int w0 = x - x0;
            int w1 = x1 - x;

            new_data[x][y].r = (w1 * new_data[x0][y].r + w0 * new_data[x1][y].r) / (w0 + w1);
            new_data[x][y].g = (w1 * new_data[x0][y].g + w0 * new_data[x1][y].g) / (w0 + w1);
            new_data[x][y].b = (w1 * new_data[x0][y].b + w0 * new_data[x1][y].b) / (w0 + w1);
        }
    }
    data = new_data;
    info_header.width *= scale;
    info_header.height *= scale;
}

Bitmap_cpp Bitmap_cpp::operator+(const Bitmap_cpp& other)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    Bitmap_cpp result = *this;
    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            result.data[y][x].r = min(255, data[y][x].r + other.data[y][x].r);
            result.data[y][x].g = min(255, data[y][x].g + other.data[y][x].g);
            result.data[y][x].b = min(255, data[y][x].b + other.data[y][x].b);
        }
    }
    return result;
}

Bitmap_cpp Bitmap_cpp::operator-(const Bitmap_cpp& other)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    Bitmap_cpp result = *this;
    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            result.data[y][x].r = max(0, data[y][x].r - other.data[y][x].r);
            result.data[y][x].g = max(0, data[y][x].g - other.data[y][x].g);
            result.data[y][x].b = max(0, data[y][x].b - other.data[y][x].b);
        }
    }
    return result;
}

Bitmap_cpp Bitmap_cpp::operator*(const int& scaler)
{
    Bitmap_cpp result = *this;
    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            result.data[y][x].r = min(255, max(0, data[y][x].r * scaler));
            result.data[y][x].g = min(255, max(0, data[y][x].g * scaler));
            result.data[y][x].b = min(255, max(0, data[y][x].b * scaler));
        }
    }
    return result;
}

Bitmap_cpp Bitmap_cpp::operator/(const int& scaler)
{
    Bitmap_cpp result = *this;
    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            result.data[y][x].r = min(255, max(0, data[y][x].r / scaler));
            result.data[y][x].g = min(255, max(0, data[y][x].g / scaler));
            result.data[y][x].b = min(255, max(0, data[y][x].b / scaler));
        }
    }
    return result;
}

void Bitmap_cpp::and_with(const Bitmap_cpp& other)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            data[y][x].r &= other.data[y][x].r;
            data[y][x].g &= other.data[y][x].g;
            data[y][x].b &= other.data[y][x].b;
        }
    }
}

void Bitmap_cpp::or_with(const Bitmap_cpp& other)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            data[y][x].r |= other.data[y][x].r;
            data[y][x].g |= other.data[y][x].g;
            data[y][x].b |= other.data[y][x].b;
        }
    }
}

void Bitmap_cpp::xor_with(const Bitmap_cpp& other)
{
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int y = 0; y < info_header.height; y++)
    {
        for (int x = 0; x < info_header.width; x++)
        {
            data[y][x].r ^= other.data[y][x].r;
            data[y][x].g ^= other.data[y][x].g;
            data[y][x].b ^= other.data[y][x].b;
        }
    }
}

#ifdef __cplusplus_cli
#include <msclr/marshal_cppstd.h>
void Bitmap_cpp::LoadBmp(System::String^ file_path)
{
    // 將 managed String 轉換為 std::string
    string file_path_std = msclr::interop::marshal_as<string>(file_path);

    // 使用標準函數
    LoadBmp(file_path_std);
}

Bitmap_cpp::Bitmap_cpp(System::String^ file_path)
{
    LoadBmp(file_path);
}

Bitmap_cpp::operator System::Drawing::Bitmap^() 
{
    return toBitmap();
}

// 定義轉換運算子
System::Drawing::Bitmap^ Bitmap_cpp::toBitmap()
{
    // 準備數據
    int stride = ((info_header.width * 3 + 3) / 4) * 4;
    vector<unsigned char> raw_data(stride * info_header.height);
    
    // 排列像素數據
    for (int y = 0; y < info_header.height; y++) 
    {
        for (int x = 0; x < info_header.width; x++) 
        {
            int dest_row = info_header.height - 1 - y;
            int dest_index = dest_row * stride + x * 3;
            
            raw_data[dest_index] = data[y][x].b;
            raw_data[dest_index + 1] = data[y][x].g;
            raw_data[dest_index + 2] = data[y][x].r;
        }
    }

    // 創建 Bitmap 並複製數據
    System::Drawing::Bitmap^ bitmap = gcnew System::Drawing::Bitmap(
        info_header.width,
        info_header.height,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb
    );

    System::Drawing::Imaging::BitmapData^ bitmapData = bitmap->LockBits(
        System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
        System::Drawing::Imaging::ImageLockMode::WriteOnly,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb
    );

    // 將 vector 轉換為 managed array
    cli::array<System::Byte>^ managedArray = gcnew cli::array<System::Byte>(raw_data.size());
    for (int i = 0; i < raw_data.size(); i++)
        managedArray[i] = raw_data[i];

    // 使用正確的 Marshal::Copy 重載
    System::Runtime::InteropServices::Marshal::Copy(
        managedArray,
        0,
        bitmapData->Scan0,
        managedArray->Length
    );

    bitmap->UnlockBits(bitmapData);
    return bitmap;
}
#endif