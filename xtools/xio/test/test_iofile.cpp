#include <gtest/gtest.h>
#include <limits>

#include "xio.hpp"

static size_t fileSize(FILE *fp)
{
    size_t size_file = 0;
    size_t size_origin = 0;
    if (fp)
    {
        size_origin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size_file = ftell(fp);
        fseek(fp, size_origin, SEEK_SET);
    }

    return size_file;
}

static std::vector<uint8_t> readFile(const std::string &filename)
{
    std::vector<uint8_t> buffer;
    FILE *fp = fopen(filename.c_str(), "rb");

    if (fp)
    {
        size_t filesize = fileSize(fp);

        
        buffer.resize(filesize);

        fread(buffer.data(), 1, filesize, fp);

        fclose(fp);
        fp = nullptr;
    }

    return buffer;
}

static void saveFile(const std::string &filename, std::vector<uint8_t> data)
{
    FILE *fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        fwrite(data.data(), 1, data.size(), fp);
        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
    }
}

static std::vector<uint8_t> generateData()
{
    std::vector<uint8_t> data;
    std::size_t size = 1000;

    data.reserve(size);

    for (std::size_t i = 0; i < size; ++i)
    {
        data.push_back(i % std::numeric_limits<uint8_t>().max());
    }

    return data;
}

class BaseFileIOTest : public testing::Test
{
protected:

    const char filename[64] = "xtools_xio_fileio_test.data";
    const char filename_output[64] = "xtools_xio_fileio_output.data";

    std::vector<uint8_t> data;

    void SetUp() override
    {
        data = generateData();
        if (readFile(filename) != data)
        {
            saveFile(filename, data);
        }

        EXPECT_EQ(readFile(filename), data);
    }
    void TearDown() override
    {

    }
};

TEST_F(BaseFileIOTest, base)
{
    XIOFile iofile(filename, "rb");

    EXPECT_FALSE(iofile.eof());
    EXPECT_FALSE(iofile.error());

    EXPECT_EQ(iofile.size(), (int64_t)data.size());
    EXPECT_EQ(iofile.tell(), 0);
}

TEST_F(BaseFileIOTest, read_ele)
{
    XIOFile iofile(filename, "rb");

    std::size_t offset = 0;

    do 
    {
        offset = 5;
        iofile.seek(offset, SEEK_SET);

        ASSERT_EQ((int64_t)offset, iofile.tell());

        ASSERT_LT(offset + 1, data.size());
        uint8_t r8 = iofile.r8();
        EXPECT_EQ(r8, data[offset]);
        offset += 1;

        ASSERT_EQ((int64_t)offset, iofile.tell());
        
        ASSERT_LT(offset + 1, data.size());
        r8 = iofile.r8();
        EXPECT_EQ(r8, data[offset]);
        offset += 1;

        
        ASSERT_LT(offset + 2, data.size());
        uint16_t r16 = iofile.rl16();
        EXPECT_EQ(r16, *(uint16_t*)&data[offset]);
        offset += 2;


        ASSERT_LT(offset + 4, data.size());
        uint32_t r32 = iofile.rl24();
        EXPECT_EQ(r32, (*(uint32_t*)&data[offset]) & 0x00ffffff);
        offset += 3;

        ASSERT_LT(offset + 4, data.size());
        r32 = iofile.rl32();
        EXPECT_EQ(r32, (*(uint32_t*)&data[offset]));
        offset += 4;

        ASSERT_LT(offset + 8, data.size());
        uint64_t r64 = iofile.rl64();
        EXPECT_EQ(r64, (*(uint64_t*)&data[offset]));
        offset += 8;
        
        ASSERT_EQ((int64_t)offset, iofile.tell());
    }
    while (0);
}

TEST_F(BaseFileIOTest, read_data)
{
    XIOFile iofile(filename, "rb");

    std::size_t offset = 0;
    std::size_t size = 0;

    offset = 100;
    size = 64;
    ASSERT_TRUE(offset + size < data.size());
    iofile.seek(offset, SEEK_SET);
    std::vector<uint8_t> read_data = iofile.read(size);
    std::vector<uint8_t> ref_data(data.begin() + offset, 
        data.begin() + offset + size);
    ASSERT_EQ(read_data, ref_data);
}

TEST_F(BaseFileIOTest, write_ele)
{
    auto iofile = std::make_shared<XIOFile>(filename_output, "wb+");
    uint64_t v64 = 0x6464646464646464;
    uint32_t v32 = 0x32323232;
    uint32_t v24 = 0x242424;
    uint16_t v16 = 0x1616;
    uint8_t v8 = 0x88;
    std::size_t count_limit = 32;
    
    for (std::size_t count = 0; count < count_limit; ++count)
    {
        if (count % 3 == 0)
        {
            iofile->wl64(v64);
            iofile->wl32(v32);
            iofile->wl24(v24);
            iofile->wl16(v16);
            iofile->w8(v8);
        }
        else if (count % 3 == 1)
        {
            iofile->w8(v8);
            iofile->wl16(v16);
            iofile->wl24(v24);
            iofile->wl32(v32);
            iofile->wl64(v64);
        }
        else 
        {
            iofile->wl32(v32);
            iofile->w8(v8);
            iofile->wl64(v64);
            iofile->wl16(v16);
            iofile->wl24(v24);
        }
    }

    iofile.reset();

    iofile = std::make_shared<XIOFile>(filename_output, "rb");

    for (std::size_t count = 0; count < count_limit; ++count)
    {
        if (count % 3 == 0)
        {
            EXPECT_EQ(v64, iofile->rl64());
            EXPECT_EQ(v32, iofile->rl32());
            EXPECT_EQ(v24, iofile->rl24());
            EXPECT_EQ(v16, iofile->rl16());
            EXPECT_EQ(v8, iofile->r8());
        }
        else if (count % 3 == 1)
        {
            EXPECT_EQ(v8, iofile->r8());
            EXPECT_EQ(v16, iofile->rl16());
            EXPECT_EQ(v24, iofile->rl24());
            EXPECT_EQ(v32, iofile->rl32());
            EXPECT_EQ(v64, iofile->rl64());
        }
        else 
        {
            EXPECT_EQ(v32, iofile->rl32());
            EXPECT_EQ(v8, iofile->r8());
            EXPECT_EQ(v64, iofile->rl64());
            EXPECT_EQ(v16, iofile->rl16());
            EXPECT_EQ(v24, iofile->rl24());
        }
    }

    iofile.reset();
}

TEST_F(BaseFileIOTest, write_data)
{
    auto iofile = std::make_shared<XIOFile>(filename_output, "wb+");
    iofile->write(data);
    iofile.reset();

    auto output_data = readFile(filename_output);
    EXPECT_EQ(output_data, data);
}