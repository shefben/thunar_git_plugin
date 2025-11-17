/*
 * Thunar Git Plugin - GTK Dialogs Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-dialogs.h"
#include "tgp-git-utils.h"
#include "tgp-credentials.h"
#include <string.h>

void
tgp_show_info_dialog(GtkWindow *parent, const gchar *title, const gchar *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent,
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_INFO,
                                                GTK_BUTTONS_OK,
                                                "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void
tgp_show_error_dialog(GtkWindow *parent, const gchar *title, const gchar *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent,
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_OK,
                                                "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Login Dialog */
gboolean
tgp_show_login_dialog(GtkWindow *parent, const gchar *host,
                      gchar **username, gchar **password,
                      gboolean *save_credentials)
{
    GtkWidget *dialog, *content_area, *grid;
    GtkWidget *label, *username_entry, *password_entry, *save_check;
    GtkWidget *auth_label;
    gint response;
    gchar *title, *message;

    if (!host || !username || !password || !save_credentials)
        return FALSE;

    title = g_strdup_printf("Authentication Required");
    message = g_strdup_printf("Please provide authentication credentials for: %s", host);

    dialog = gtk_dialog_new_with_buttons(title,
                                         parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "Cancel", GTK_RESPONSE_CANCEL,
                                         "Authenticate", GTK_RESPONSE_OK,
                                         NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 250);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    /* Create main grid layout */
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_box_pack_start(GTK_BOX(content_area), grid, TRUE, TRUE, 0);

    /* Authentication info label */
    auth_label = gtk_label_new(message);
    gtk_label_set_line_wrap(GTK_LABEL(auth_label), TRUE);
    gtk_grid_attach(GTK_GRID(grid), auth_label, 0, 0, 2, 1);

    /* Username label and entry */
    label = gtk_label_new("Username:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 1, 1, 1);
    gtk_widget_set_hexpand(username_entry, TRUE);

    /* Pre-fill with stored username if available */
    if (*username)
    {
        gtk_entry_set_text(GTK_ENTRY(username_entry), *username);
        gtk_editable_set_position(GTK_EDITABLE(username_entry), -1);
    }

    /* Password label and entry */
    label = gtk_label_new("Password:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);

    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 2, 1, 1);
    gtk_widget_set_hexpand(password_entry, TRUE);

    /* Pre-fill with stored password if available */
    if (*password)
    {
        gtk_entry_set_text(GTK_ENTRY(password_entry), *password);
    }

    /* Save credentials checkbox */
    save_check = gtk_check_button_new_with_label("Save credentials for this session");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_check), *save_credentials);
    gtk_grid_attach(GTK_GRID(grid), save_check, 0, 3, 2, 1);

    gtk_widget_show_all(dialog);

    /* Run dialog */
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
        const gchar *user_text = gtk_entry_get_text(GTK_ENTRY(username_entry));
        const gchar *pass_text = gtk_entry_get_text(GTK_ENTRY(password_entry));

        /* Update output parameters */
        if (user_text && *user_text)
        {
            if (*username)
                g_free(*username);
            *username = g_strdup(user_text);
        }

        if (pass_text && *pass_text)
        {
            if (*password)
            {
                memset(*password, 0, strlen(*password));
                g_free(*password);
            }
            *password = g_strdup(pass_text);
        }

        *save_credentials = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(save_check));

        /* If save was requested, store the credentials */
        if (*save_credentials && *username && *password)
        {
            /* Store for this session (3600 seconds = 1 hour) */
            tgp_credentials_store(host, *username, *password, 3600);
        }
    }

    gtk_widget_destroy(dialog);
    g_free(title);
    g_free(message);

    return response == GTK_RESPONSE_OK;
}

