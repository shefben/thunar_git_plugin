/*
 * Thunar Git Plugin - Credential Management Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-credentials.h"
#include <string.h>
#include <time.h>

/* Global credential storage */
static GHashTable *credentials_store = NULL;
static GMutex credentials_mutex;

/* Credential entry structure */
typedef struct {
    gchar *username;
    gchar *password;
    guint expires;  /* 0 = never, otherwise unix timestamp */
} CredentialEntry;

static CredentialEntry*
credential_entry_new(const gchar *username, const gchar *password, guint timeout_seconds)
{
    CredentialEntry *entry = g_new0(CredentialEntry, 1);
    entry->username = g_strdup(username);
    entry->password = g_strdup(password);

    if (timeout_seconds > 0)
        entry->expires = (guint)time(NULL) + timeout_seconds;
    else
        entry->expires = 0;  /* Never expires */

    return entry;
}

static void
credential_entry_free(CredentialEntry *entry)
{
    if (!entry)
        return;

    if (entry->username)
    {
        /* Clear password from memory before freeing */
        memset(entry->username, 0, strlen(entry->username));
        g_free(entry->username);
    }

    if (entry->password)
    {
        memset(entry->password, 0, strlen(entry->password));
        g_free(entry->password);
    }

    g_free(entry);
}

static gboolean
credential_entry_is_valid(CredentialEntry *entry)
{
    if (!entry)
        return FALSE;

    if (entry->expires == 0)
        return TRUE;  /* Never expires */

    return time(NULL) < (time_t)entry->expires;
}

void
tgp_credentials_init(void)
{
    g_mutex_lock(&credentials_mutex);

    if (!credentials_store)
    {
        credentials_store = g_hash_table_new_full(
            g_str_hash, g_str_equal,
            g_free, (GDestroyNotify)credential_entry_free
        );
    }

    g_mutex_unlock(&credentials_mutex);
}

void
tgp_credentials_cleanup(void)
{
    g_mutex_lock(&credentials_mutex);

    if (credentials_store)
    {
        g_hash_table_destroy(credentials_store);
        credentials_store = NULL;
    }

    g_mutex_unlock(&credentials_mutex);
}

void
tgp_credentials_store(const gchar *host, const gchar *username,
                      const gchar *password, guint timeout_seconds)
{
    CredentialEntry *entry;

    if (!host || !username || !password)
        return;

    tgp_credentials_init();

    g_mutex_lock(&credentials_mutex);

    entry = credential_entry_new(username, password, timeout_seconds);
    g_hash_table_insert(credentials_store, g_strdup(host), entry);

    g_mutex_unlock(&credentials_mutex);

    g_message("Stored credentials for host: %s (username: %s, expires in: %u seconds)",
              host, username, timeout_seconds);
}

gboolean
tgp_credentials_get(const gchar *host, gchar **username, gchar **password)
{
    CredentialEntry *entry;
    gboolean result = FALSE;

    if (!host || !username || !password)
        return FALSE;

    if (!credentials_store)
        return FALSE;

    g_mutex_lock(&credentials_mutex);

    entry = g_hash_table_lookup(credentials_store, host);

    if (entry && credential_entry_is_valid(entry))
    {
        *username = g_strdup(entry->username);
        *password = g_strdup(entry->password);
        result = TRUE;
    }
    else if (entry)
    {
        /* Expired - remove it */
        g_hash_table_remove(credentials_store, host);
    }

    g_mutex_unlock(&credentials_mutex);

    return result;
}

void
tgp_credentials_remove(const gchar *host)
{
    if (!host || !credentials_store)
        return;

    g_mutex_lock(&credentials_mutex);
    g_hash_table_remove(credentials_store, host);
    g_mutex_unlock(&credentials_mutex);

    g_message("Removed credentials for host: %s", host);
}

void
tgp_credentials_clear_all(void)
{
    if (!credentials_store)
        return;

    g_mutex_lock(&credentials_mutex);
    g_hash_table_remove_all(credentials_store);
    g_mutex_unlock(&credentials_mutex);

    g_message("Cleared all stored credentials");
}

gboolean
tgp_credentials_exists(const gchar *host)
{
    CredentialEntry *entry;
    gboolean result = FALSE;

    if (!host || !credentials_store)
        return FALSE;

    g_mutex_lock(&credentials_mutex);

    entry = g_hash_table_lookup(credentials_store, host);

    if (entry && credential_entry_is_valid(entry))
        result = TRUE;
    else if (entry)
        g_hash_table_remove(credentials_store, host);

    g_mutex_unlock(&credentials_mutex);

    return result;
}

/*
 * Libgit2 credential callback
 * This is called when git operations need authentication
 * We first try stored credentials, then fall back to user interaction
 */
int
tgp_git_credentials_callback(git_credential **out, const char *url,
                              const char *username_from_url,
                              unsigned int allowed_types,
                              void *payload)
{
    gchar *host = NULL;
    gchar *username = NULL;
    gchar *password = NULL;
    int error = GIT_PASSTHROUGH;  /* Default: let libgit2 handle it */

    (void)username_from_url;
    (void)payload;

    if (!url || !out)
        return GIT_PASSTHROUGH;

    /* Extract host from URL */
    if (g_str_has_prefix(url, "https://"))
        host = g_strdup(url + 8);
    else if (g_str_has_prefix(url, "http://"))
        host = g_strdup(url + 7);
    else
        return GIT_PASSTHROUGH;

    /* Remove path from host */
    gchar *slash = strchr(host, '/');
    if (slash)
        *slash = '\0';

    /* Try to get stored credentials */
    if (tgp_credentials_get(host, &username, &password))
    {
        if ((allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) &&
            username && password)
        {
            error = git_credential_userpass_plaintext_new(out, username, password);
            g_message("Using stored credentials for %s", host);
        }
    }

    g_free(username);
    g_free(password);
    g_free(host);

    return error;
}
