#include "BmpMetaData.h"
#include "Image3x8.h"

int main()
{
    const BmpMetaData meta("sample_1280×853.bmp");
    Image3x8 copy(meta);
    copy.gray_scale();
    copy.edges();
    copy.write("copy.bmp");
    return 0;
}
