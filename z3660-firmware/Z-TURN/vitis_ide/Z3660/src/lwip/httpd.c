#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcpbase.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "netif/xadapter.h"
#include "ff.h"

void print_hdmi_ln(int xpos, char *message, int line_inc);
#define printf2(N,...) do {char message[N]; \
    snprintf(message, N, __VA_ARGS__);\
     printf("%s\n",message); print_hdmi_ln(0,message,1); } while(0)

static struct tcp_pcb *http_pcb;
static FATFS fatfs;
static int fatfs_mounted = 0;

struct http_conn_state {
    int close_pending;
    int upload_active; /* 1 while receiving upload body */
    FIL upload_file;   /* file being written */
    uint32_t upload_remaining; /* bytes left to receive */
    char upload_path[128];
};

#ifndef HTTPD_TRACE
#define HTTPD_TRACE() printf2(64,"[httpd] %s", __func__)
#endif

const char *mime_type(const char *name);
static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
void http_send_file(struct tcp_pcb *pcb, char *request, struct http_conn_state *state);
void http_send_404(struct tcp_pcb *pcb);
void send_http_header(struct tcp_pcb *pcb, const char *filename);
void extract_filename(char *request, char *filename);
void http_send_json_system(struct tcp_pcb *pcb, struct http_conn_state *state);
void http_send_json_dirs(struct tcp_pcb *pcb, struct http_conn_state *state);

/* URL-decode in-place into dst, returns dst. Simple implementation. */
static char *url_decode(char *dst, const char *src)
{
    char *o = dst;
    while (*src) {
        if (*src == '+') {
            *o++ = ' ';
            src++;
        } else if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], 0 };
            *o++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else {
            *o++ = *src++;
        }
    }
    *o = '\0';
    return dst;
}

static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    (void)len;
    struct http_conn_state *state = (struct http_conn_state *)arg;
    if (!state) {
        printf2(64,"[httpd] %s: no state", __func__);
        return ERR_OK;
    }
    printf2(64,"[httpd] %s: called, close_pending=%d", __func__, state->close_pending);
    if (state->close_pending) {
        err_t err = tcp_close(pcb);
        if (err == ERR_OK) {
            printf2(64,"[httpd] %s: tcp_close succeeded", __func__);
            tcp_arg(pcb, NULL);
            free(state);
        } else {
            printf2(64,"[httpd] %s: tcp_close still failed (%d)", __func__, err);
        }
    }
    return ERR_OK;
}

