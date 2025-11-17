/*
 * Thunar Git Plugin - Credential Management
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_CREDENTIALS_H__
#define __TGP_CREDENTIALS_H__

#include <glib.h>
#include <git2.h>

G_BEGIN_DECLS

/* Credential types */
typedef enum {
    TGP_CREDENTIAL_USERPASS,      /* Username/password for HTTPS */
    TGP_CREDENTIAL_SSH_KEY,       /* SSH public key */
    TGP_CREDENTIAL_SSH_PASSPHRASE, /* SSH key passphrase */
    TGP_CREDENTIAL_TOKEN          /* Personal access token */
} TgpCredentialType;

/* Credential storage structure */
typedef struct {
    gchar *host;
    gchar *username;
    gchar *password;
    gchar *token;
    gint   port;
    guint  expires;  /* Unix timestamp, 0 = never expires */
} TgpCredential;

/* Initialize credential system */
void tgp_credentials_init(void);

/* Cleanup credential system */
void tgp_credentials_cleanup(void);

/* Store credentials */
void tgp_credentials_store(const gchar *host, const gchar *username,
                           const gchar *password, guint timeout_seconds);

/* Retrieve stored credentials */
gboolean tgp_credentials_get(const gchar *host, gchar **username, gchar **password);

/* Remove credentials */
void tgp_credentials_remove(const gchar *host);

/* Clear all stored credentials */
void tgp_credentials_clear_all(void);

/* Check if credentials exist for a host */
gboolean tgp_credentials_exists(const gchar *host);

/* Libgit2 credential callback */
int tgp_git_credentials_callback(git_credential **out, const char *url,
                                  const char *username_from_url,
                                  unsigned int allowed_types,
                                  void *payload);

G_END_DECLS

#endif /* __TGP_CREDENTIALS_H__ */
