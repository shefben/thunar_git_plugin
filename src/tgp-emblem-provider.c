/*
 * Thunar Git Plugin - Emblem Provider Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-emblem-provider.h"

const gchar*
tgp_emblem_get_icon_name(TgpStatusFlags flags)
{
    if (flags & TGP_STATUS_CONFLICTED)
        return "emblem-git-conflict";
    
    if (flags & TGP_STATUS_MODIFIED)
        return "emblem-git-modified";
    
    if (flags & TGP_STATUS_ADDED)
        return "emblem-git-added";
    
    if (flags & TGP_STATUS_DELETED)
        return "emblem-git-deleted";
    
    if (flags & TGP_STATUS_UNTRACKED)
        return "emblem-git-untracked";
    
    if (flags & TGP_STATUS_IGNORED)
        return "emblem-git-ignored";
    
    if (flags & TGP_STATUS_AHEAD)
        return "emblem-git-ahead";
    
    if (flags & TGP_STATUS_BEHIND)
        return "emblem-git-behind";
    
    if (flags & TGP_STATUS_CLEAN)
        return "emblem-git-clean";
    
    return NULL;
}

gchar*
tgp_emblem_get_status_text(TgpStatusFlags flags)
{
    GString *text = g_string_new("");
    
    if (flags & TGP_STATUS_CONFLICTED)
        g_string_append(text, "Conflicted ");
    
    if (flags & TGP_STATUS_MODIFIED)
        g_string_append(text, "Modified ");
    
    if (flags & TGP_STATUS_ADDED)
        g_string_append(text, "Added ");
    
    if (flags & TGP_STATUS_DELETED)
        g_string_append(text, "Deleted ");
    
    if (flags & TGP_STATUS_UNTRACKED)
        g_string_append(text, "Untracked ");
    
    if (flags & TGP_STATUS_IGNORED)
        g_string_append(text, "Ignored ");
    
    if (flags & TGP_STATUS_AHEAD)
        g_string_append(text, "Ahead ");
    
    if (flags & TGP_STATUS_BEHIND)
        g_string_append(text, "Behind ");
    
    if (flags & TGP_STATUS_CLEAN)
        g_string_append(text, "Clean");
    
    if (text->len == 0)
        g_string_append(text, "Unknown");
    
    return g_string_free(text, FALSE);
}
