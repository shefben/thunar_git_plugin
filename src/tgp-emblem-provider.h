/*
 * Thunar Git Plugin - Emblem Provider
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_EMBLEM_PROVIDER_H__
#define __TGP_EMBLEM_PROVIDER_H__

#include <glib.h>
#include "tgp-plugin.h"

G_BEGIN_DECLS

const gchar* tgp_emblem_get_icon_name(TgpStatusFlags flags);
gchar*       tgp_emblem_get_status_text(TgpStatusFlags flags);

G_END_DECLS

#endif /* __TGP_EMBLEM_PROVIDER_H__ */
