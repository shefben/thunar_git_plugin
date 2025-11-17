/*
 * Thunar Git Plugin - Emblem Provider Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-emblem-provider.h"
#include <string.h>

/* GVFS custom attribute namespace for Git status */
#define GIT_STATUS_ATTRIBUTE "metadata::git-status"
#define GIT_EMBLEM_ATTRIBUTE "metadata::git-emblem"

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

/*
 * Set Git status as GVFS metadata attribute on a file via GFile
 * This allows Thunar and other file managers to display emblems
 */
void
tgp_emblem_set_git_status_attribute(GFile *file, TgpStatusFlags flags, GError **error)
{
    GFileInfo *info;
    const gchar *emblem_name;
    gchar *status_text;

    if (!file || !G_IS_FILE(file))
        return;

    emblem_name = tgp_emblem_get_icon_name(flags);
    if (!emblem_name)
        return;

    status_text = tgp_emblem_get_status_text(flags);

    /* Set custom attributes via GIO */
    info = g_file_info_new();

    /* Set the emblem name - Thunar can read this and display the corresponding emblem */
    g_file_info_set_attribute_string(info, GIT_EMBLEM_ATTRIBUTE, emblem_name);

    /* Set the status text for tooltips/info */
    g_file_info_set_attribute_string(info, GIT_STATUS_ATTRIBUTE, status_text);

    /* Try to set attributes on the file - this works with GVFS backends that support metadata */
    g_file_set_attributes_from_info(file, info,
                                     G_FILE_QUERY_INFO_NONE,
                                     NULL, error);

    g_object_unref(info);
    g_free(status_text);
}

/*
 * Convenience function to set Git status from a file path
 */
void
tgp_emblem_set_git_status_on_file(const gchar *file_path, TgpStatusFlags flags)
{
    GFile *file;
    GError *error = NULL;

    if (!file_path)
        return;

    file = g_file_new_for_path(file_path);
    tgp_emblem_set_git_status_attribute(file, flags, &error);

    if (error)
    {
        g_warning("Failed to set Git status attribute on %s: %s",
                  file_path, error->message);
        g_error_free(error);
    }

    g_object_unref(file);
}

/*
 * Retrieve Git status from GVFS metadata attributes
 */
TgpStatusFlags
tgp_emblem_get_git_status_attribute(GFile *file)
{
    GFileInfo *info;
    const gchar *emblem_name;
    TgpStatusFlags flags = 0;
    GError *error = NULL;

    if (!file || !G_IS_FILE(file))
        return flags;

    /* Query the custom attributes we set */
    info = g_file_query_info(file,
                             GIT_EMBLEM_ATTRIBUTE,
                             G_FILE_QUERY_INFO_NONE,
                             NULL,
                             &error);

    if (error)
    {
        g_error_free(error);
        return flags;
    }

    if (!info)
        return flags;

    emblem_name = g_file_info_get_attribute_string(info, GIT_EMBLEM_ATTRIBUTE);

    if (emblem_name)
    {
        /* Map emblem names back to status flags */
        if (g_strcmp0(emblem_name, "emblem-git-conflict") == 0)
            flags = TGP_STATUS_CONFLICTED;
        else if (g_strcmp0(emblem_name, "emblem-git-modified") == 0)
            flags = TGP_STATUS_MODIFIED;
        else if (g_strcmp0(emblem_name, "emblem-git-added") == 0)
            flags = TGP_STATUS_ADDED;
        else if (g_strcmp0(emblem_name, "emblem-git-deleted") == 0)
            flags = TGP_STATUS_DELETED;
        else if (g_strcmp0(emblem_name, "emblem-git-untracked") == 0)
            flags = TGP_STATUS_UNTRACKED;
        else if (g_strcmp0(emblem_name, "emblem-git-ignored") == 0)
            flags = TGP_STATUS_IGNORED;
        else if (g_strcmp0(emblem_name, "emblem-git-ahead") == 0)
            flags = TGP_STATUS_AHEAD;
        else if (g_strcmp0(emblem_name, "emblem-git-behind") == 0)
            flags = TGP_STATUS_BEHIND;
        else if (g_strcmp0(emblem_name, "emblem-git-clean") == 0)
            flags = TGP_STATUS_CLEAN;
    }

    g_object_unref(info);
    return flags;
}
