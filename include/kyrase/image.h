#ifndef IMAGE_H_
#define IMAGE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <string_view>
#include <utility> // std::move

using std::vector;
using std::string; 

class Image {
public:
    // Constructors

    Image(
        const string& image_format,
        int width,
        int height,
        int channels,
        int max_color_val
    );

    // RGB 
    Image(
        const string& image_format,
        int width,
        int height,
        int max_color_val,
        const vector<uint8_t>& r,
        const vector<uint8_t>& g,
        const vector<uint8_t>& b
    );

    // RGBA
    Image(
        const string& image_format,
        int width,
        int height,
        int max_color_val,
        const vector<uint8_t>& r,
        const vector<uint8_t>& g,
        const vector<uint8_t>& b,
        const vector<uint8_t>& a
    );

    // Getters
    const string& get_image_format() const;
    int get_width() const;
    int get_height() const;
    int get_channels() const;
    int get_max_color_val() const;

    const vector<uint8_t>& get_r() const;
    const vector<uint8_t>& get_g() const;
    const vector<uint8_t>& get_b() const;
    const vector<uint8_t>& get_a() const;

    // Setters
    void set_image_format(const string& format);
    void set_max_color_val(int max_color_val);

    void set_r(const vector<uint8_t>& r);
    void set_g(const vector<uint8_t>& g);
    void set_b(const vector<uint8_t>& b);
    void set_a(const vector<uint8_t>& a);
    void set_rgb(
        const vector<uint8_t>& r,
        const vector<uint8_t>& g,
        const vector<uint8_t>& b
    );
    void set_rgba(
        const vector<uint8_t>& r,
        const vector<uint8_t>& g,
        const vector<uint8_t>& b,
        const vector<uint8_t>& a
    );

    void set_r(vector<uint8_t>&& new_r);
    void set_g(vector<uint8_t>&& new_g);
    void set_b(vector<uint8_t>&& new_b);
    void set_a(vector<uint8_t>&& new_a);
    void set_rgb(
        vector<uint8_t>&& new_r,
        vector<uint8_t>&& new_g,
        vector<uint8_t>&& new_b
    );
    void set_rgba(
        vector<uint8_t>&& new_r,
        vector<uint8_t>&& new_g,
        vector<uint8_t>&& new_b,
        vector<uint8_t>&& new_a
    );

private:
    static void validateConstructorInput(
        const string& image_format,
        int width,
        int height,
        int channels,
        int max_color_val
    );

    static size_t getExpectedChannelSize(int width, int height);
    
    static void validateChannelSize(
        const vector<uint8_t>& values,
        size_t expected_size,
        std::string_view channel_name
    );

    string image_format;
	int width;
	int height;
	int channels;
	// Max 255
	int max_color_val;
	// width * height
	vector<uint8_t> r;
	vector<uint8_t> g;
	vector<uint8_t> b;
	vector<uint8_t> a;
};

#endif