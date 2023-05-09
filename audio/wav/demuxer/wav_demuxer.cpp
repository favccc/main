#include "wav_demuxer.hpp"

#include <string.h>

#include "xlog.hpp"

#define MKTAG(a,b,c,d)   ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#define BIT2BYTE(value)   ((value) / 8)

WavDemuxer::WavDemuxer(const std::string &filename)
    : _filename(filename)
{
    doLoadFile();
}

WavDemuxer::~WavDemuxer()
{
    doCloseFile();
}

bool WavDemuxer::loadSuccessful()
{
    if (_load_info)
    {
        return true;
    }
    return false;
}

int WavDemuxer::format()
{
    if (_load_info)
    {
        return _load_info->fmt.audio_format;
    }

    return -1;
}

int WavDemuxer::numChannels()
{
    if (_load_info)
    {
        return _load_info->fmt.num_channels;
    }
    return -1;
}

int WavDemuxer::sampleRate()
{
    if (_load_info)
    {
        return _load_info->fmt.sample_rate;
    }
    return -1;
}

int WavDemuxer::sampleBits()
{
    if (_load_info)
    {
        return _load_info->fmt.bits_per_sample;
    }
    return -1;
}

int WavDemuxer::numSamples()
{
    int num_samples = -1;

    do
    {
        if (!_load_info)
        {
            break;
        }

        int bytes_per_sample = BIT2BYTE(_load_info->fmt.bits_per_sample);
        if (bytes_per_sample <= 0)
        {
            break;
        }

        num_samples = _load_info->data_size / bytes_per_sample;
    }
    while(0);

    return num_samples;
}

std::vector<uint8_t> WavDemuxer::getSamples(int ch, int pos, int count)
{
    std::vector<uint8_t> buf;

    do 
    {
        if (!_load_info)
        {
            break;
        }

        if (ch < 0 || pos < 0 || count < 0)
        {
            break;
        }

        int bytes_per_sample = BIT2BYTE(_load_info->fmt.bits_per_sample);
        if (bytes_per_sample <= 0)
        {
            break;
        }

        int pos_in_bytes = pos * bytes_per_sample + _load_info->data_offset;
        int count_in_bytes = count * bytes_per_sample;
        
        _load_info->xio->seek(pos_in_bytes, SEEK_SET);

        buf = _load_info->xio->read(count_in_bytes);
    }
    while (0);

    return buf;
}

int WavDemuxer::doLoadFile()
{
    int berror = false;

    std::shared_ptr<LoadInfo> info;

    info = std::make_shared<LoadInfo>();

    do 
    {
        if (!info->xio)
        {
            info->xio = std::make_shared<XIOFile>(_filename, 0);
        }

        if (!info->xio)
        {
            xlog_err("open file failed");
            berror = true;
            break;
        }

        if (readHeader(info) < 0)
        {
            xlog_err("read header failed");
            berror = true;
            break;
        }
    }
    while (0);
    
    if (berror)
    {
        info.reset();
        return -1;
    }

    _load_info = info;

    return 0;
}

int WavDemuxer::doCloseFile()
{
    _load_info.reset();

    return 0;
}

int WavDemuxer::readHeader(std::shared_ptr<LoadInfo> info)
{
    int berror = false;

    bool has_riff = false;
    bool has_fmt = false;
    bool has_data = false;

    if (!info || !info->xio)
    {
        xlog_err("null");
        return -1;
    }

    info->xio->seek(0, SEEK_SET);

    for (;;)
    {
        if (berror)
        {
            xlog_err("error");
            break;
        }

        if (info->xio->eof())
        {
            xlog_trc("eof(read)");
            break;
        }
        
        if (info->xio->tell() >= info->xio->size())
        {
            xlog_trc("eof(seek)");
            break;
        }

        xlog_trc("offset=%d", (int)info->xio->tell());

        // get next tag
        uint32_t chunkid = info->xio->rl32();
        uint32_t chunksize = info->xio->rl32();

        switch (chunkid)
        {
            case MKTAG('R', 'I', 'F', 'F'):
            {
                xlog_trc("parsing chunk RIFF");
                info->riff.tag.chunkid = chunkid;
                info->riff.tag.chunksize = chunksize;
                info->riff.format = info->xio->rl32();
                has_riff = true;
                break;
            }
            case MKTAG('f','m','t',' '):
            {
                xlog_trc("parsing chunk fmt");
                info->fmt.tag.chunkid = chunkid;
                info->fmt.tag.chunksize = chunksize;
                info->fmt.audio_format = info->xio->rl16();
                info->fmt.num_channels = info->xio->rl16();
                info->fmt.sample_rate = info->xio->rl32();
                info->fmt.byte_rate = info->xio->rl32();
                info->fmt.align = info->xio->rl16();
                info->fmt.bits_per_sample = info->xio->rl16();
                has_fmt = true;
                xlog_trc("fmt=%d,num_chn=%d,sample_rate=%d,"
                    "byte_rate=%d,align=%d,bits_per_sample=%d",
                    (int)info->fmt.audio_format,
                    (int)info->fmt.num_channels,
                    (int)info->fmt.sample_rate,
                    (int)info->fmt.byte_rate,
                    (int)info->fmt.align,
                    (int)info->fmt.bits_per_sample);
                break;
            }
            case MKTAG('d', 'a', 't', 'a'):
            {
                xlog_trc("parsing chunk data");
                info->fmt.tag.chunkid = chunkid;
                info->fmt.tag.chunksize = chunksize;
                info->data_offset = info->xio->tell();
                info->data_size = chunksize;
                if (info->xio->seek(chunksize, SEEK_CUR) < 0)
                {
                    xlog_err("seek over chunk data failed");
                    berror = true;
                    break;
                }
                has_data = true;
                break;
            }
            default:
            {
                char chunkname[5] = {};
                static_assert(4 == sizeof(chunkid), "chunkid size err");
                memcpy(chunkname, &chunkid, 4);

                xlog_trc("parsing chunk (%s), skipped", chunkname);
                if (info->xio->seek(chunksize, SEEK_CUR) < 0)
                {
                    xlog_err("seek over chunk unknown failed");
                    berror = true;
                    break;
                }
                break;
            }
        } // switch 
    } // for()

    if (!has_riff || !has_fmt || !has_data)
    {
        xlog_err("chunk incomplete(riff=%d,fmt=%d,data=%d)", 
            has_riff, has_fmt, has_data);
        berror = true;
    }

    // 为了读取的方便，做出一些限制
    if ((info->fmt.bits_per_sample % 8 != 0)
        || info->fmt.num_channels != 1)
    {
        xlog_err("bits_per_sample=%d, num_channels=%d", 
            info->fmt.bits_per_sample,
            info->fmt.num_channels);
        berror = true;
    }

    if (berror)
    {
        return -1;
    }
    return 0;
}

