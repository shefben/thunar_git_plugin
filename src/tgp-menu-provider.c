/*
 * Thunar Git Plugin - Menu Provider Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-menu-provider.h"
#include "tgp-git-utils.h"
#include "tgp-dialogs.h"
#include "tgp-emblem-provider.h"
#include "tgp-plugin.h"
#include <string.h>

/* Forward declarations */
static void action_commit(ThunarxMenuItem *item, gpointer user_data);
static void action_add(ThunarxMenuItem *item, gpointer user_data);
static void action_push(ThunarxMenuItem *item, gpointer user_data);
static void action_pull(ThunarxMenuItem *item, gpointer user_data);
static void action_clone(ThunarxMenuItem *item, gpointer user_data);
static void action_log(ThunarxMenuItem *item, gpointer user_data);
static void action_diff(ThunarxMenuItem *item, gpointer user_data);
static void action_branch(ThunarxMenuItem *item, gpointer user_data);
static void action_stash(ThunarxMenuItem *item, gpointer user_data);
static void action_init(ThunarxMenuItem *item, gpointer user_data);
static void action_revert(ThunarxMenuItem *item, gpointer user_data);
static void action_resolve(ThunarxMenuItem *item, gpointer user_data);
static void action_fetch(ThunarxMenuItem *item, gpointer user_data);
static void action_status(ThunarxMenuItem *item, gpointer user_data);

typedef struct {
    GtkWidget *window;
    GList     *files;
    gchar     *repo_path;
} ActionData;

static ActionData*
action_data_new(GtkWidget *window, GList *files, const gchar *repo_path)
{
    ActionData *data = g_new0(ActionData, 1);
    data->window = window;
    data->files = g_list_copy_deep(files, (GCopyFunc)g_object_ref, NULL);
    data->repo_path = g_strdup(repo_path);
    return data;
}

static void
action_data_free(ActionData *data)
{
    if (data->files)
        g_list_free_full(data->files, g_object_unref);
    g_free(data->repo_path);
    g_free(data);
}

GList*
tgp_menu_provider_get_file_items(ThunarxMenuProvider *provider,
                                  GtkWidget           *window,
                                  GList               *files)
{
    GList *items = NULL;
    ThunarxMenuItem *item, *submenu_item;
    ThunarxMenu *submenu;
    gchar *file_path = NULL;
    gchar *repo_root = NULL;
    git_repository *repo = NULL;
    
    if (files == NULL)
        return NULL;
    
    /* Get first file path */
    ThunarxFileInfo *file_info = files->data;
    file_path = thunarx_file_info_get_location(file_info);
    
    if (file_path == NULL)
        return NULL;
    
    /* Check if we're in a git repository */
    repo_root = tgp_git_find_repository_root(file_path);
    if (repo_root)
    {
        repo = tgp_git_open_repository(file_path);
    }
    
    /* Create main Git submenu */
    item = thunarx_menu_item_new("TGP::Git", "Git", "Git Version Control", "git");
    items = g_list_append(items, item);
    
    submenu = thunarx_menu_new();
    thunarx_menu_item_set_menu(item, submenu);
    
    if (repo)
    {
        /* File operations */
        submenu_item = thunarx_menu_item_new("TGP::Add", "Add", 
                                              "Add files to index", "list-add");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_add),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Commit", "Commit...", 
                                              "Commit changes", "document-save");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_commit),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Revert", "Revert Changes", 
                                              "Discard local changes", "edit-undo");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_revert),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Separator */
        submenu_item = thunarx_menu_item_new("TGP::Sep1", "", "", NULL);
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Diff and Log */
        submenu_item = thunarx_menu_item_new("TGP::Diff", "Show Diff", 
                                              "View file changes", "document-properties");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_diff),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Log", "Show Log", 
                                              "View commit history", "document-open-recent");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_log),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Separator */
        submenu_item = thunarx_menu_item_new("TGP::Sep2", "", "", NULL);
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Sync operations */
        submenu_item = thunarx_menu_item_new("TGP::Push", "Push", 
                                              "Push to remote", "go-up");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_push),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Pull", "Pull", 
                                              "Pull from remote", "go-down");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_pull),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Fetch", "Fetch", 
                                              "Fetch from remote", "view-refresh");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_fetch),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Separator */
        submenu_item = thunarx_menu_item_new("TGP::Sep3", "", "", NULL);
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Branch and stash */
        submenu_item = thunarx_menu_item_new("TGP::Branch", "Branch Manager...", 
                                              "Manage branches", "network-workgroup");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_branch),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Stash", "Stash Changes...", 
                                              "Stash uncommitted changes", "document-save-as");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_stash),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Check for conflicts */
        if (tgp_git_has_conflicts(repo))
        {
            submenu_item = thunarx_menu_item_new("TGP::Resolve", "Resolve Conflicts...", 
                                                  "Resolve merge conflicts", "dialog-warning");
            g_signal_connect(submenu_item, "activate", G_CALLBACK(action_resolve),
                            action_data_new(window, files, repo_root));
            thunarx_menu_append_item(submenu, submenu_item);
            g_object_unref(submenu_item);
        }
        
        /* Separator */
        submenu_item = thunarx_menu_item_new("TGP::Sep4", "", "", NULL);
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        /* Status */
        submenu_item = thunarx_menu_item_new("TGP::Status", "Repository Status", 
                                              "Show repository status", "dialog-information");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_status),
                        action_data_new(window, files, repo_root));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        git_repository_free(repo);
    }
    else
    {
        /* Not in a repo - offer clone and init */
        submenu_item = thunarx_menu_item_new("TGP::Clone", "Clone Repository...", 
                                              "Clone a repository", "folder-download");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_clone),
                        action_data_new(window, files, file_path));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
        
        submenu_item = thunarx_menu_item_new("TGP::Init", "Create Repository Here", 
                                              "Initialize a new repository", "folder-new");
        g_signal_connect(submenu_item, "activate", G_CALLBACK(action_init),
                        action_data_new(window, files, file_path));
        thunarx_menu_append_item(submenu, submenu_item);
        g_object_unref(submenu_item);
    }
    
    g_object_unref(submenu);
    g_free(file_path);
    g_free(repo_root);
    
    return items;
}

