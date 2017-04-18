/*! \todo Add license */

#include "fillRect.h"

using namespace gepard;

int main(int argc, char *argv[])
{
    if(!initVulkan())
        return 1;

    fillRect(0, 0, 100, 100);

    deinitVulkan();
    return 0;
}
