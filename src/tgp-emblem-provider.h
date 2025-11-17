/*
 * Thunar Git Plugin - Emblem Provider
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_EMBLEM_PROVIDER_H__
#define __TGP_EMBLEM_PROVIDER_H__

#include <glib.h>
#include <gio/gio.h>
#include "tgp-plugin.h"

G_BEGIN_DECLS

/* Emblem icon and status text retrieval */
const gchar* tgp_emblem_get_icon_name(TgpStatusFlags flags);
gchar*       tgp_emblem_get_status_text(TgpStatusFlags flags);

/* GVFS attribute setters */
void tgp_emblem_set_git_status_attribute(GFile *file, TgpStatusFlags flags, GError **error);
void tgp_emblem_set_git_status_on_file(const gchar *file_path, TgpStatusFlags flags);

/* GVFS attribute retrieval */
TgpStatusFlags tgp_emblem_get_git_status_attribute(GFile *file);

G_END_DECLS

#endif /* __TGP_EMBLEM_PROVIDER_H__ */
