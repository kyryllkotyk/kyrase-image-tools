#include "kyrase/image.h"

// Constructors

Image::Image(
    const string& image_format,
    int width,
    int height,
    int channels,
    int max_color_val
)
    : image_format(image_format),
    width(width),
    height(height),
    channels(channels),
    max_color_val(max_color_val),
    r(),
    g(),
    b(),
    a() {

    validateConstructorInput(image_format, width, height, channels, max_color_val);

    const size_t channel_size = getExpectedChannelSize(width, height);

    r.resize(channel_size);
    g.resize(channel_size);
    b.resize(channel_size);

    if (channels == 4) {
        a.resize(channel_size);
    }
}

Image::Image(
    const string& image_format,
    int width,
    int height,
    int max_color_val,
    const vector<uint8_t>& r_values,
    const vector<uint8_t>& g_values,
    const vector<uint8_t>& b_values
)
    : image_format(image_format),
    width(width),
    height(height),
    channels(3),
    max_color_val(max_color_val)
{

    validateConstructorInput(image_format, width, height, 3, max_color_val);

    const size_t expected_size = getExpectedChannelSize(width, height);

    validateChannelSize(r_values, expected_size, "R");
    validateChannelSize(g_values, expected_size, "G");
    validateChannelSize(b_values, expected_size, "B");

    r = r_values;
    g = g_values;
    b = b_values;
}

Image::Image(
    const string& image_format,
    int width,
    int height,
    int max_color_val,
    const vector<uint8_t>& r_values,
    const vector<uint8_t>& g_values,
    const vector<uint8_t>& b_values,
    const vector<uint8_t>& a_values
)
    : image_format(image_format),
    width(width),
    height(height),
    channels(4),
    max_color_val(max_color_val)
{

    validateConstructorInput(image_format, width, height, 4, max_color_val);

    const size_t expected_size = getExpectedChannelSize(width, height);

    validateChannelSize(r_values, expected_size, "R");
    validateChannelSize(g_values, expected_size, "G");
    validateChannelSize(b_values, expected_size, "B");
    validateChannelSize(a_values, expected_size, "A");

    r = r_values;
    g = g_values;
    b = b_values;
    a = a_values;
}

// Getters

const string& Image::get_image_format() const {
    return image_format;
}

int Image::get_width() const {
    return width;
}

int Image::get_height() const {
    return height;
}

int Image::get_channels() const {
    return channels;
}

int Image::get_max_color_val() const {
    return max_color_val;
}

const vector<uint8_t>& Image::get_r() const {
    return r;
}

const vector<uint8_t>& Image::get_g() const {
    return g;
}

const vector<uint8_t>& Image::get_b() const {
    return b;
}

const vector<uint8_t>& Image::get_a() const {
    return a;
}

// Setters

void Image::set_image_format(const string& format) {
    if (format.empty()) {
        throw std::invalid_argument("Image: Format must not be empty");
    }
    
    // Currently assumes the right format was specified
    image_format = format;
}

void Image::set_max_color_val(int max_color_val) {
    if (max_color_val != 255) {
        throw std::invalid_argument("Image: Only max color value of 255 is supported");
    }

    this->max_color_val = max_color_val;
}

void Image::set_r(const vector<uint8_t>& r) {
    const size_t expected_size = getExpectedChannelSize(width, height);
    validateChannelSize(r, expected_size, "R");

    this->r = r;
}

void Image::set_g(const vector<uint8_t>& g) {
    const size_t expected_size = getExpectedChannelSize(width, height);
    validateChannelSize(g, expected_size, "G");


    this->g = g;
}

void Image::set_b(const vector<uint8_t>& b) {
    const size_t expected_size = getExpectedChannelSize(width, height);
    validateChannelSize(r, expected_size, "B");


    this->b = b;
}

void Image::set_a(const vector<uint8_t>& a) {
    // This should get changed if 5+ channels ever get supported
    // or grayscale + alpha
    if (channels != 4) {
        throw std::invalid_argument("Image: A channel can only be set for RGBA");
    }

    const size_t expected_size = getExpectedChannelSize(width, height);
    validateChannelSize(a, expected_size, "A");

    this->a = a;
}

void Image::set_rgb(
    const vector<uint8_t>& r, 
    const vector<uint8_t>& g,
    const vector<uint8_t>& b
) {
    set_r(r);
    set_g(g);
    set_b(b);
}

void Image::set_rgba(
    const vector<uint8_t>& r,
    const vector<uint8_t>& g,
    const vector<uint8_t>& b,
    const vector<uint8_t>& a
) {
    set_r(r);
    set_g(g);
    set_b(b);
    set_a(a);
}

void Image::set_r(vector<uint8_t>&& new_r) {
    r = std::move(new_r);
}

void Image::set_g(vector<uint8_t>&& new_g) {
    g = std::move(new_g);
}

void Image::set_b(vector<uint8_t>&& new_b) {
    b = std::move(new_b);
}

void Image::set_a(vector<uint8_t>&& new_a) {
    a = std::move(new_a);
}

void Image::set_rgb(
    vector<uint8_t>&& r,
    vector<uint8_t>&& g,
    vector<uint8_t>&& b
) {
    set_r(std::move(r));
    set_g(std::move(g));
    set_b(std::move(b));
}

void Image::set_rgba(
    vector<uint8_t>&& r,
    vector<uint8_t>&& g,
    vector<uint8_t>&& b,
    vector<uint8_t>&& a
) {
    set_r(std::move(r));
    set_g(std::move(g));
    set_b(std::move(b));
    set_b(std::move(a));
}

void Image::validateConstructorInput(
    const string& image_format,
    int width,
    int height,
    int channels,
    int max_color_val
) {
    if (image_format.empty()) {
        throw std::invalid_argument("Image: Format must not be empty");
    }

    if (width <= 0) {
        throw std::invalid_argument("Image: Width must be greater than 0");
    }

    if (height <= 0) {
        throw std::invalid_argument("Image: Height must be greater than 0");
    }

    if (channels != 3 && channels != 4) {
        throw std::invalid_argument("Image: Only RGB and RGBA images are supported");
    }

    if (max_color_val != 255) {
        throw std::invalid_argument("Image: Only max color value of 255 is supported");
    }

    constexpr size_t max_int = static_cast<size_t>(std::numeric_limits<int>::max());

    const size_t width_size = static_cast<size_t>(width);
    const size_t height_size = static_cast<size_t>(height);
    const size_t channel_count = static_cast<size_t>(channels);

    if (width_size > max_int / height_size) {
        throw std::invalid_argument("Image: Image size is too large");
    }

    const size_t channel_size = width_size * height_size;

    if (channel_size > max_int / channel_count) {
        throw std::invalid_argument("Image: Image size is too large");
    }
}

size_t Image::getExpectedChannelSize(int width, int height) {
    return static_cast<size_t>(width) * static_cast<size_t>(height);
}

void Image::validateChannelSize(
    const vector<uint8_t>& values,
    size_t expected_size,
    std::string_view channel_name
) {
    if (values.size() != expected_size) {
        throw std::invalid_argument(
            std::string("Image: ") + std::string(channel_name) +
            " size doesn't match expected channel size"
        );
    }
}