#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#pragma pack(push, 1)
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

struct pixel
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;

    pixel() : r(0), g(0), b(0), a(255) {}
    pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
    pixel operator+(const pixel& other);
    pixel operator-(const pixel& other);
    pixel operator*(const int& scaler);
    pixel operator/(const int& scaler);
};
#pragma pack(pop)

pixel::pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    int _r = min(255, max(0, static_cast<int>(r)));
    int _g = min(255, max(0, static_cast<int>(g)));
    int _b = min(255, max(0, static_cast<int>(b)));
    int _a = min(255, max(0, static_cast<int>(a)));

    this->r = _r;
    this->g = _g;
    this->b = _b;
    this->a = _a;
}

pixel pixel::operator+(const pixel& other)
{
    pixel result;
    result.b = min(255, b + other.b);
    result.g = min(255, g + other.g);
    result.r = min(255, r + other.r);
    return result;
}

pixel pixel::operator-(const pixel& other)
{
    pixel result;
    result.b = max(0, b - other.b);
    result.g = max(0, g - other.g);
    result.r = max(0, r - other.r);
    return result;
}

pixel pixel::operator*(const int& scaler)
{
    pixel result;
    result.b = min(255, max(0, b * scaler));
    result.g = min(255, max(0, g * scaler));
    result.r = min(255, max(0, r * scaler));
    return result;
}

pixel pixel::operator/(const int& scaler)
{
    pixel result;
    result.b = min(255, max(0, b / scaler));
    result.g = min(255, max(0, g / scaler));
    result.r = min(255, max(0, r / scaler));
    return result;
}

class Bitmap_cpp
{
public:
    // Data
    bmp_header header;
    bmp_info_header info_header;
    vector<vector<pixel>> data;

    // Constructors
    Bitmap_cpp() = default;
    ~Bitmap_cpp();
    Bitmap_cpp(string file_path);
    void LoadBmp(string file_path);
    void SaveBmp(string file_path);

    // Basic functions
    bool empty() const { return data.empty(); }
    void CheckValid() const;
    void Resize(int width, int height, int start_x = 0, int start_y = 0);
    void toGray();
    void InvertColor();

