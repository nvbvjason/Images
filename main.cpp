#include "Image3x8.h"

int main()
{
    Image3x8 copy("sample_1280×853.bmp");
    copy.grey_scale();
    copy.gaussian_blur(6);
    copy.edges();
    copy.write("sample_1280×853_copy.bmp");
    return 0;
}