static int nsv_parse_NSVf_header(AVFormatContext *s)

{

    NSVContext *nsv = s->priv_data;

    AVIOContext *pb = s->pb;

    unsigned int av_unused file_size;

    unsigned int size;

    int64_t duration;

    int strings_size;

    int table_entries;

    int table_entries_used;



    av_dlog(s, "%s()\n", __FUNCTION__);



    nsv->state = NSV_UNSYNC; 



    size = avio_rl32(pb);

    if (size < 28)

        return -1;

    nsv->NSVf_end = size;



    

    file_size = (uint32_t)avio_rl32(pb);

    av_dlog(s, "NSV NSVf chunk_size %u\n", size);

    av_dlog(s, "NSV NSVf file_size %u\n", file_size);



    nsv->duration = duration = avio_rl32(pb); 

    av_dlog(s, "NSV NSVf duration %"PRId64" ms\n", duration);

    



    strings_size = avio_rl32(pb);

    table_entries = avio_rl32(pb);

    table_entries_used = avio_rl32(pb);

    av_dlog(s, "NSV NSVf info-strings size: %d, table entries: %d, bis %d\n",

            strings_size, table_entries, table_entries_used);

    if (pb->eof_reached)

        return -1;



    av_dlog(s, "NSV got header; filepos %"PRId64"\n", avio_tell(pb));



    if (strings_size > 0) {

        char *strings; 

        char *p, *endp;

        char *token, *value;

        char quote;



        p = strings = av_mallocz(strings_size + 1);

        endp = strings + strings_size;

        avio_read(pb, strings, strings_size);

        while (p < endp) {

            while (*p == ' ')

                p++; 

            if (p >= endp-2)

                break;

            token = p;

            p = strchr(p, '=');

            if (!p || p >= endp-2)

                break;

            *p++ = '\0';

            quote = *p++;

            value = p;

            p = strchr(p, quote);

            if (!p || p >= endp)

                break;

            *p++ = '\0';

            av_dlog(s, "NSV NSVf INFO: %s='%s'\n", token, value);

            av_dict_set(&s->metadata, token, value, 0);

        }

        av_free(strings);

    }

    if (pb->eof_reached)

        return -1;



    av_dlog(s, "NSV got infos; filepos %"PRId64"\n", avio_tell(pb));



    if (table_entries_used > 0) {

        int i;

        nsv->index_entries = table_entries_used;

        if((unsigned)table_entries_used >= UINT_MAX / sizeof(uint32_t))

            return -1;

        nsv->nsvs_file_offset = av_malloc((unsigned)table_entries_used * sizeof(uint32_t));



        for(i=0;i<table_entries_used;i++)

            nsv->nsvs_file_offset[i] = avio_rl32(pb) + size;



        if(table_entries > table_entries_used &&

           avio_rl32(pb) == MKTAG('T','O','C','2')) {

            nsv->nsvs_timestamps = av_malloc((unsigned)table_entries_used*sizeof(uint32_t));

            for(i=0;i<table_entries_used;i++) {

                nsv->nsvs_timestamps[i] = avio_rl32(pb);

            }

        }

    }



    av_dlog(s, "NSV got index; filepos %"PRId64"\n", avio_tell(pb));



#ifdef DEBUG_DUMP_INDEX

#define V(v) ((v<0x20 || v > 127)?'.':v)

    

    av_dlog(s, "NSV %d INDEX ENTRIES:\n", table_entries);

    av_dlog(s, "NSV [dataoffset][fileoffset]\n", table_entries);

    for (i = 0; i < table_entries; i++) {

        unsigned char b[8];

        avio_seek(pb, size + nsv->nsvs_file_offset[i], SEEK_SET);

        avio_read(pb, b, 8);

        av_dlog(s, "NSV [0x%08lx][0x%08lx]: %02x %02x %02x %02x %02x %02x %02x %02x"

           "%c%c%c%c%c%c%c%c\n",

           nsv->nsvs_file_offset[i], size + nsv->nsvs_file_offset[i],

           b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],

           V(b[0]), V(b[1]), V(b[2]), V(b[3]), V(b[4]), V(b[5]), V(b[6]), V(b[7]) );

    }

    

#undef V

#endif



    avio_seek(pb, nsv->base_offset + size, SEEK_SET); 



    if (pb->eof_reached)

        return -1;

    nsv->state = NSV_HAS_READ_NSVF;

    return 0;

}