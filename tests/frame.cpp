#include <CANBridge.h>
#include <cassert>
using namespace karura::can;
int main() {
    Frame f;
    assert(valid(f));
    f.id=0x7FF; f.length=8; assert(valid(f));
    f.id=0x800; assert(!valid(f));
    f.extended=true; assert(valid(f));
    f.id=0x1FFFFFFF; assert(valid(f));
    f.id=0x20000000; assert(!valid(f));
    f.id=1; f.length=9; assert(!valid(f));
    f.length=8; f.remote=true; assert(valid(f));
    Health h;
    assert(!h.lossDetectionComplete);
}
