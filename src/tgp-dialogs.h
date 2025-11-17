/*
 * Thunar Git Plugin - GTK Dialogs
 * Copyright (C) 2025 MiniMax Agent
 */

#ifndef __TGP_DIALOGS_H__
#define __TGP_DIALOGS_H__

#include <gtk/gtk.h>
#include <thunarx/thunarx.h>

G_BEGIN_DECLS

/* Information dialogs */
void tgp_show_info_dialog(GtkWindow *parent, const gchar *title, const gchar *message);
void tgp_show_error_dialog(GtkWindow *parent, const gchar *title, const gchar *message);

/* Authentication dialog */
gboolean tgp_show_login_dialog(GtkWindow *parent, const gchar *host,
                               gchar **username, gchar **password,
                               gboolean *save_credentials);

/* Main operation dialogs */
void tgp_show_commit_dialog(GtkWindow *parent, const gchar *repo_path, GList *files);
void tgp_show_clone_dialog(GtkWindow *parent, const gchar *target_path);
void tgp_show_push_dialog(GtkWindow *parent, const gchar *repo_path);
void tgp_show_pull_dialog(GtkWindow *parent, const gchar *repo_path);

/* Advanced dialogs */
void tgp_show_log_dialog(GtkWindow *parent, const gchar *repo_path);
void tgp_show_diff_dialog(GtkWindow *parent, const gchar *repo_path, const gchar *file_path);
void tgp_show_branch_dialog(GtkWindow *parent, const gchar *repo_path);
void tgp_show_stash_dialog(GtkWindow *parent, const gchar *repo_path);
void tgp_show_conflict_dialog(GtkWindow *parent, const gchar *repo_path);
void tgp_show_status_dialog(GtkWindow *parent, const gchar *repo_path);

G_END_DECLS

#endif /* __TGP_DIALOGS_H__ */
