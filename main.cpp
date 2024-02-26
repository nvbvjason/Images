#include "BmpMetaData.h"
#include "Image3x8.h"

int main()
{
    BmpMetaData meta("yard.bmp");
    Image3x8 copy(meta);
    copy.gray_scale_lum();
    copy.write("copy.bmp");
    return 0;
}