    // Image processing
    void mix_with(const Bitmap_cpp& other, const double& ratio = 0.5);
    void ZoomIn_ZeroOrder(int scale = 2);
    void ZoomIn_FirstOrder(int scale = 2);
    void ZoomIn_Compare(int scale = 2);
    void ZoomIn_Bilinear(int scale = 2);
    void ZoomOut(int scale = 2);
    void HistogramEqualization_Global();
    void HistogramEqualization_Local(int block_size = 7);

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

Bitmap_cpp::Bitmap_cpp(string file_path)
{
    LoadBmp(file_path);
}

Bitmap_cpp::~Bitmap_cpp()
{
    data.clear();
}

void Bitmap_cpp::LoadBmp(string file_path)
{
    ifstream file(file_path, ios::binary);
    if (!file.is_open())
        throw runtime_error("Error: file not found");

    file.read(reinterpret_cast<char*>(&header), sizeof(bmp_header));
    if (header.signature[0] != 'B' || header.signature[1] != 'M')
        throw runtime_error("Error: file is not a Bitmap file");

    file.read(reinterpret_cast<char*>(&info_header), sizeof(bmp_info_header));
    if (info_header.bit_count != 24 && info_header.bit_count != 32)
        throw runtime_error("Error: unsupported bit count");
    if (info_header.height <= 0 || info_header.width <= 0)
        throw runtime_error("Error: invalid image size");

    switch(info_header.bit_count)
    {
        case 24:
        {
            #ifndef __cplusplus_cli
            cout << "It's a 24-bit Bitmap file" << endl;
            #endif

            int padding = (4 - (info_header.width * 3) % 4) % 4;
            data.resize(info_header.height);
            vector<unsigned char> row_data(info_header.width * 3);
            
            for (int x = 0; x < info_header.height; x++)
            {
                data[x].resize(info_header.width);
                file.read(reinterpret_cast<char*>(row_data.data()), info_header.width * 3);
                for (int y = 0; y < info_header.width; y++)
                {
                    data[x][y].b = row_data[y * 3];
                    data[x][y].g = row_data[y * 3 + 1];
                    data[x][y].r = row_data[y * 3 + 2];
                }

                if (padding > 0)
                    file.seekg(padding, ios::cur);
            }
            break;
        }
        case 32:
        {
            #ifndef __cplusplus_cli
            cout << "It's a 32-bit Bitmap file" << endl;
            #endif

            data.resize(info_header.height);
            for (auto& row : data)
            {
                row.resize(info_header.width);
                file.read(reinterpret_cast<char*>(row.data()), info_header.width * 4);
            }
            break;
        }
        default:
        {
            throw runtime_error("Error: unsupported bit count");
        }
    }
    file.close();
}

void Bitmap_cpp::SaveBmp(string file_path)
{
    CheckValid();
    if (file_path.find(".bmp") == string::npos)
        file_path += ".bmp";

    ofstream file(file_path, ios::binary);
    if (!file.is_open())
        throw runtime_error("Error: Cannot create file");
    
    file.write(reinterpret_cast<char*>(&header), sizeof(bmp_header));
    file.write(reinterpret_cast<char*>(&info_header), sizeof(bmp_info_header));

    switch(info_header.bit_count)
    {
        case 24:
        {
            int padding = (4 - (info_header.width * 3) % 4) % 4;
            vector<unsigned char> row_data(info_header.width * 3);
            vector<unsigned char> padding_buffer(padding, 0);
            for (int x = 0; x < info_header.height; x++)
            {
                for (int y = 0; y < info_header.width; y++)
                {
                    row_data[y * 3] = data[x][y].b;
                    row_data[y * 3 + 1] = data[x][y].g;
                    row_data[y * 3 + 2] = data[x][y].r;
                }
                file.write(reinterpret_cast<char*>(row_data.data()), info_header.width * 3);

                if (padding > 0)
                    file.write(reinterpret_cast<char*>(padding_buffer.data()), padding);
            }
            if (!file.good())
                throw runtime_error("Error: failed to write 24-bit Bitmap file");
            break;
        }
        case 32:
        {
            for (auto& row : data)
                file.write(reinterpret_cast<char*>(row.data()), info_header.width * 4);
            if (!file.good())
                throw runtime_error("Error: failed to write 32-bit Bitmap file");
            break;
        }
        default:
        {
            throw runtime_error("Error: unsupported bit count");
        }
    }
    file.close();
    if (!file.good())
        throw runtime_error("Error: file write error");
}

void Bitmap_cpp::CheckValid() const
{
    if (data.empty())
        throw runtime_error("Error: image data is empty");
    if (info_header.width <= 0 || info_header.height <= 0)
        throw runtime_error("Error: invalid image size");
}

void Bitmap_cpp::Resize(int width, int height, int start_y, int start_x)
{
    CheckValid();
    width = min(width, info_header.width);
    height = min(height, info_header.height);

    start_x = info_header.height - start_x;
    if (start_x + height > info_header.height)
        start_x = max(0, info_header.height - height);
    if (start_y + width > info_header.width)
        start_y = max(0, info_header.width - width);
    vector<vector<pixel>> new_data(height, vector<pixel>(width));
    for (int x = 0; x < height; x++)
        for (int y = 0; y < width; y++)
            new_data[x][y] = data[start_x + x][start_y + y];

    data = new_data;
    info_header.width = width;
    info_header.height = height;
}

void Bitmap_cpp::toGray()
{
    CheckValid();
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
    CheckValid();
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
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int x = 0; x < info_header.height; x++)
    {
        for (int y = 0; y < info_header.width; y++)
        {
            data[x][y].r = data[x][y].r * (1 - ratio) + other.data[x][y].r * ratio;
            data[x][y].g = data[x][y].g * (1 - ratio) + other.data[x][y].g * ratio;
            data[x][y].b = data[x][y].b * (1 - ratio) + other.data[x][y].b * ratio;
        }
    }
}