err_t http_recv(void *arg,
                struct tcp_pcb *pcb,
                struct pbuf *p,
                err_t err)
{
    struct http_conn_state *state = (struct http_conn_state *)arg;
    (void)err;
//    HTTPD_TRACE();
    if (p == NULL)
    {
        printf2(64,"[httpd] %s: p == NULL, cleaning up state", __func__);
        if (state) {
            free(state);
            tcp_arg(pcb, NULL);
        }
        printf2(64,"[httpd] %s: return ERR_OK", __func__);
        return ERR_OK;
    }

    char *data = (char *)p->payload;

    /* If an upload is in progress, write the received payload to file */
    if (state && state->upload_active) {
        size_t to_write = p->tot_len;
        if (to_write > state->upload_remaining) to_write = state->upload_remaining;
        UINT bw;
        FRESULT fres = f_write(&state->upload_file, data, to_write, &bw);
        if (fres != FR_OK || bw != to_write) {
            printf2(64,"[httpd] %s: f_write error during upload %d", __func__, (int)fres);
            /* abort upload */
            f_close(&state->upload_file);
            state->upload_active = 0;
            state->upload_remaining = 0;
            http_send_404(pcb);
        } else {
            state->upload_remaining -= bw;
            // This printf spams the console, so reduce its frequency
            static int upload_log_counter = 0;
            upload_log_counter++;
            if (upload_log_counter % 100 == 0) {
            printf2(64,"[httpd] %s: wrote %u bytes, remaining %lu", __func__, bw, (unsigned long)state->upload_remaining);
            }
            if (state->upload_remaining == 0) {
                /* finish upload */
                f_close(&state->upload_file);
                state->upload_active = 0;
                /* send success response */
                const char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nUPLOAD OK";
                tcp_write(pcb, msg, strlen(msg), TCP_WRITE_FLAG_COPY);
                tcp_output(pcb);
                state->close_pending = 1;
                printf2(1000,"[httpd] %s: upload finished %s", __func__, state->upload_path);
            }
        }
        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
//        printf2(64,"[httpd] %s: return ERR_OK (upload path)", __func__);
        return ERR_OK;
    }

    if (!strncmp(data, "GET ", 4))
    {
        printf2(64,"[httpd] %s: GET request", __func__);
        http_send_file(pcb, data, state);
    }

    else if (!strncmp(data, "PUT ", 4)) {
        /* Handle simple PUT /upload/<filename> HTTP/1.1\r\n...\r\n<binary> */
        printf2(64,"[httpd] %s: PUT request", __func__);
        /* extract path */
        char filename[256];
        extract_filename(data, filename);
        /* If there is a query string, split it off */
        char *q = strchr(filename, '?');
        char query[256] = {0};
        if (q) {
            strncpy(query, q+1, sizeof(query)-1);
            *q = '\0';
        }
        /* expect /upload/<name> */
        const char *pfx = "/upload/";
        if (strncmp(filename, pfx, strlen(pfx)) != 0) {
            printf2(1000,"[httpd] %s: PUT to unsupported path %s", __func__, filename);
            http_send_404(pcb);
        } else {
            const char *name = filename + strlen(pfx);
            if (*name == '\0') {
                printf2(64,"[httpd] %s: PUT missing filename", __func__);
                http_send_404(pcb);
            } else {
                /* parse path= in query if provided */
                char dest[257] = {0};
                if (query[0]) {
                    /* look for path= value */
                    char *p = strstr(query, "path=");
                    if (p) {
                        p += 5;
                        char decoded[256];
                        url_decode(decoded, p);
                        strncpy(dest, decoded, sizeof(dest)-1);
                    }
                }
                /* default destination if none provided */
                if (dest[0] == '\0') strncpy(dest, "1:/web", sizeof(dest)-1);

                /* validate dest: must start with '0:' or '1:' */
                if (!(dest[0] == '0' && dest[1] == ':') && !(dest[0] == '1' && dest[1] == ':')) {
                    printf2(64,"[httpd] %s: invalid destination %s", __func__, dest);
                    http_send_404(pcb);
                } else {
                    /* ensure directory exists */
                    FRESULT st = f_stat(dest, NULL);
                    if (st != FR_OK) {
                        /* try to create */
                        f_mkdir(dest);
                    }
                    char fullpath[512];
                    /* join dest and name */
                    if (dest[strlen(dest)-1] == '/')
                        snprintf(fullpath, sizeof(fullpath), "%s%s", dest, name);
                    else
                        snprintf(fullpath, sizeof(fullpath), "%s/%s", dest, name);
                    /* try to find Content-Length header */
                    char *cl = strstr(data, "Content-Length:");
                uint32_t content_len = 0;
                if (cl) {
                    cl += strlen("Content-Length:");
                    while (*cl == ' ') cl++;
                    content_len = (uint32_t)atoi(cl);
                } else {
                    printf2(64,"[httpd] %s: no Content-Length header", __func__);
                }
                /* open file for write */
                FRESULT fres = f_open(&state->upload_file, fullpath, FA_CREATE_ALWAYS | FA_WRITE);
                if (fres != FR_OK) {
                    printf2(1000,"[httpd] %s: f_open upload failed %s: %d", __func__, fullpath, (int)fres);
                    http_send_404(pcb);
                } else {
                    printf2(1000,"[httpd] %s: upload start %s, expecting %lu bytes", __func__, fullpath, (unsigned long)content_len);
                    state->upload_active = 1;
                    state->upload_remaining = content_len;
                    strncpy(state->upload_path, fullpath, sizeof(state->upload_path)-1);
                    state->upload_path[sizeof(state->upload_path)-1] = '\0';
                    /* find end of headers */
                    char *body = strstr(data, "\r\n\r\n");
                    if (body) {
                        body += 4;
                        size_t body_len = (char *)p->payload + p->tot_len - body;
                        if (body_len > 0) {
                            UINT bw;
                            FRESULT fres2 = f_write(&state->upload_file, body, body_len, &bw);
                            if (fres2 != FR_OK) {
                                printf2(64,"[httpd] %s: f_write initial body failed %d", __func__, (int)fres2);
                                f_close(&state->upload_file);
                                state->upload_active = 0;
                                http_send_404(pcb);
                            } else {
                                if (bw > state->upload_remaining) bw = state->upload_remaining;
                                state->upload_remaining -= bw;
                                printf2(64,"[httpd] %s: wrote initial %u bytes, remaining %lu", __func__, bw, (unsigned long)state->upload_remaining);
                                if (state->upload_remaining == 0) {
                                    /* finished within first packet */
                                    f_close(&state->upload_file);
                                    state->upload_active = 0;
                                    const char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nUPLOAD OK";
                                    tcp_write(pcb, msg, strlen(msg), TCP_WRITE_FLAG_COPY);
                                    tcp_output(pcb);
                                    state->close_pending = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

    tcp_recved(pcb, p->tot_len);
    printf2(64,"[httpd] %s: tcp_recved %d", __func__, p->tot_len);
    pbuf_free(p);

    printf2(64,"[httpd] %s: return ERR_OK", __func__);
    return ERR_OK;
}

void extract_filename(char *request,
                      char *filename)
{
    HTTPD_TRACE();
    char *start = request + 4;

    char *end = strchr(start, ' ');

    int len = end - start;

    memcpy(filename, start, len);

    filename[len] = 0;

    if (!strcmp(filename, "/")) {
        printf2(64,"[httpd] %s: root request, using /index.html", __func__);
        strcpy(filename, "/index.html");
    }
}
char ip_address[30]={0};
char firmware[300]={0};
void http_send_json_system(struct tcp_pcb *pcb, struct http_conn_state *state)
{
    HTTPD_TRACE();
    DWORD free_clusters;
    FATFS *fs = &fatfs;
    FRESULT fres = f_getfree("1:", &free_clusters, &fs);
    
    unsigned long free_bytes = 0;
    if (fres == FR_OK) {
        free_bytes = free_clusters * fs->csize * 512;  /* csize is cluster size in sectors */
    }
    
    char json_buf[512];
    int json_len = snprintf(json_buf, sizeof(json_buf),
        "{"
        "\"firmware\":\"%s\","
        "\"ip\":\"%s\","
        "\"free_mb\":%lu,"
        "\"sd_mounted\":1"
        "}", firmware, ip_address,
        free_bytes / (1024 * 1024));
    
    if (json_len < 0 || json_len >= (int)sizeof(json_buf)) {
        printf2(64,"[httpd] %s: json snprintf failed", __func__);
        return;
    }
    
    /* Enviar cabecera HTTP con Content-Type JSON */
    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n");
    
    err_t err = tcp_write(pcb, header, header_len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf2(64,"[httpd] %s: header write failed", __func__);
        return;
    }
    
    /* Enviar JSON */
    err = tcp_write(pcb, json_buf, json_len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf2(64,"[httpd] %s: json write failed", __func__);
        return;
    }
    
    tcp_output(pcb);
    printf2(100,"[httpd] %s: sent json system info (%d bytes)", __func__, json_len);
    printf2(1000,"jsonn %s",json_buf);
    
    if (state) {
        state->close_pending = 1;
    }
    tcp_output(pcb);
}

void http_send_json_dirs(struct tcp_pcb *pcb, struct http_conn_state *state)
{
    HTTPD_TRACE();
    /* Candidate paths we expose to the user. Adjust as needed. */
    const char *candidates[] = { "1:/web",
                                 "1:/web/locales",
                                 "1:/hdf",
                                 "1:/kicks",
                                 "1:/presets",
                                 "0:",
                                 "1:" };
    const int ncand = sizeof(candidates)/sizeof(candidates[0]);

    char json[512];
    int off = 0;
    off += snprintf(json+off, sizeof(json)-off, "{\"dirs\":[");
    int first = 1;
    for (int i = 0; i < ncand; ++i) {
        const char *p = candidates[i];
        FILINFO fi;
        FRESULT res = f_stat(p, &fi);
        /* For root paths (0:/, 1:/) always include them even if f_stat fails */
        int is_root = (strcmp(p, "0:/") == 0 || strcmp(p, "1:/") == 0);
        if (res == FR_OK || is_root) {
            if (!first) off += snprintf(json+off, sizeof(json)-off, ",");
            off += snprintf(json+off, sizeof(json)-off, "\"%s\"", p);
            first = 0;
        } else {
            /* try to create uploads dir if candidate ends with /uploads */
            if (strstr(p, "/uploads") != NULL) {
                FRESULT mres = f_mkdir(p);
                if (mres == FR_OK) {
                    if (!first) off += snprintf(json+off, sizeof(json)-off, ",");
                    off += snprintf(json+off, sizeof(json)-off, "\"%s\"", p);
                    first = 0;
                }
            }
        }
    }
    off += snprintf(json+off, sizeof(json)-off, "]}\n");
    printf("json dirs:\n%s\n", json);

    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n");
    tcp_write(pcb, header, header_len, TCP_WRITE_FLAG_COPY);
    tcp_write(pcb, json, off, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    if (state) state->close_pending = 1;
}
/*
const char *mime_type(const char *name)
{
    const char *dot = strrchr(name, '.');
    if (!dot) return "application/octet-stream";
    if (!strcmp(dot, ".html") || !strcmp(dot, ".htm")) return "text/html";
    if (!strcmp(dot, ".css")) return "text/css";
    if (!strcmp(dot, ".js")) return "application/javascript";
    if (!strcmp(dot, ".png")) return "image/png";
    if (!strcmp(dot, ".jpg") || !strcmp(dot, ".jpeg")) return "image/jpeg";
    if (!strcmp(dot, ".gif")) return "image/gif";
    if (!strcmp(dot, ".svg")) return "image/svg+xml";
    return "application/octet-stream";
}
*/
void http_send_file(struct tcp_pcb *pcb, char *request, struct http_conn_state *state)
{
    HTTPD_TRACE();
    if (!fatfs_mounted) {
        printf2(64,"[httpd] %s: fatfs not mounted", __func__);
        http_send_404(pcb);
        return;
    }

    char filename[128];
    extract_filename(request, filename);
    
    /* Check for API endpoints */
    if (!strcmp(filename, "/api/system")) {
        printf2(64,"[httpd] %s: api/system request", __func__);
        http_send_json_system(pcb, state);
        return;
    }
    if (!strcmp(filename, "/api/dirs")) {
        printf2(64,"[httpd] %s: api/dirs request", __func__);
        http_send_json_dirs(pcb, state);
        return;
    }
    
    char fullpath[256];
    sprintf(fullpath, "1:/web%s", filename);
    FIL file;
    FRESULT fres = f_open(&file, fullpath, FA_READ);
    if (fres != FR_OK)
    {
        printf2(1000,"[httpd] %s: f_open failed for %s: %d", __func__, fullpath, (int)fres);
        http_send_404(pcb);
        printf2(64,"[httpd] %s: returning after 404", __func__);
        return;
    }
    else {
        printf2(1000,"[httpd] %s: f_open succeeded for %s", __func__, fullpath);
    }
    /* Enviar cabecera HTTP */
    send_http_header(pcb, fullpath);
    err_t out_err = tcp_output(pcb);
    if (out_err != ERR_OK) {
        printf2(64,"[httpd] %s: tcp_output returned %d after header", __func__, out_err);
    }
    BYTE buffer[1024];
    UINT br;
    while (1)
    {
        if (f_read(&file, buffer, sizeof(buffer), &br) != FR_OK)
        {
            printf2(64,"[httpd] %s: f_read error", __func__);
            break;
        }
        if (br == 0)
            break;
        err_t write_err = tcp_write(pcb, buffer, br, TCP_WRITE_FLAG_COPY);
        if (write_err != ERR_OK) {
            printf2(64,"[httpd] %s: tcp_write failed (%d) for %u bytes", __func__, write_err, br);
            break;
        }
        printf2(64,"[httpd] %s: tcp_write ok %u bytes", __func__, br);
        err_t out_err1 = tcp_output(pcb);
        if (out_err1 != ERR_OK) {
            printf2(100,"[httpd] %s: tcp_output returned %d after body chunk", __func__, out_err1);
        }
    }

    f_close(&file);

    printf2(1000,"[httpd] %s: finished sending file %s", __func__, fullpath);
    if (state) {
        state->close_pending = 1;
        printf2(64,"[httpd] %s: marked connection pending close", __func__);
    }
    err_t out_err2 = tcp_output(pcb);
    if (out_err2 != ERR_OK) {
        printf2(64,"[httpd] %s: tcp_output returned %d", __func__, out_err2);
    }
}
void send_http_header(struct tcp_pcb *pcb, const char *filename)
{
    HTTPD_TRACE();
    const char *type = mime_type(filename);
    char header[256];
    int len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r"
        "Content-Type: %s\r"
        "Connection: close\r"
        "\r\n",
        type);
    if (len < 0 || len >= (int)sizeof(header)) {
        printf2(64,"[httpd] %s: header snprintf failed", __func__);
        return;
    }

    err_t header_err = tcp_write(pcb,
              header,
              len,
              TCP_WRITE_FLAG_COPY);
    if (header_err != ERR_OK) {
        printf2(64,"[httpd] %s: tcp_write(header) failed %d", __func__, header_err);
    } else {
        printf2(64,"[httpd] %s: tcp_write(header) ok %d", __func__, len);
    }
}

void http_send_404(struct tcp_pcb *pcb)
{
    HTTPD_TRACE();
    static const char msg[] =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<html><body><h1>404</h1></body></html>";

    tcp_write(pcb,
              msg,
              strlen(msg),
              TCP_WRITE_FLAG_COPY);

    tcp_output(pcb);

    printf2(64,"[httpd] %s: closing pcb after 404", __func__);
    tcp_close(pcb);
}

const char *mime_type(const char *name)
{
    HTTPD_TRACE();
    const char *ext = strrchr(name, '.');

    if (!ext) {
        printf2(64,"[httpd] %s: no extension for %s", __func__, name);
        return "application/octet-stream";
    }

    if (!strcmp(ext, ".html")) {
        printf2(64,"[httpd] %s: .html -> text/html", __func__);
        return "text/html";
    }

    if (!strcmp(ext, ".css")) {
        printf2(64,"[httpd] %s: .css -> text/css", __func__);
        return "text/css";
    }

    if (!strcmp(ext, ".js")) {
        printf2(64,"[httpd] %s: .js -> application/javascript", __func__);
        return "application/javascript";
    }

    if (!strcmp(ext, ".png")) {
        printf2(64,"[httpd] %s: .png -> image/png", __func__);
        return "image/png";
    }

    if (!strcmp(ext, ".jpg")) {
        printf2(64,"[httpd] %s: .jpg -> image/jpeg", __func__);
        return "image/jpeg";
    }

    if (!strcmp(ext, ".gif")) {
        printf2(64,"[httpd] %s: .gif -> image/gif", __func__);
        return "image/gif";
    }

    if (!strcmp(ext, ".svg")) {
        printf2(64,"[httpd] %s: .svg -> image/svg+xml", __func__);
        return "image/svg+xml";
    }

    printf2(64,"[httpd] %s: unknown ext %s -> octet-stream", __func__, ext);
    return "application/octet-stream";
}


err_t http_accept(void *arg,
                  struct tcp_pcb *newpcb,
                  err_t err)
{
    (void)arg;
    (void)err;
    HTTPD_TRACE();
    printf2(64,"[httpd] %s: new pcb %p", __func__, (void *)newpcb);
    struct http_conn_state *state = malloc(sizeof(*state));
    if (!state) {
        printf2(64,"[httpd] %s: malloc failed, closing pcb", __func__);
        tcp_close(newpcb);
        return ERR_MEM;
    }
    memset(state, 0, sizeof(*state));
    state->close_pending = 0;
    tcp_arg(newpcb, state);
    tcp_sent(newpcb, http_sent);
    tcp_recv(newpcb, http_recv);

    printf2(64,"[httpd] %s: return ERR_OK", __func__);
    return ERR_OK;
}

void http_server_init(void)
{
    HTTPD_TRACE();

    if (!fatfs_mounted) {
        FRESULT res = f_mount(&fatfs, "1:", 1);
        if (res != FR_OK) {
            printf2(64,"[httpd] f_mount failed: %d", (int)res);
        } else {
            fatfs_mounted = 1;
            printf2(64, "[httpd] FATFS mounted on 1:");
        }
    }

    http_pcb = tcp_new();

    printf2(64, "[httpd] %s: binding to port 80", __func__);
    tcp_bind(http_pcb, IP_ADDR_ANY, 80);

    http_pcb = tcp_listen(http_pcb);

    tcp_accept(http_pcb, http_accept);
}

extern struct netif server_netif;
void start_httpd(void)
{
    HTTPD_TRACE();
    http_server_init();
}
void run_httpd(void)
{
//    HTTPD_TRACE();
    xemacif_input(&server_netif);
    sys_check_timeouts();
}