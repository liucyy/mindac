static int http_connect(URLContext *h, const char *path, const char *local_path,

                        const char *hoststr, const char *auth,

                        const char *proxyauth, int *new_location)

{

    HTTPContext *s = h->priv_data;

    int post, err;

    char headers[HTTP_HEADERS_SIZE] = "";

    char *authstr = NULL, *proxyauthstr = NULL;

    uint64_t off = s->off;

    int len = 0;

    const char *method;

    int send_expect_100 = 0;



    

    post = h->flags & AVIO_FLAG_WRITE;



    if (s->post_data) {

        

        post            = 1;

        s->chunked_post = 0;

    }



    if (s->method)

        method = s->method;

    else

        method = post ? "POST" : "GET";



    authstr      = ff_http_auth_create_response(&s->auth_state, auth,

                                                local_path, method);

    proxyauthstr = ff_http_auth_create_response(&s->proxy_auth_state, proxyauth,

                                                local_path, method);

    if (post && !s->post_data) {

        send_expect_100 = s->send_expect_100;

        

        if (auth && *auth &&

            s->auth_state.auth_type == HTTP_AUTH_NONE &&

            s->http_code != 401)

            send_expect_100 = 1;

    }



#if FF_API_HTTP_USER_AGENT

    if (strcmp(s->user_agent_deprecated, DEFAULT_USER_AGENT)) {

        av_log(s, AV_LOG_WARNING, "the user-agent option is deprecated, please use user_agent option\n");

        s->user_agent = av_strdup(s->user_agent_deprecated);

    }

#endif

    

    if (!has_header(s->headers, "\r\nUser-Agent: "))

        len += av_strlcatf(headers + len, sizeof(headers) - len,

                           "User-Agent: %s\r\n", s->user_agent);

    if (!has_header(s->headers, "\r\nAccept: "))

        len += av_strlcpy(headers + len, "Accept: *

    if (s->headers)

        av_strlcpy(headers + len, s->headers, sizeof(headers) - len);



    snprintf(s->buffer, sizeof(s->buffer),

             "%s %s HTTP/1.1\r\n"

             "%s"

             "%s"

             "%s"

             "%s%s"

             "\r\n",

             method,

             path,

             post && s->chunked_post ? "Transfer-Encoding: chunked\r\n" : "",

             headers,

             authstr ? authstr : "",

             proxyauthstr ? "Proxy-" : "", proxyauthstr ? proxyauthstr : "");



    av_log(h, AV_LOG_DEBUG, "request: %s\n", s->buffer);



    if ((err = ffurl_write(s->hd, s->buffer, strlen(s->buffer))) < 0)

        goto done;



    if (s->post_data)

        if ((err = ffurl_write(s->hd, s->post_data, s->post_datalen)) < 0)

            goto done;



    

    s->buf_ptr          = s->buffer;

    s->buf_end          = s->buffer;

    s->line_count       = 0;

    s->off              = 0;

    s->icy_data_read    = 0;

    s->filesize         = UINT64_MAX;

    s->willclose        = 0;

    s->end_chunked_post = 0;

    s->end_header       = 0;

    if (post && !s->post_data && !send_expect_100) {

        

        s->http_code = 200;

        err = 0;

        goto done;

    }



    

    err = http_read_header(h, new_location);

    if (err < 0)

        goto done;



    if (*new_location)

        s->off = off;



    err = (off == s->off) ? 0 : -1;

done:

    av_freep(&authstr);

    av_freep(&proxyauthstr);

    return err;

}