void Bitmap_cpp::ZoomIn_ZeroOrder(int scale)
{
    CheckValid();
    vector<vector<pixel>> new_data(info_header.height * scale, vector<pixel>(info_header.width * scale));
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            new_data[x * scale][y * scale] = data[x][y];

    for (int x = 0; x < (info_header.height * scale); x++)
    {
        for (int y = 0; y < (info_header.width * scale); y++)
        {
            if (x % scale == 0 && y % scale == 0)
                continue;
            
            new_data[x][y] = data[x / scale][y / scale];
        }
    }
    data = new_data;
    info_header.width *= scale;
    info_header.height *= scale;
}

void Bitmap_cpp::ZoomIn_FirstOrder(int scale)
{
    CheckValid();
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
            int w = w0 + w1;

            new_data[x][y] = pixel(
                (w1 * new_data[x][y0].r + w0 * new_data[x][y1].r) / w, 
                (w1 * new_data[x][y0].g + w0 * new_data[x][y1].g) / w, 
                (w1 * new_data[x][y0].b + w0 * new_data[x][y1].b) / w);
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
            int w = w0 + w1;

            new_data[x][y] = pixel(
                (w1 * new_data[x0][y].r + w0 * new_data[x1][y].r) / w, 
                (w1 * new_data[x0][y].g + w0 * new_data[x1][y].g) / w, 
                (w1 * new_data[x0][y].b + w0 * new_data[x1][y].b) / w);
        }
    }
    data = new_data;
    info_header.width *= scale;
    info_header.height *= scale;
}

void Bitmap_cpp::ZoomIn_Compare(int scale)
{
    CheckValid();
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
            int w = w0 + w1;

            new_data[x][y] = pixel(
                (w1 * new_data[x][y0].r + w0 * new_data[x][y1].r) / w, 
                (w1 * new_data[x][y0].g + w0 * new_data[x][y1].g) / w, 
                (w1 * new_data[x][y0].b + w0 * new_data[x][y1].b) / w);
        }
    }
    
    for (int x = 0; x < (info_header.height * scale); x++)
    {
        if (x % scale == 0)
            continue;
        for (int y = 0; y < (info_header.width * scale); y++)
            new_data[x][y] = new_data[x - 1][y];
    }
    data = new_data;
    info_header.width *= scale;
    info_header.height *= scale;
}

void Bitmap_cpp::ZoomIn_Bilinear(int scale)
{
    CheckValid();
    vector<vector<pixel>> new_data(info_header.height * scale, vector<pixel>(info_header.width * scale));
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            new_data[x * scale][y * scale] = data[x][y];
    
    for (int x = 0; x < (info_header.height * scale); x++)
    {
        for (int y = 0; y < (info_header.width * scale); y++)
        {
            if (x % scale == 0 && y % scale == 0)
                continue;
            if (x >= (info_header.height * scale) - (scale - 1))
            {
                new_data[x][y] = new_data[x - 1][y];
                continue;
            }
            if (y >= (info_header.width * scale) - (scale - 1))
            {
                new_data[x][y] = new_data[x][y - 1];
                continue;
            }

            int x0 = (x / scale) * scale;
            int x1 = min(x0 + scale, info_header.height * scale - 1);
            int x_ratio0 = x - x0;
            int x_ratio1 = x1 - x;
            int x_ratio = x_ratio0 + x_ratio1;

            int y0 = (y / scale) * scale;
            int y1 = min(y0 + scale, info_header.width * scale - 1);
            int y_ratio0 = y - y0;
            int y_ratio1 = y1 - y;
            int y_ratio = y_ratio0 + y_ratio1;

            float r0 = (y_ratio1 * new_data[x0][y0].r + y_ratio0 * new_data[x0][y1].r) / y_ratio;
            float r1 = (y_ratio1 * new_data[x1][y0].r + y_ratio0 * new_data[x1][y1].r) / y_ratio;

            float g0 = (y_ratio1 * new_data[x0][y0].g + y_ratio0 * new_data[x0][y1].g) / y_ratio;
            float g1 = (y_ratio1 * new_data[x1][y0].g + y_ratio0 * new_data[x1][y1].g) / y_ratio;

            float b0 = (y_ratio1 * new_data[x0][y0].b + y_ratio0 * new_data[x0][y1].b) / y_ratio;
            float b1 = (y_ratio1 * new_data[x1][y0].b + y_ratio0 * new_data[x1][y1].b) / y_ratio;

            new_data[x][y] = pixel(
                (x_ratio1 * r0 + x_ratio0 * r1) / x_ratio,
                (x_ratio1 * g0 + x_ratio0 * g1) / x_ratio,
                (x_ratio1 * b0 + x_ratio0 * b1) / x_ratio);
        }
    }
    data = new_data;
    info_header.width *= scale;
    info_header.height *= scale;
}

