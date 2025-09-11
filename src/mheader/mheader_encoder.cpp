#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string>
#include "./mheader.h"


mHeader *generate_hdr(char *string_data, size_t len,
	       	uint32_t seq,
	       	bool eoh, char type)
{
    mHeader *hdr = (mHeader *) calloc(1, sizeof(mHeader));
    
    hdr->timestamp = time(0);
    hdr->seq  = seq;
    hdr->eoh  = eoh;
    hdr->type = type;
    hdr->len  = len;
    memcpy(hdr->data, string_data, sizeof(hdr->data));

    return hdr;
}

// for debugging
void print_mheader_attributes(mHeader *hdr)
{
    
    struct tm *time = localtime(&hdr->timestamp);

    printf(
        "timestamp field: %d:%d:%d\n"\
	"sequence field: %d\n"\
	"length field: %lu\n"\
	"data field: %s\n%s",
	time->tm_hour, time->tm_min, time->tm_sec,
	hdr->seq,
	hdr->len,
	hdr->data,
	(hdr->eoh) ? "END OF HEADER\n" : ""
    );
}

mheader_ls encode_mheader(char *data, size_t len, char type)
{
    if (data == NULL)
	return (mheader_ls) NULL;
    

    mheader_ls ls;
    mHeader *hChunk;
    uint32_t chunk_index = 0;

    for (; len > 0; len -= DATA_SIZE) {

         size_t pos = chunk_index * DATA_SIZE; // determine the chunk we are going to copy

         if (len <= DATA_SIZE) {


	    hChunk = generate_hdr ( 
		data + pos,
                len,             // we use remaining length because it can determine last chunk's actual size
		chunk_index,
		true,            // end of header
                type
	    );
	    
	    
	    ls.push_back(hChunk);
	    return ls;          // returns the list of headers created
	    
	}
	hChunk = generate_hdr (
	    data + pos,
	    DATA_SIZE,          // full length, because we are carrying middle parts
	    chunk_index,
	    false,
            type
	);
	
	
	ls.push_back(hChunk);
        chunk_index ++;
    
    }

    return (mheader_ls) NULL;

    
}

void destroy_mheader (mheader_ls ls)
{

    for (int i=0; i < ls.size(); i++) {
        free(ls[i]);
	ls[i] = NULL;
    }
    ls.clear();

}

