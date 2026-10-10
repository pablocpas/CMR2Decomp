// Runtime-library code linked into CMR2.exe from Microsoft's CRT.

typedef unsigned char BYTE;

// Scalar deleting destructor the compiler generates for the CRT's type_info
// (RTTI support): it calls the imported destructor directly and, when the low
// bit of the flag is set, frees the object with the imported operator delete.
class type_info
{
public:
    virtual ~type_info();
    void *DeleteWithFlags(BYTE param_1);
};

// The runtime library is built for size (/O1, see scripts/build.py), which is
// why the argument is popped with `pop ecx` rather than `add esp,4`.
// FUNCTION: CMR2 0x005105a0
void *type_info::DeleteWithFlags(BYTE param_1)
{
    this->type_info::~type_info();
    if (param_1 & 1)
        ::operator delete(this);
    return this;
}