void Bitmap_cpp::ZoomOut(int scale)
{
    CheckValid();
    vector<vector<pixel>> new_data(info_header.height / scale, vector<pixel>(info_header.width / scale));
    for (int x = 0; x < info_header.height / scale; x++)
    {
        for (int y = 0; y < info_header.width / scale; y++)
        {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int i = 0; i < scale; i++)
            {
                for (int j = 0; j < scale; j++)
                {
                    int pos_x = x * scale + i;
                    int pos_y = y * scale + j;
                    sum_r += data[pos_x][pos_y].r;
                    sum_g += data[pos_x][pos_y].g;
                    sum_b += data[pos_x][pos_y].b;
                }
            }
            int scale_2 = scale * scale;
            new_data[x][y].r = sum_r / scale_2;
            new_data[x][y].g = sum_g / scale_2;
            new_data[x][y].b = sum_b / scale_2;
        }
    }
    data = new_data;
    info_header.width /= scale;
    info_header.height /= scale;
}

void Bitmap_cpp::HistogramEqualization_Global()
{
    CheckValid();
    if (data[0][0].r != data[0][0].g || data[0][0].r != data[0][0].b)
        throw runtime_error("Error: image is not a gray image");

    int histogram[256] = {0};
    for (auto& row : data)
        for (auto& p : row)
            histogram[p.r]++;

    int cdf[256] = {0};
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i - 1] + histogram[i];
    
    int min_cdf = cdf[0];
    for (int i = 0; i < 256; i++)
    {
        if (histogram[i] > 0)
        {
            min_cdf = cdf[i];
            break;
        }
    }

    int totel_pixel = info_header.width * info_header.height;
    if (totel_pixel == min_cdf)
        return;

    for (auto& row : data)
    {
        for (auto& p : row)
        {
            int new_value = (cdf[p.r] - min_cdf) * 255 / (totel_pixel - min_cdf);
            p.r = p.g = p.b = new_value;
        }
    }
}

void Bitmap_cpp::HistogramEqualization_Local(int block_size)
{
    CheckValid();
    if (data[0][0].r != data[0][0].g || data[0][0].r != data[0][0].b)
        throw runtime_error("Error: image is not a gray image");
    if (block_size <= 0)
        throw runtime_error("Error: block size must be greater than 0");

    int padding = block_size / 2 + block_size % 2 - 1;
    int block_adjustment = 1 - block_size % 2;
    vector<vector<pixel>> new_data(info_header.height, vector<pixel>(info_header.width));
    for (int x = padding; x < info_header.height - padding - block_adjustment; x++)
    {
        int histogram[256] = {0};
        for (int i = -padding; i <= padding + block_adjustment; i++)
            for (int j = -padding; j <= padding + block_adjustment; j++)
                histogram[data[x + i][padding + j].r]++;

        for (int y = padding; y <= info_header.width - padding - block_adjustment; y++)
        {
            if (y > padding)
            {
                for (int i = -padding; i <= padding + block_adjustment; i++)
                {
                    histogram[data[x + i][y - padding - 1].r]--;
                    histogram[data[x + i][y + padding + block_adjustment].r]++;
                }
            }

            int cdf[256] = {0};
            cdf[0] = histogram[0];
            for (int i = 1; i < 256; i++)
                cdf[i] = cdf[i - 1] + histogram[i];
            
            int min_cdf = cdf[0];
            for (int i = 0; i < 256; i++)
            {
                if (histogram[i] > 0)
                {
                    min_cdf = cdf[i];
                    break;
                }
            }

            int totel_pixel = (block_size + block_adjustment) * (block_size + block_adjustment);
            if (totel_pixel == min_cdf)
                continue;

            int new_value = (cdf[data[x][y].r] - min_cdf) * 255 / (totel_pixel - min_cdf);
            new_data[x][y].r = new_data[x][y].g = new_data[x][y].b = new_value;
        }
    }
    data = new_data;
}

