#include <ctime>
#include <vector>
#include <cstdint>


#define DATA_SIZE 256

enum mheader_types {
    MHEADER_MESSAGE = 1,
    MHEADER_FILE    = 2
};

typedef struct mHeader {
    time_t   timestamp;
    uint32_t seq;
    char     type;
    bool     eoh;
    char     data[DATA_SIZE];
    size_t   len;


} mHeader;


typedef std::vector<mHeader *> mheader_ls;

mheader_ls encode_mheader (char *data, size_t len, char type);

void destroy_mheader (mheader_ls ls);
