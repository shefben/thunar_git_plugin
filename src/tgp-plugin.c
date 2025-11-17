/*
 * Thunar Git Plugin - Main Plugin Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-plugin.h"
#include "tgp-menu-provider.h"
#include "tgp-emblem-provider.h"
#include "tgp-git-utils.h"
#include <string.h>

/* Global type variable for manual registration */
static GType tgp_plugin_type = G_TYPE_INVALID;

static void tgp_plugin_menu_provider_init(ThunarxMenuProviderIface *iface);
static void tgp_plugin_emblem_provider_init(ThunarxFileInfoIface *iface);
static void tgp_plugin_finalize(GObject *object);
static void tgp_plugin_class_init(TgpPluginClass *klass);
static void tgp_plugin_class_finalize(TgpPluginClass *klass);
static void tgp_plugin_init(TgpPlugin *plugin);

/* Implement get_type function manually */
GType
tgp_plugin_get_type(void)
{
    return tgp_plugin_type;
}

static void
tgp_plugin_class_init(TgpPluginClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = tgp_plugin_finalize;
}

static void
tgp_plugin_class_finalize(TgpPluginClass *klass)
{
}

static void
tgp_plugin_init(TgpPlugin *plugin)
{
    plugin->repo_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    plugin->status_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    plugin->cache_timeout = 0;
}

static void
tgp_plugin_finalize(GObject *object)
{
    TgpPlugin *plugin = TGP_PLUGIN(object);
    
    if (plugin->repo_cache)
    {
        g_hash_table_destroy(plugin->repo_cache);
        plugin->repo_cache = NULL;
    }
    
    if (plugin->status_cache)
    {
        g_hash_table_destroy(plugin->status_cache);
        plugin->status_cache = NULL;
    }
    
    if (plugin->cache_timeout)
    {
        g_source_remove(plugin->cache_timeout);
        plugin->cache_timeout = 0;
    }
    
    G_OBJECT_CLASS(g_type_class_peek_parent(G_OBJECT_GET_CLASS(object)))->finalize(object);
}

static void
tgp_plugin_menu_provider_init(ThunarxMenuProviderIface *iface)
{
    iface->get_file_menu_items = tgp_menu_provider_get_file_items;
    iface->get_folder_menu_items = tgp_menu_provider_get_folder_items;
}

static void
tgp_plugin_emblem_provider_init(ThunarxFileInfoIface *iface)
{
    /* Note: Emblem provider interface is limited in Thunarx
     * We'll use property pages and custom overlays as workaround */
}

void
tgp_plugin_register_type(ThunarxProviderPlugin *plugin)
{
    static const GTypeInfo type_info = {
        sizeof(TgpPluginClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) tgp_plugin_class_init,
        (GClassFinalizeFunc) tgp_plugin_class_finalize,
        NULL,           /* class_data */
        sizeof(TgpPlugin),
        0,              /* n_preallocs */
        (GInstanceInitFunc) tgp_plugin_init,
        NULL            /* value_table */
    };

    static const GInterfaceInfo menu_provider_info = {
        (GInterfaceInitFunc) tgp_plugin_menu_provider_init,
        NULL,           /* interface_finalize */
        NULL            /* interface_data */
    };

    static const GInterfaceInfo emblem_provider_info = {
        (GInterfaceInitFunc) tgp_plugin_emblem_provider_init,
        NULL,           /* interface_finalize */
        NULL            /* interface_data */
    };

    tgp_plugin_type = g_type_module_register_type(G_TYPE_MODULE(plugin),
                                                   G_TYPE_OBJECT,
                                                   "TgpPlugin",
                                                   &type_info,
                                                   0);

    g_type_module_add_interface(G_TYPE_MODULE(plugin),
                                tgp_plugin_type,
                                THUNARX_TYPE_MENU_PROVIDER,
                                &menu_provider_info);

    g_type_module_add_interface(G_TYPE_MODULE(plugin),
                                tgp_plugin_type,
                                THUNARX_TYPE_FILE_INFO,
                                &emblem_provider_info);
}

G_MODULE_EXPORT void
thunar_extension_initialize(ThunarxProviderPlugin *plugin)
{
    const gchar *mismatch;
    
    /* Verify that the thunarx versions are compatible */
    mismatch = thunarx_check_version(THUNARX_MAJOR_VERSION,
                                      THUNARX_MINOR_VERSION,
                                      THUNARX_MICRO_VERSION);
    if (G_UNLIKELY(mismatch != NULL))
    {
        g_warning("Version mismatch: %s", mismatch);
        return;
    }
    
    /* Initialize libgit2 */
    tgp_git_init();
    
    /* Register the plugin types */
    tgp_plugin_register_type(plugin);
    
    g_message("Thunar Git Plugin initialized successfully");
}

G_MODULE_EXPORT void
thunar_extension_shutdown(void)
{
    tgp_git_shutdown();
    g_message("Thunar Git Plugin shut down");
}

G_MODULE_EXPORT void
thunar_extension_list_types(const GType **types,
                            gint         *n_types)
{
    static GType type_list[1];
    
    type_list[0] = TGP_TYPE_PLUGIN;
    
    *types = type_list;
    *n_types = G_N_ELEMENTS(type_list);
}
