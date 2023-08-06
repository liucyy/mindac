static CURLState *curl_init_state(BDRVCURLState *s)

{

    CURLState *state = NULL;

    int i, j;



    do {

        for (i=0; i<CURL_NUM_STATES; i++) {

            for (j=0; j<CURL_NUM_ACB; j++)

                if (s->states[i].acb[j])

                    continue;

            if (s->states[i].in_use)

                continue;



            state = &s->states[i];

            state->in_use = 1;

            break;

        }

        if (!state) {

            qemu_aio_wait();

        }

    } while(!state);



    if (!state->curl) {

        state->curl = curl_easy_init();

        if (!state->curl) {

            return NULL;

        }

        curl_easy_setopt(state->curl, CURLOPT_URL, s->url);

        curl_easy_setopt(state->curl, CURLOPT_SSL_VERIFYPEER,

                         (long) s->sslverify);

        curl_easy_setopt(state->curl, CURLOPT_TIMEOUT, 5);

        curl_easy_setopt(state->curl, CURLOPT_WRITEFUNCTION,

                         (void *)curl_read_cb);

        curl_easy_setopt(state->curl, CURLOPT_WRITEDATA, (void *)state);

        curl_easy_setopt(state->curl, CURLOPT_PRIVATE, (void *)state);

        curl_easy_setopt(state->curl, CURLOPT_AUTOREFERER, 1);

        curl_easy_setopt(state->curl, CURLOPT_FOLLOWLOCATION, 1);

        curl_easy_setopt(state->curl, CURLOPT_NOSIGNAL, 1);

        curl_easy_setopt(state->curl, CURLOPT_ERRORBUFFER, state->errmsg);

        curl_easy_setopt(state->curl, CURLOPT_FAILONERROR, 1);



        

#if LIBCURL_VERSION_NUM >= 0x071304

        curl_easy_setopt(state->curl, CURLOPT_PROTOCOLS, PROTOCOLS);

        curl_easy_setopt(state->curl, CURLOPT_REDIR_PROTOCOLS, PROTOCOLS);

#endif



#ifdef DEBUG_VERBOSE

        curl_easy_setopt(state->curl, CURLOPT_VERBOSE, 1);

#endif

    }



    state->s = s;



    return state;

}