#if 0
int WavDemuxer::readHeader(std::shared_ptr<LoadInfo> info)
{
    int berror = false;

    size_t offset = 0;  // 下一个要扫描的块的起始位置
    size_t size = 0;    // 临时读大小

    bool has_riff = false;
    bool has_fmt = false;
    bool has_data = false;

    if (!info)
    {
        xlog_err("null info");
        return -1;
    }

    for (;;)
    {
        if (berror)
        {
            break;
        }

        xlog_dbg("offset=%lu", offset);

        // get next tag
        size = sizeof(NA_TAG);
        std::vector<uint8_t> buftag = readBuf(info, offset, size);
        if (!buftag.data() || buftag.size() != size)
        {
            xlog_dbg("no next tag");
            break;
        }

        NA_TAG tag{};
        memcpy(&tag, buftag.data(), size);

        std::vector<uint8_t> buf;
        switch (tag.chunkid)
        {
            case MKTAG('R', 'I', 'F', 'F'):
            {
                xlog_dbg("parsing chunk RIFF");
                size = sizeof(NA_RIFF);
                buf = readBuf(info, offset, size);
                NA_RIFF *nariff = (NA_RIFF*)buf.data();
                if (!nariff || buf.size() != size)
                {
                    xlog_err("parse chunk riff failed");
                    berror = true;
                    break;
                }
                
                memcpy(&info->riff, nariff, size);
                offset += size;

                has_riff = true;

                break;
            }
            case MKTAG('f','m','t',' '):
            {
                xlog_dbg("parsing chunk fmt");
                size = sizeof(NA_SubChunkFmt);
                buf = readBuf(info, offset, size);
                NA_SubChunkFmt* nafmt = (NA_SubChunkFmt*)buf.data();
                if (!nafmt || buf.size() != size)
                {
                    xlog_err("parse chunk fmt failed");
                    berror = true;
                    break;
                }

                memcpy(&info->fmt, nafmt, size);
                offset += tag.chunksize + sizeof(NA_TAG);

                has_fmt = true;

                break;
            }
            case MKTAG('d', 'a', 't', 'a'):
            {
                xlog_dbg("parsing chunk data");
                size = sizeof(NA_SubChunkData);
                buf = readBuf(info, offset, size);
                NA_SubChunkData* data = (NA_SubChunkData*)buf.data();
                if(!data || buf.size() != size)
                {
                    xlog_err("parse chunk data failed");
                    berror = true;
                    break;
                }

                info->data_offset = offset + sizeof(NA_TAG);
                info->data_size = data->tag.chunksize;
                offset += tag.chunksize + sizeof(NA_TAG);

                has_data = true;

                break;
            }
            default:
            {
                xlog_dbg("parsing chunk unknown");
                offset += tag.chunksize + sizeof(NA_TAG);
                break;
            }
        } // switch
    } // for()

    if (!has_riff || !has_fmt || !has_data)
    {
        xlog_err("chunk incomplete(riff=%d,fmt=%d,data=%d)", 
            has_riff, has_fmt, has_data);
        berror = true;
    }

    // 为了读取的方便，做出一些限制
    if ((info->fmt.bits_per_sample % 8 != 0)
        || info->fmt.num_channels != 1)
    {
        xlog_err("bits_per_sample=%d, num_channels=%d", 
            info->fmt.bits_per_sample,
            info->fmt.num_channels);
        berror = true;
    }

    if (berror)
    {
        return -1;
    }
    return 0;
}
#endif