/* Commit Dialog */
void
tgp_show_commit_dialog(GtkWindow *parent, const gchar *repo_path, GList *files)
{
    GtkWidget *dialog, *content_area, *grid, *label, *message_view, *scroll;
    GtkWidget *file_list_view, *file_scroll;
    GtkTextBuffer *buffer;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Git Commit",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Commit", GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    /* Commit message */
    label = gtk_label_new("Commit message:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    
    message_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(message_view), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(message_view));
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 150);
    gtk_container_add(GTK_CONTAINER(scroll), message_view);
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 1, 1, 1);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    
    /* Files to commit */
    label = gtk_label_new("Files to commit:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    
    store = gtk_list_store_new(2, G_TYPE_BOOLEAN, G_TYPE_STRING);
    
    /* Add files to list */
    git_repository *repo = tgp_git_open_repository(repo_path);
    if (repo)
    {
        const gchar *workdir = git_repository_workdir(repo);
        for (GList *l = files; l != NULL; l = l->next)
        {
            gchar *file_path = thunarx_file_info_get_location(l->data);
            const gchar *relative_path = file_path;
            
            if (workdir && g_str_has_prefix(file_path, workdir))
            {
                relative_path = file_path + strlen(workdir);
            }
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, TRUE, 1, relative_path, -1);
            
            g_free(file_path);
        }
        git_repository_free(repo);
    }
    
    file_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Include", renderer, "active", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_list_view), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_list_view), column);
    
    file_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(file_scroll), 200);
    gtk_container_add(GTK_CONTAINER(file_scroll), file_list_view);
    gtk_grid_attach(GTK_GRID(grid), file_scroll, 0, 3, 1, 1);
    gtk_widget_set_hexpand(file_scroll, TRUE);
    gtk_widget_set_vexpand(file_scroll, TRUE);
    
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT)
    {
        GtkTextIter start, end;
        gchar *commit_message;
        
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        commit_message = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        
        if (commit_message && strlen(commit_message) > 0)
        {
            git_repository *repo = tgp_git_open_repository(repo_path);
            if (repo)
            {
                GError *error = NULL;
                if (tgp_git_commit(repo, commit_message, files, &error))
                {
                    tgp_show_info_dialog(parent, "Commit Successful", 
                                        "Changes have been committed successfully.");
                }
                else
                {
                    tgp_show_error_dialog(parent, "Commit Failed", 
                                         error ? error->message : "Unknown error");
                    if (error) g_error_free(error);
                }
                git_repository_free(repo);
            }
        }
        else
        {
            tgp_show_error_dialog(parent, "Invalid Input", 
                                 "Commit message cannot be empty.");
        }
        
        g_free(commit_message);
    }
    
    gtk_widget_destroy(dialog);
}

/* Clone Dialog */
void
tgp_show_clone_dialog(GtkWindow *parent, const gchar *target_path)
{
    GtkWidget *dialog, *content_area, *grid, *label;
    GtkWidget *url_entry, *path_entry, *path_button;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Clone Repository",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Clone", GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 200);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    /* Repository URL */
    label = gtk_label_new("Repository URL:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    
    url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(url_entry), "https://github.com/user/repo.git");
    gtk_grid_attach(GTK_GRID(grid), url_entry, 1, 0, 2, 1);
    gtk_widget_set_hexpand(url_entry, TRUE);
    
    /* Target path */
    label = gtk_label_new("Target directory:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    
    path_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(path_entry), target_path);
    gtk_grid_attach(GTK_GRID(grid), path_entry, 1, 1, 1, 1);
    gtk_widget_set_hexpand(path_entry, TRUE);
    
    path_button = gtk_button_new_with_label("Browse...");
    gtk_grid_attach(GTK_GRID(grid), path_button, 2, 1, 1, 1);
    
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT)
    {
        const gchar *url = gtk_entry_get_text(GTK_ENTRY(url_entry));
        const gchar *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
        
        if (url && strlen(url) > 0 && path && strlen(path) > 0)
        {
            GError *error = NULL;
            if (tgp_git_clone(url, path, &error))
            {
                tgp_show_info_dialog(parent, "Clone Successful", 
                                    "Repository has been cloned successfully.");
            }
            else
            {
                tgp_show_error_dialog(parent, "Clone Failed", 
                                     error ? error->message : "Unknown error");
                if (error) g_error_free(error);
            }
        }
        else
        {
            tgp_show_error_dialog(parent, "Invalid Input", 
                                 "URL and path cannot be empty.");
        }
    }
    
    gtk_widget_destroy(dialog);
}

