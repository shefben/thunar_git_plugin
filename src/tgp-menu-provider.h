/*
 * Thunar Git Plugin - Menu Provider
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_MENU_PROVIDER_H__
#define __TGP_MENU_PROVIDER_H__

#include <thunarx/thunarx.h>
#include "tgp-plugin.h"

G_BEGIN_DECLS

GList* tgp_menu_provider_get_file_items(ThunarxMenuProvider *provider,
                                         GtkWidget           *window,
                                         GList               *files);

GList* tgp_menu_provider_get_folder_items(ThunarxMenuProvider *provider,
                                           GtkWidget           *window,
                                           ThunarxFileInfo     *folder);

G_END_DECLS

#endif /* __TGP_MENU_PROVIDER_H__ */