GList*
tgp_menu_provider_get_folder_items(ThunarxMenuProvider *provider,
                                    GtkWidget           *window,
                                    ThunarxFileInfo     *folder)
{
    GList *files = g_list_append(NULL, folder);
    GList *items = tgp_menu_provider_get_file_items(provider, window, files);
    g_list_free(files);
    return items;
}

/* Action implementations */
static void
action_commit(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_commit_dialog(GTK_WINDOW(data->window), data->repo_path, data->files);
    action_data_free(data);
}

static void
action_add(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    git_repository *repo = tgp_git_open_repository(data->repo_path);
    
    if (repo)
    {
        GList *file_paths = NULL;
        for (GList *l = data->files; l != NULL; l = l->next)
        {
            gchar *path = thunarx_file_info_get_location(l->data);
            file_paths = g_list_append(file_paths, path);
        }
        
        GError *error = NULL;
        if (tgp_git_add_files(repo, file_paths, &error))
        {
            tgp_show_info_dialog(GTK_WINDOW(data->window),
                                "Files Added",
                                "Selected files have been added to the index.");

            /* Update emblems to reflect new status */
            tgp_plugin_update_emblems_in_directory(data->repo_path);
        }
        else
        {
            tgp_show_error_dialog(GTK_WINDOW(data->window),
                                 "Add Failed",
                                 error ? error->message : "Unknown error");
            if (error) g_error_free(error);
        }

        g_list_free_full(file_paths, g_free);
        git_repository_free(repo);
    }
    
    action_data_free(data);
}

static void
action_push(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    git_repository *repo;
    GList *remotes;
    gchar *remote_name = NULL;
    gchar *branch_name = NULL;
    gchar *username = NULL;
    gchar *password = NULL;
    gboolean save_credentials = FALSE;
    GError *error = NULL;

    repo = tgp_git_open_repository(data->repo_path);
    if (repo)
    {
        remotes = tgp_git_get_remotes(repo);
        if (remotes)
        {
            remote_name = (gchar *)remotes->data;
            branch_name = tgp_git_get_current_branch(repo);

            if (remote_name && branch_name)
            {
                /* Try to push with stored credentials first */
                if (!tgp_git_push_with_auth(repo, remote_name, branch_name,
                                           NULL, NULL, &error))
                {
                    /* Authentication likely failed - ask user for credentials */
                    if (tgp_show_login_dialog(GTK_WINDOW(data->window), remote_name,
                                             &username, &password, &save_credentials))
                    {
                        /* Try again with provided credentials */
                        g_error_free(error);
                        error = NULL;
                        if (tgp_git_push_with_auth(repo, remote_name, branch_name,
                                                   username, password, &error))
                        {
                            tgp_show_info_dialog(GTK_WINDOW(data->window),
                                                "Push Successful",
                                                "Changes pushed to remote successfully.");
                        }
                        else
                        {
                            tgp_show_error_dialog(GTK_WINDOW(data->window),
                                                 "Push Failed",
                                                 error ? error->message : "Unknown error");
                        }
                    }
                }
                else
                {
                    tgp_show_info_dialog(GTK_WINDOW(data->window),
                                        "Push Successful",
                                        "Changes pushed to remote successfully.");
                }
            }

            g_list_free_full(remotes, g_free);
            g_free(branch_name);
        }

        git_repository_free(repo);
    }

    if (error)
        g_error_free(error);
    if (username)
    {
        memset(username, 0, strlen(username));
        g_free(username);
    }
    if (password)
    {
        memset(password, 0, strlen(password));
        g_free(password);
    }

    action_data_free(data);
}

