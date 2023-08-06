sosendoob(struct socket *so)

{

	struct sbuf *sb = &so->so_rcv;

	char buff[2048]; 



	int n, len;



	DEBUG_CALL("sosendoob");

	DEBUG_ARG("so = %p", so);

	DEBUG_ARG("sb->sb_cc = %d", sb->sb_cc);



	if (so->so_urgc > 2048)

	   so->so_urgc = 2048; 



	if (sb->sb_rptr < sb->sb_wptr) {

		

		n = slirp_send(so, sb->sb_rptr, so->so_urgc, (MSG_OOB)); 

		so->so_urgc -= n;



		DEBUG_MISC((dfd, " --- sent %d bytes urgent data, %d urgent bytes left\n", n, so->so_urgc));

	} else {

		

		len = (sb->sb_data + sb->sb_datalen) - sb->sb_rptr;

		if (len > so->so_urgc) len = so->so_urgc;

		memcpy(buff, sb->sb_rptr, len);

		so->so_urgc -= len;

		if (so->so_urgc) {

			n = sb->sb_wptr - sb->sb_data;

			if (n > so->so_urgc) n = so->so_urgc;

			memcpy((buff + len), sb->sb_data, n);

			so->so_urgc -= n;

			len += n;

		}

		n = slirp_send(so, buff, len, (MSG_OOB)); 

#ifdef DEBUG

		if (n != len)

		   DEBUG_ERROR((dfd, "Didn't send all data urgently XXXXX\n"));

#endif

		DEBUG_MISC((dfd, " ---2 sent %d bytes urgent data, %d urgent bytes left\n", n, so->so_urgc));

	}



	sb->sb_cc -= n;

	sb->sb_rptr += n;

	if (sb->sb_rptr >= (sb->sb_data + sb->sb_datalen))

		sb->sb_rptr -= sb->sb_datalen;



	return n;

}