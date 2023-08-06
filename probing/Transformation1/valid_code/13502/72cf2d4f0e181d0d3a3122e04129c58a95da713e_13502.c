static int net_slirp_init(Monitor *mon, VLANState *vlan, const char *model,

                          const char *name, int restricted,

                          const char *vnetwork, const char *vhost,

                          const char *vhostname, const char *tftp_export,

                          const char *bootfile, const char *vdhcp_start,

                          const char *vnameserver, const char *smb_export,

                          const char *vsmbserver)

{

    

    struct in_addr net  = { .s_addr = htonl(0x0a000200) }; 

    struct in_addr mask = { .s_addr = htonl(0xffffff00) }; 

    struct in_addr host = { .s_addr = htonl(0x0a000202) }; 

    struct in_addr dhcp = { .s_addr = htonl(0x0a00020f) }; 

    struct in_addr dns  = { .s_addr = htonl(0x0a000203) }; 

#ifndef _WIN32

    struct in_addr smbsrv = { .s_addr = 0 };

#endif

    SlirpState *s;

    char buf[20];

    uint32_t addr;

    int shift;

    char *end;



    if (!tftp_export) {

        tftp_export = legacy_tftp_prefix;

    }

    if (!bootfile) {

        bootfile = legacy_bootp_filename;

    }



    if (vnetwork) {

        if (get_str_sep(buf, sizeof(buf), &vnetwork, '/') < 0) {

            if (!inet_aton(vnetwork, &net)) {

                return -1;

            }

            addr = ntohl(net.s_addr);

            if (!(addr & 0x80000000)) {

                mask.s_addr = htonl(0xff000000); 

            } else if ((addr & 0xfff00000) == 0xac100000) {

                mask.s_addr = htonl(0xfff00000); 

            } else if ((addr & 0xc0000000) == 0x80000000) {

                mask.s_addr = htonl(0xffff0000); 

            } else if ((addr & 0xffff0000) == 0xc0a80000) {

                mask.s_addr = htonl(0xffff0000); 

            } else if ((addr & 0xffff0000) == 0xc6120000) {

                mask.s_addr = htonl(0xfffe0000); 

            } else if ((addr & 0xe0000000) == 0xe0000000) {

                mask.s_addr = htonl(0xffffff00); 

            } else {

                mask.s_addr = htonl(0xfffffff0); 

            }

        } else {

            if (!inet_aton(buf, &net)) {

                return -1;

            }

            shift = strtol(vnetwork, &end, 10);

            if (*end != '\0') {

                if (!inet_aton(vnetwork, &mask)) {

                    return -1;

                }

            } else if (shift < 4 || shift > 32) {

                return -1;

            } else {

                mask.s_addr = htonl(0xffffffff << (32 - shift));

            }

        }

        net.s_addr &= mask.s_addr;

        host.s_addr = net.s_addr | (htonl(0x0202) & ~mask.s_addr);

        dhcp.s_addr = net.s_addr | (htonl(0x020f) & ~mask.s_addr);

        dns.s_addr  = net.s_addr | (htonl(0x0203) & ~mask.s_addr);

    }



    if (vhost && !inet_aton(vhost, &host)) {

        return -1;

    }

    if ((host.s_addr & mask.s_addr) != net.s_addr) {

        return -1;

    }



    if (vdhcp_start && !inet_aton(vdhcp_start, &dhcp)) {

        return -1;

    }

    if ((dhcp.s_addr & mask.s_addr) != net.s_addr ||

        dhcp.s_addr == host.s_addr || dhcp.s_addr == dns.s_addr) {

        return -1;

    }



    if (vnameserver && !inet_aton(vnameserver, &dns)) {

        return -1;

    }

    if ((dns.s_addr & mask.s_addr) != net.s_addr ||

        dns.s_addr == host.s_addr) {

        return -1;

    }



#ifndef _WIN32

    if (vsmbserver && !inet_aton(vsmbserver, &smbsrv)) {

        return -1;

    }

#endif



    s = qemu_mallocz(sizeof(SlirpState));

    s->slirp = slirp_init(restricted, net, mask, host, vhostname,

                          tftp_export, bootfile, dhcp, dns, s);

    TAILQ_INSERT_TAIL(&slirp_stacks, s, entry);



    while (slirp_configs) {

        struct slirp_config_str *config = slirp_configs;



        if (config->flags & SLIRP_CFG_HOSTFWD) {

            slirp_hostfwd(s, mon, config->str,

                          config->flags & SLIRP_CFG_LEGACY);

        } else {

            slirp_guestfwd(s, mon, config->str,

                           config->flags & SLIRP_CFG_LEGACY);

        }

        slirp_configs = config->next;

        qemu_free(config);

    }

#ifndef _WIN32

    if (!smb_export) {

        smb_export = legacy_smb_export;

    }

    if (smb_export) {

        slirp_smb(s, mon, smb_export, smbsrv);

    }

#endif



    s->vc = qemu_new_vlan_client(vlan, model, name, NULL, slirp_receive, NULL,

                                 net_slirp_cleanup, s);

    snprintf(s->vc->info_str, sizeof(s->vc->info_str),

             "net=%s, restricted=%c", inet_ntoa(net), restricted ? 'y' : 'n');

    return 0;

}