#include "imageview.hpp"

#include "xlog.hpp"

ImageView::ImageView(int w, int h, std::shared_ptr<std::vector<uint8_t>> mem)
{
    _state = init(w, h, mem);
}

ImageView::~ImageView()
{

}

int ImageView::ok() const
{
    return (_state ? true : false);
}

int ImageView::width() const
{
    if (_state)
    {
        return _state->w;
    }

    xlog_err("null");
    return 0;
}

int ImageView::height() const
{
    if (_state)
    {
        return _state->h;
    }

    xlog_err("null");
    return 0;
}

int ImageView::pixelBytes() const
{
    if (_state)
    {
        return _state->pixel_bytes;
    }

    xlog_err("null");
    return 0;
}

std::shared_ptr<std::vector<uint8_t>> ImageView::mem()
{
    if (_state)
    {
        return _state->mem;
    }
    
    return nullptr;
}

std::shared_ptr<const std::vector<uint8_t>> ImageView::mem() const
{
    if (_state)
    {
        return _state->mem;
    }
    
    return nullptr;
}

std::vector<uint8_t> ImageView::pixels(int x, int y, int num_pixels) const
{
    int berror = false;

    std::vector<uint8_t> data;

    do 
    {
        if (!_state)
        {
            xlog_err("null");
            berror = true;
            break;
        }

        if (x < 0 || y < 0 || num_pixels < 0)
        {
            xlog_err("invalid arg");
            berror = true;
            break;
        }

        int total_bytes = num_pixels * _state->pixel_bytes;
        if (total_bytes < 0)
        {
            xlog_err("invalid arg");
            berror = true;
            break;
        }

        if (!pixelAt(x, y, num_pixels))
        {
            xlog_err("over");
            berror = true;
            break;
        }

        data.reserve(total_bytes);

        const uint8_t *src = pixelAt(x, y, 0);
        for (int i = 0; i < total_bytes; ++i)
        {
            data.push_back(src[i]);
        }
    }
    while (0);

    return (berror ? std::vector<uint8_t>() : data);
}

int ImageView::drawPixels(int x, int y, const std::vector<uint8_t> &pixels)
{
    int berror = false;

    do 
    {
        if (!_state)
        {
            xlog_err("not ok");
            berror = true;
            break;
        }

        if (pixels.size() % _state->pixel_bytes)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        int num_pixels = pixels.size() / _state->pixel_bytes;

        if (!pixelAt(x, y, num_pixels))
        {
            xlog_err("invalid args[over]");
            berror = true;
            break;
        }

        for (int i = 0; i < num_pixels; ++i)
        {
            const uint8_t *psrc = &pixels[i * _state->pixel_bytes];
            uint8_t *pdst = pixelAt(x, y, i);

            for (int k = 0; k < _state->pixel_bytes; ++k)
            {
                pdst[k] = psrc[k];
            }
        }
    }
    while (0);

    return (berror ? -1 : 0);
}

int ImageView::scale(int w, int h)
{
    xlog_err("not implement");
    return -1;
}

int ImageView::drawRect(int x, int y, int w, int h, const std::vector<uint8_t> &pixels)
{
    xlog_err("not implement");
    return -1;
}

std::shared_ptr<ImageView::State> ImageView::init(int w, int h, std::shared_ptr<std::vector<uint8_t>> mem_arg)
{
    std::shared_ptr<State> state;
    int berror = false;

    do
    {
        if (w <= 0 || h <= 0 || !mem_arg)
        {
            xlog_err("invalid args");
            berror = true;
            break;
        }

        int num_pixel = w * h;
        if (num_pixel <= 0)
        {
            xlog_err("invalid args(w,h over)");
            berror = true;
            break;
        }

        if (mem_arg->size() % num_pixel)
        {
            xlog_err("invalid args(size != w * h * depth)");
            berror = true;
            break;
        }

        state = std::make_shared<State>();

        state->mem = mem_arg;
        state->w = w;
        state->h = h;
        state->pixel_bytes = mem_arg->size() / num_pixel;

        if (!state->pixel_bytes)
        {
            xlog_err("pixel depth is 0");
            berror = true;
            break;
        }
    }
    while (0);

    return (berror ? nullptr : state);
}

uint8_t* ImageView::pixelAt(int x, int y, int offset_pixels)
{
    if (!_state)
    {
        return nullptr;
    }

    if (x < 0 || x >= _state->w
        || y < 0 || y >= _state->h)
    {
        return nullptr;
    }

    int pixel_offset = y * _state->w + x + offset_pixels;
    int bytes_offset = pixel_offset * _state->pixel_bytes;

    if (bytes_offset < 0 || bytes_offset >= _state->mem->size())
    {
        return nullptr;
    }

    return _state->mem->data() + bytes_offset;
}

const uint8_t* ImageView::pixelAt(int x, int y, int offset_pixels) const
{
    if (!_state)
    {
        return nullptr;
    }

    if (x < 0 || x >= _state->w
        || y < 0 || y >= _state->h)
    {
        return nullptr;
    }

    int pixel_offset = y * _state->w + x + offset_pixels;
    int bytes_offset = pixel_offset * _state->pixel_bytes;

    if (bytes_offset < 0 || bytes_offset >= _state->mem->size())
    {
        return nullptr;
    }

    return _state->mem->data() + bytes_offset;
}