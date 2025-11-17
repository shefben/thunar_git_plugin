/*
 * Thunar Git Plugin - Native Git Integration for Thunar File Manager
 * Copyright (C) 2025 MiniMax Agent
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __TGP_PLUGIN_H__
#define __TGP_PLUGIN_H__

#include <thunarx/thunarx.h>
#include <gtk/gtk.h>
#include <git2.h>

G_BEGIN_DECLS

#define TGP_TYPE_PLUGIN            (tgp_plugin_get_type())
#define TGP_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TGP_TYPE_PLUGIN, TgpPlugin))
#define TGP_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TGP_TYPE_PLUGIN, TgpPluginClass))
#define TGP_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TGP_TYPE_PLUGIN))
#define TGP_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TGP_TYPE_PLUGIN))
#define TGP_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TGP_TYPE_PLUGIN, TgpPluginClass))

typedef struct _TgpPlugin      TgpPlugin;
typedef struct _TgpPluginClass TgpPluginClass;

struct _TgpPlugin
{
    GObject __parent__;
    
    /* Plugin state */
    GHashTable *repo_cache;     /* Cache of git repositories */
    GHashTable *status_cache;   /* Cache of file statuses */
    guint       cache_timeout;  /* Timeout for cache invalidation */
};

struct _TgpPluginClass
{
    GObjectClass __parent__;
};

GType tgp_plugin_get_type(void) G_GNUC_CONST;
void  tgp_plugin_register_type(ThunarxProviderPlugin *plugin);
void  tgp_plugin_update_emblems_in_directory(const gchar *repo_path);

/* Git status flags */
typedef enum {
    TGP_STATUS_UNTRACKED   = 1 << 0,
    TGP_STATUS_MODIFIED    = 1 << 1,
    TGP_STATUS_ADDED       = 1 << 2,
    TGP_STATUS_DELETED     = 1 << 3,
    TGP_STATUS_RENAMED     = 1 << 4,
    TGP_STATUS_COPIED      = 1 << 5,
    TGP_STATUS_IGNORED     = 1 << 6,
    TGP_STATUS_CONFLICTED  = 1 << 7,
    TGP_STATUS_CLEAN       = 1 << 8,
    TGP_STATUS_AHEAD       = 1 << 9,
    TGP_STATUS_BEHIND      = 1 << 10,
} TgpStatusFlags;

G_END_DECLS

#endif /* __TGP_PLUGIN_H__ */
