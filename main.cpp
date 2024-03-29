#include "Image3x8.h"
#include "Bmp.h"

int main()
{
    Image3x8 image = create_3x8_from_bmp("original.bmp");
    image.gaussian_blur(10);
    write_bmp_file(image, "original_copy.bmp");
    return 0;
}