/* Push Dialog */
void
tgp_show_push_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *grid, *label;
    GtkWidget *remote_combo, *branch_entry;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Push to Remote",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Push", GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 150);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    /* Remote */
    label = gtk_label_new("Remote:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    
    remote_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(remote_combo), "origin");
    gtk_combo_box_set_active(GTK_COMBO_BOX(remote_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), remote_combo, 1, 0, 1, 1);
    gtk_widget_set_hexpand(remote_combo, TRUE);
    
    /* Branch */
    label = gtk_label_new("Branch:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    
    branch_entry = gtk_entry_new();
    
    git_repository *repo = tgp_git_open_repository(repo_path);
    if (repo)
    {
        gchar *branch = tgp_git_get_current_branch(repo);
        if (branch)
        {
            gtk_entry_set_text(GTK_ENTRY(branch_entry), branch);
            g_free(branch);
        }
        git_repository_free(repo);
    }
    
    gtk_grid_attach(GTK_GRID(grid), branch_entry, 1, 1, 1, 1);
    gtk_widget_set_hexpand(branch_entry, TRUE);
    
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT)
    {
        const gchar *remote = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remote_combo));
        const gchar *branch = gtk_entry_get_text(GTK_ENTRY(branch_entry));
        
        git_repository *repo = tgp_git_open_repository(repo_path);
        if (repo)
        {
            GError *error = NULL;
            if (tgp_git_push(repo, remote, branch, &error))
            {
                tgp_show_info_dialog(parent, "Push Successful", 
                                    "Changes have been pushed to remote.");
            }
            else
            {
                tgp_show_error_dialog(parent, "Push Failed", 
                                     error ? error->message : "Unknown error");
                if (error) g_error_free(error);
            }
            git_repository_free(repo);
        }
    }
    
    gtk_widget_destroy(dialog);
}

/* Pull Dialog */
void
tgp_show_pull_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *grid, *label;
    GtkWidget *remote_combo, *branch_entry;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Pull from Remote",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Pull", GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 150);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    /* Remote */
    label = gtk_label_new("Remote:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    
    remote_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(remote_combo), "origin");
    gtk_combo_box_set_active(GTK_COMBO_BOX(remote_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), remote_combo, 1, 0, 1, 1);
    gtk_widget_set_hexpand(remote_combo, TRUE);
    
    /* Branch */
    label = gtk_label_new("Branch:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    
    branch_entry = gtk_entry_new();
    
    git_repository *repo = tgp_git_open_repository(repo_path);
    if (repo)
    {
        gchar *branch = tgp_git_get_current_branch(repo);
        if (branch)
        {
            gtk_entry_set_text(GTK_ENTRY(branch_entry), branch);
            g_free(branch);
        }
        git_repository_free(repo);
    }
    
    gtk_grid_attach(GTK_GRID(grid), branch_entry, 1, 1, 1, 1);
    gtk_widget_set_hexpand(branch_entry, TRUE);
    
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT)
    {
        const gchar *remote = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remote_combo));
        const gchar *branch = gtk_entry_get_text(GTK_ENTRY(branch_entry));
        
        git_repository *repo = tgp_git_open_repository(repo_path);
        if (repo)
        {
            GError *error = NULL;
            if (tgp_git_pull(repo, remote, branch, &error))
            {
                tgp_show_info_dialog(parent, "Pull Successful", 
                                    "Changes have been pulled from remote.");
            }
            else
            {
                tgp_show_error_dialog(parent, "Pull Failed", 
                                     error ? error->message : "Unknown error");
                if (error) g_error_free(error);
            }
            git_repository_free(repo);
        }
    }
    
    gtk_widget_destroy(dialog);
}

/* Continued in next part due to size... */