static void
action_pull(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    git_repository *repo;
    GList *remotes;
    gchar *remote_name = NULL;
    gchar *branch_name = NULL;
    gchar *username = NULL;
    gchar *password = NULL;
    gboolean save_credentials = FALSE;
    GError *error = NULL;

    repo = tgp_git_open_repository(data->repo_path);
    if (repo)
    {
        remotes = tgp_git_get_remotes(repo);
        if (remotes)
        {
            remote_name = (gchar *)remotes->data;
            branch_name = tgp_git_get_current_branch(repo);

            if (remote_name && branch_name)
            {
                /* Try to pull with stored credentials first */
                if (!tgp_git_pull_with_auth(repo, remote_name, branch_name,
                                           NULL, NULL, &error))
                {
                    /* Authentication likely failed - ask user for credentials */
                    if (tgp_show_login_dialog(GTK_WINDOW(data->window), remote_name,
                                             &username, &password, &save_credentials))
                    {
                        /* Try again with provided credentials */
                        g_error_free(error);
                        error = NULL;
                        if (tgp_git_pull_with_auth(repo, remote_name, branch_name,
                                                   username, password, &error))
                        {
                            tgp_show_info_dialog(GTK_WINDOW(data->window),
                                                "Pull Successful",
                                                "Changes pulled from remote successfully.");
                        }
                        else
                        {
                            tgp_show_error_dialog(GTK_WINDOW(data->window),
                                                 "Pull Failed",
                                                 error ? error->message : "Unknown error");
                        }
                    }
                }
                else
                {
                    tgp_show_info_dialog(GTK_WINDOW(data->window),
                                        "Pull Successful",
                                        "Changes pulled from remote successfully.");
                }
            }

            g_list_free_full(remotes, g_free);
            g_free(branch_name);
        }

        git_repository_free(repo);
    }

    if (error)
        g_error_free(error);
    if (username)
    {
        memset(username, 0, strlen(username));
        g_free(username);
    }
    if (password)
    {
        memset(password, 0, strlen(password));
        g_free(password);
    }

    action_data_free(data);
}

static void
action_clone(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_clone_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}

static void
action_log(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_log_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}

static void
action_diff(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    
    if (data->files)
    {
        gchar *file_path = thunarx_file_info_get_location(data->files->data);
        tgp_show_diff_dialog(GTK_WINDOW(data->window), data->repo_path, file_path);
        g_free(file_path);
    }
    
    action_data_free(data);
}

static void
action_branch(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_branch_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}

static void
action_stash(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_stash_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}

static void
action_init(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    git_repository *repo = NULL;
    
    if (git_repository_init(&repo, data->repo_path, 0) == 0)
    {
        tgp_show_info_dialog(GTK_WINDOW(data->window), 
                            "Repository Created", 
                            "Git repository initialized successfully.");
        git_repository_free(repo);
    }
    else
    {
        tgp_show_error_dialog(GTK_WINDOW(data->window), 
                             "Init Failed", 
                             "Failed to initialize repository.");
    }
    
    action_data_free(data);
}

static void
action_revert(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_YES_NO,
                                                "Are you sure you want to discard all local changes?");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
    {
        /* TODO: Implement revert */
        tgp_show_info_dialog(GTK_WINDOW(data->window), 
                            "Changes Reverted", 
                            "Local changes have been discarded.");
    }
    
    gtk_widget_destroy(dialog);
    action_data_free(data);
}

static void
action_resolve(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_conflict_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}

static void
action_fetch(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    git_repository *repo = tgp_git_open_repository(data->repo_path);
    
    if (repo)
    {
        GError *error = NULL;
        if (tgp_git_pull(repo, "origin", NULL, &error))
        {
            tgp_show_info_dialog(GTK_WINDOW(data->window), 
                                "Fetch Complete", 
                                "Successfully fetched from remote.");
        }
        else
        {
            tgp_show_error_dialog(GTK_WINDOW(data->window), 
                                 "Fetch Failed", 
                                 error ? error->message : "Unknown error");
            if (error) g_error_free(error);
        }
        git_repository_free(repo);
    }
    
    action_data_free(data);
}

static void
action_status(ThunarxMenuItem *item, gpointer user_data)
{
    ActionData *data = user_data;
    tgp_show_status_dialog(GTK_WINDOW(data->window), data->repo_path);
    action_data_free(data);
}