Bitmap_cpp Bitmap_cpp::operator+(const Bitmap_cpp& other)
{
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    Bitmap_cpp result = *this;
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            result.data[x][y] = data[x][y] + other.data[x][y];

    return result;
}

Bitmap_cpp Bitmap_cpp::operator-(const Bitmap_cpp& other)
{
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    Bitmap_cpp result = *this;
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            result.data[x][y] = data[x][y] - other.data[x][y];

    return result;
}

Bitmap_cpp Bitmap_cpp::operator*(const int& scaler)
{
    CheckValid();
    Bitmap_cpp result = *this;
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            result.data[x][y] = data[x][y] * scaler;

    return result;
}

Bitmap_cpp Bitmap_cpp::operator/(const int& scaler)
{
    CheckValid();
    if (scaler == 0)
        throw runtime_error("Error: division by zero");

    Bitmap_cpp result = *this;
    for (int x = 0; x < info_header.height; x++)
        for (int y = 0; y < info_header.width; y++)
            result.data[x][y] = data[x][y] / scaler;

    return result;
}

void Bitmap_cpp::and_with(const Bitmap_cpp& other)
{
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int x = 0; x < info_header.height; x++)
    {
        for (int y = 0; y < info_header.width; y++)
        {
            data[x][y].r &= other.data[x][y].r;
            data[x][y].g &= other.data[x][y].g;
            data[x][y].b &= other.data[x][y].b;
        }
    }
}

void Bitmap_cpp::or_with(const Bitmap_cpp& other)
{
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int x = 0; x < info_header.height; x++)
    {
        for (int y = 0; y < info_header.width; y++)
        {
            data[x][y].r |= other.data[x][y].r;
            data[x][y].g |= other.data[x][y].g;
            data[x][y].b |= other.data[x][y].b;
        }
    }
}

void Bitmap_cpp::xor_with(const Bitmap_cpp& other)
{
    CheckValid();
    if (info_header.width != other.info_header.width || info_header.height != other.info_header.height)
        throw runtime_error("Error: image size error, " + to_string(info_header.width) + "x" + to_string(info_header.height) + "(origin) vs " + to_string(other.info_header.width) + "x" + to_string(other.info_header.height) + "(other)");

    for (int x = 0; x < info_header.height; x++)
    {
        for (int y = 0; y < info_header.width; y++)
        {
            data[x][y].r ^= other.data[x][y].r;
            data[x][y].g ^= other.data[x][y].g;
            data[x][y].b ^= other.data[x][y].b;
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
    CheckValid();
    // 準備數據
    int stride = ((info_header.width * 3 + 3) / 4) * 4;
    vector<unsigned char> raw_data(stride * info_header.height);
    
    // 排列像素數據
    for (int x = 0; x < info_header.height; x++)
    {
        for (int y = 0; y < info_header.width; y++)
        {
            int dest_row = info_header.height - 1 - x;
            int dest_index = dest_row * stride + y * 3;
            
            raw_data[dest_index] = data[x][y].b;
            raw_data[dest_index + 1] = data[x][y].g;
            raw_data[dest_index + 2] = data[x][y].r;
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