/* Log Dialog */
void
tgp_show_log_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *scroll, *text_view;
    GtkTextBuffer *buffer;
    git_repository *repo;
    git_revwalk *walker;
    git_oid oid;
    git_commit *commit;
    GString *log_text;
    
    dialog = gtk_dialog_new_with_buttons("Git Log",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Close", GTK_RESPONSE_CLOSE,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    
    /* Load git log */
    log_text = g_string_new("");
    repo = tgp_git_open_repository(repo_path);
    
    if (repo)
    {
        if (git_revwalk_new(&walker, repo) == 0)
        {
            git_revwalk_sorting(walker, GIT_SORT_TIME);
            git_revwalk_push_head(walker);
            
            int count = 0;
            while (git_revwalk_next(&oid, walker) == 0 && count < 100)
            {
                if (git_commit_lookup(&commit, repo, &oid) == 0)
                {
                    char oid_str[GIT_OID_HEXSZ + 1];
                    git_oid_tostr(oid_str, sizeof(oid_str), &oid);
                    
                    const git_signature *author = git_commit_author(commit);
                    const gchar *message = git_commit_message(commit);
                    git_time_t time = git_commit_time(commit);
                    
                    g_string_append_printf(log_text, 
                        "commit %s\n"
                        "Author: %s <%s>\n"
                        "Date:   %s\n"
                        "\n    %s\n\n",
                        oid_str,
                        author->name,
                        author->email,
                        g_date_time_format(g_date_time_new_from_unix_local(time), "%c"),
                        message ? message : "(no message)");
                    
                    git_commit_free(commit);
                    count++;
                }
            }
            
            git_revwalk_free(walker);
        }
        git_repository_free(repo);
    }
    
    if (log_text->len == 0)
    {
        g_string_append(log_text, "No commits found or unable to read log.");
    }
    
    gtk_text_buffer_set_text(buffer, log_text->str, -1);
    g_string_free(log_text, TRUE);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Diff Dialog */
void
tgp_show_diff_dialog(GtkWindow *parent, const gchar *repo_path, const gchar *file_path)
{
    GtkWidget *dialog, *content_area, *scroll, *text_view;
    GtkTextBuffer *buffer;
    gchar *diff_text;
    
    dialog = gtk_dialog_new_with_buttons("Git Diff",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Close", GTK_RESPONSE_CLOSE,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    
    /* Load diff */
    git_repository *repo = tgp_git_open_repository(repo_path);
    if (repo)
    {
        diff_text = tgp_git_get_diff(repo, file_path);
        if (diff_text)
        {
            gtk_text_buffer_set_text(buffer, diff_text, -1);
            g_free(diff_text);
        }
        else
        {
            gtk_text_buffer_set_text(buffer, "No changes or unable to generate diff.", -1);
        }
        git_repository_free(repo);
    }
    else
    {
        gtk_text_buffer_set_text(buffer, "Unable to open repository.", -1);
    }
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Branch Dialog */
void
tgp_show_branch_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *grid, *scroll, *tree_view;
    GtkWidget *button_box, *create_button, *delete_button, *checkout_button;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    git_repository *repo;
    git_branch_iterator *branch_iter;
    git_reference *branch_ref;
    git_branch_t branch_type;
    gchar *current_branch;
    
    dialog = gtk_dialog_new_with_buttons("Branch Manager",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Close", GTK_RESPONSE_CLOSE,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    /* Branch list */
    store = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
    
    repo = tgp_git_open_repository(repo_path);
    current_branch = NULL;
    
    if (repo)
    {
        current_branch = tgp_git_get_current_branch(repo);
        
        if (git_branch_iterator_new(&branch_iter, repo, GIT_BRANCH_LOCAL) == 0)
        {
            while (git_branch_next(&branch_ref, &branch_type, branch_iter) == 0)
            {
                const gchar *branch_name;
                if (git_branch_name(&branch_name, branch_ref) == 0)
                {
                    gboolean is_current = (current_branch && strcmp(branch_name, current_branch) == 0);
                    
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                                      0, is_current,
                                      1, branch_name,
                                      2, is_current ? "Current" : "Local",
                                      -1);
                }
                git_reference_free(branch_ref);
            }
            git_branch_iterator_free(branch_iter);
        }
        git_repository_free(repo);
    }
    
    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Current", renderer, "active", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Branch Name", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), tree_view);
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    
    /* Button box */
    button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_START);
    gtk_box_set_spacing(GTK_BOX(button_box), 5);
    
    create_button = gtk_button_new_with_label("Create Branch");
    gtk_container_add(GTK_CONTAINER(button_box), create_button);
    
    checkout_button = gtk_button_new_with_label("Checkout");
    gtk_container_add(GTK_CONTAINER(button_box), checkout_button);
    
    delete_button = gtk_button_new_with_label("Delete");
    gtk_container_add(GTK_CONTAINER(button_box), delete_button);
    
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 1, 1, 1);
    
    g_free(current_branch);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Stash Dialog */
void
tgp_show_stash_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *grid, *label, *entry;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Stash Changes",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Stash", GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 150);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    label = gtk_label_new("Stash message:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Optional stash message");
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_widget_set_hexpand(entry, TRUE);
    
    gtk_widget_show_all(dialog);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT)
    {
        const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry));
        git_repository *repo = tgp_git_open_repository(repo_path);
        
        if (repo)
        {
            GError *error = NULL;
            if (tgp_git_stash(repo, message && strlen(message) > 0 ? message : NULL, &error))
            {
                tgp_show_info_dialog(parent, "Stash Successful", 
                                    "Changes have been stashed.");
            }
            else
            {
                tgp_show_error_dialog(parent, "Stash Failed", 
                                     error ? error->message : "Unknown error");
                if (error) g_error_free(error);
            }
            git_repository_free(repo);
        }
    }
    
    gtk_widget_destroy(dialog);
}

