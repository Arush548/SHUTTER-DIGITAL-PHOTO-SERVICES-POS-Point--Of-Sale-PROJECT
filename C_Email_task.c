#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    const char **payload_text = (const char **)userp;
    if (*payload_text) {
        size_t len = strlen(*payload_text);
        memcpy(ptr, *payload_text, len);
        *payload_text = NULL;
        return len;
    }
    return 0;
}

int SendEmailAuto(const char *to, const char *subject, const char *body) {
    CURL *curl;
    CURLcode res;

    // Sender credentials
    const char *from = "agrcatada@universityofbohol.edu.ph";
    const char *user = "agrcatada@universityofbohol.edu.ph";
    const char *app_password = "mxtk ghfk kxck gtnw";  // Your Gmail App Password

    curl = curl_easy_init();
    if (!curl)
        return 0;

    struct curl_slist *recipients = NULL;

    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password);

    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);

    recipients = curl_slist_append(recipients, to);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    char payload[3000];
    snprintf(payload, sizeof(payload),
             "To: %s\r\n"
             "From: %s\r\n"
             "Subject: %s\r\n"
             "\r\n"
             "%s\r\n",
             to, from, subject, body);

    const char *payload_ptr = payload;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload_ptr);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
