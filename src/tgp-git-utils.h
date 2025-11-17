/*
 * Thunar Git Plugin - Git Utilities
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_GIT_UTILS_H__
#define __TGP_GIT_UTILS_H__

#include <glib.h>
#include <git2.h>
#include "tgp-plugin.h"

G_BEGIN_DECLS

/* Repository operations */
git_repository* tgp_git_open_repository(const gchar *path);
gboolean        tgp_git_is_repository(const gchar *path);
gchar*          tgp_git_find_repository_root(const gchar *path);

/* Status operations */
TgpStatusFlags  tgp_git_get_file_status(git_repository *repo, const gchar *path);
gboolean        tgp_git_has_uncommitted_changes(git_repository *repo);
gboolean        tgp_git_is_ahead_behind(git_repository *repo, gint *ahead, gint *behind);

/* Branch operations */
gchar*          tgp_git_get_current_branch(git_repository *repo);
GList*          tgp_git_get_branches(git_repository *repo);
gboolean        tgp_git_checkout_branch(git_repository *repo, const gchar *branch_name, GError **error);
gboolean        tgp_git_create_branch(git_repository *repo, const gchar *branch_name, GError **error);

/* Commit operations */
gboolean        tgp_git_commit(git_repository *repo, const gchar *message, GList *files, GError **error);
gboolean        tgp_git_add_files(git_repository *repo, GList *files, GError **error);
gboolean        tgp_git_remove_files(git_repository *repo, GList *files, GError **error);

/* Remote operations */
gboolean        tgp_git_push(git_repository *repo, const gchar *remote, const gchar *branch, GError **error);
gboolean        tgp_git_pull(git_repository *repo, const gchar *remote, const gchar *branch, GError **error);
gboolean        tgp_git_fetch(git_repository *repo, const gchar *remote, GError **error);
gboolean        tgp_git_clone(const gchar *url, const gchar *path, GError **error);
GList*          tgp_git_get_remotes(git_repository *repo);

/* Remote operations with authentication support */
gboolean        tgp_git_push_with_auth(git_repository *repo, const gchar *remote, const gchar *branch,
                                       const gchar *username, const gchar *password, GError **error);
gboolean        tgp_git_pull_with_auth(git_repository *repo, const gchar *remote, const gchar *branch,
                                       const gchar *username, const gchar *password, GError **error);

/* History operations */
GList*          tgp_git_get_log(git_repository *repo, gint limit);
gchar*          tgp_git_get_diff(git_repository *repo, const gchar *path);

/* Conflict resolution */
gboolean        tgp_git_has_conflicts(git_repository *repo);
GList*          tgp_git_get_conflicted_files(git_repository *repo);
gboolean        tgp_git_resolve_conflict(git_repository *repo, const gchar *path, GError **error);

/* Stash operations */
gboolean        tgp_git_stash(git_repository *repo, const gchar *message, GError **error);
gboolean        tgp_git_stash_pop(git_repository *repo, GError **error);
GList*          tgp_git_get_stashes(git_repository *repo);

/* Initialize/cleanup */
void            tgp_git_init(void);
void            tgp_git_shutdown(void);

G_END_DECLS

#endif /* __TGP_GIT_UTILS_H__ */