/* Conflict Resolution Dialog */
void
tgp_show_conflict_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *scroll, *tree_view;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    git_repository *repo;
    git_index *index;
    
    dialog = gtk_dialog_new_with_buttons("Resolve Conflicts",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Close", GTK_RESPONSE_CLOSE,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    
    repo = tgp_git_open_repository(repo_path);
    if (repo)
    {
        if (git_repository_index(&index, repo) == 0)
        {
            git_index_conflict_iterator *conflict_iter;
            const git_index_entry *ancestor, *our, *their;
            
            if (git_index_conflict_iterator_new(&conflict_iter, index) == 0)
            {
                while (git_index_conflict_next(&ancestor, &our, &their, conflict_iter) == 0)
                {
                    const gchar *path = our ? our->path : (their ? their->path : "unknown");
                    
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                                      0, path,
                                      1, "Conflicted",
                                      -1);
                }
                git_index_conflict_iterator_free(conflict_iter);
            }
            git_index_free(index);
        }
        git_repository_free(repo);
    }
    
    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), tree_view);
    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Status Dialog */
void
tgp_show_status_dialog(GtkWindow *parent, const gchar *repo_path)
{
    GtkWidget *dialog, *content_area, *scroll, *text_view;
    GtkTextBuffer *buffer;
    GString *status_text;
    git_repository *repo;
    git_status_list *status_list;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    
    dialog = gtk_dialog_new_with_buttons("Repository Status",
                                          parent,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "_Close", GTK_RESPONSE_CLOSE,
                                          NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    
    status_text = g_string_new("");
    repo = tgp_git_open_repository(repo_path);
    
    if (repo)
    {
        gchar *branch = tgp_git_get_current_branch(repo);
        if (branch)
        {
            g_string_append_printf(status_text, "On branch: %s\n\n", branch);
            g_free(branch);
        }
        
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | 
                     GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
                     GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;
        
        if (git_status_list_new(&status_list, repo, &opts) == 0)
        {
            size_t count = git_status_list_entrycount(status_list);
            
            if (count == 0)
            {
                g_string_append(status_text, "Working directory clean.\n");
            }
            else
            {
                g_string_append(status_text, "Changes:\n\n");
                
                for (size_t i = 0; i < count; i++)
                {
                    const git_status_entry *entry = git_status_byindex(status_list, i);
                    const gchar *path = entry->head_to_index ? 
                                       entry->head_to_index->old_file.path :
                                       entry->index_to_workdir->old_file.path;
                    
                    const gchar *status_str = "Unknown";
                    
                    if (entry->status & GIT_STATUS_INDEX_NEW || entry->status & GIT_STATUS_WT_NEW)
                        status_str = "Untracked";
                    else if (entry->status & GIT_STATUS_INDEX_MODIFIED || entry->status & GIT_STATUS_WT_MODIFIED)
                        status_str = "Modified";
                    else if (entry->status & GIT_STATUS_INDEX_DELETED || entry->status & GIT_STATUS_WT_DELETED)
                        status_str = "Deleted";
                    else if (entry->status & GIT_STATUS_INDEX_RENAMED || entry->status & GIT_STATUS_WT_RENAMED)
                        status_str = "Renamed";
                    else if (entry->status & GIT_STATUS_CONFLICTED)
                        status_str = "Conflicted";
                    
                    g_string_append_printf(status_text, "  %-12s %s\n", status_str, path);
                }
            }
            
            git_status_list_free(status_list);
        }
        
        /* Check ahead/behind */
        gint ahead = 0, behind = 0;
        if (tgp_git_is_ahead_behind(repo, &ahead, &behind))
        {
            if (ahead > 0 || behind > 0)
            {
                g_string_append_printf(status_text, "\nBranch is ");
                if (ahead > 0)
                    g_string_append_printf(status_text, "%d ahead", ahead);
                if (ahead > 0 && behind > 0)
                    g_string_append(status_text, " and ");
                if (behind > 0)
                    g_string_append_printf(status_text, "%d behind", behind);
                g_string_append(status_text, " of remote.\n");
            }
        }
        
        git_repository_free(repo);
    }
    else
    {
        g_string_append(status_text, "Unable to open repository.");
    }
    
    gtk_text_buffer_set_text(buffer, status_text->str, -1);
    g_string_free(status_text, TRUE);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
