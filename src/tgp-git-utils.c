/*
 * Thunar Git Plugin - Git Utilities Implementation
 * Copyright (C) 2025 MiniMax Agent
 */

#include "tgp-git-utils.h"
#include "tgp-credentials.h"
#include <string.h>
#include <stdio.h>

void
tgp_git_init(void)
{
    git_libgit2_init();
}

void
tgp_git_shutdown(void)
{
    git_libgit2_shutdown();
}

git_repository*
tgp_git_open_repository(const gchar *path)
{
    git_repository *repo = NULL;
    git_buf repo_path = {0};
    
    if (git_repository_discover(&repo_path, path, 0, NULL) == 0)
    {
        git_repository_open(&repo, repo_path.ptr);
        git_buf_dispose(&repo_path);
    }
    
    return repo;
}

gboolean
tgp_git_is_repository(const gchar *path)
{
    git_repository *repo = tgp_git_open_repository(path);
    if (repo)
    {
        git_repository_free(repo);
        return TRUE;
    }
    return FALSE;
}

gchar*
tgp_git_find_repository_root(const gchar *path)
{
    git_buf repo_path = {0};
    gchar *result = NULL;
    
    if (git_repository_discover(&repo_path, path, 0, NULL) == 0)
    {
        result = g_strdup(repo_path.ptr);
        git_buf_dispose(&repo_path);
    }
    
    return result;
}

TgpStatusFlags
tgp_git_get_file_status(git_repository *repo, const gchar *path)
{
    TgpStatusFlags flags = 0;
    unsigned int status_flags;
    const gchar *workdir;
    gchar *relative_path;
    
    if (!repo || !path)
        return flags;
    
    workdir = git_repository_workdir(repo);
    if (!workdir)
        return flags;
    
    /* Get relative path from repository root */
    if (g_str_has_prefix(path, workdir))
    {
        relative_path = g_strdup(path + strlen(workdir));
    }
    else
    {
        relative_path = g_strdup(path);
    }
    
    if (git_status_file(&status_flags, repo, relative_path) == 0)
    {
        if (status_flags & GIT_STATUS_INDEX_NEW || status_flags & GIT_STATUS_WT_NEW)
            flags |= TGP_STATUS_UNTRACKED;
        
        if (status_flags & GIT_STATUS_INDEX_MODIFIED || status_flags & GIT_STATUS_WT_MODIFIED)
            flags |= TGP_STATUS_MODIFIED;
        
        if (status_flags & GIT_STATUS_INDEX_DELETED || status_flags & GIT_STATUS_WT_DELETED)
            flags |= TGP_STATUS_DELETED;
        
        if (status_flags & GIT_STATUS_INDEX_RENAMED || status_flags & GIT_STATUS_WT_RENAMED)
            flags |= TGP_STATUS_RENAMED;
        
        if (status_flags & GIT_STATUS_CONFLICTED)
            flags |= TGP_STATUS_CONFLICTED;
        
        if (status_flags & GIT_STATUS_IGNORED)
            flags |= TGP_STATUS_IGNORED;
        
        if (status_flags == GIT_STATUS_CURRENT)
            flags |= TGP_STATUS_CLEAN;
    }
    
    g_free(relative_path);
    return flags;
}

gboolean
tgp_git_has_uncommitted_changes(git_repository *repo)
{
    git_status_list *status_list;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    gboolean has_changes = FALSE;
    
    opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;
    
    if (git_status_list_new(&status_list, repo, &opts) == 0)
    {
        size_t count = git_status_list_entrycount(status_list);
        has_changes = (count > 0);
        git_status_list_free(status_list);
    }
    
    return has_changes;
}

gboolean
tgp_git_is_ahead_behind(git_repository *repo, gint *ahead, gint *behind)
{
    git_reference *head = NULL;
    git_reference *upstream = NULL;
    git_oid local_oid, upstream_oid;
    size_t ahead_count = 0, behind_count = 0;
    gboolean result = FALSE;
    
    if (git_repository_head(&head, repo) != 0)
        goto cleanup;
    
    if (git_branch_upstream(&upstream, head) != 0)
        goto cleanup;
    
    git_reference_name_to_id(&local_oid, repo, git_reference_name(head));
    git_reference_name_to_id(&upstream_oid, repo, git_reference_name(upstream));
    
    if (git_graph_ahead_behind(&ahead_count, &behind_count, repo, &local_oid, &upstream_oid) == 0)
    {
        if (ahead) *ahead = ahead_count;
        if (behind) *behind = behind_count;
        result = TRUE;
    }
    
cleanup:
    if (head) git_reference_free(head);
    if (upstream) git_reference_free(upstream);
    
    return result;
}

gchar*
tgp_git_get_current_branch(git_repository *repo)
{
    git_reference *head = NULL;
    const gchar *branch_name = NULL;
    gchar *result = NULL;
    
    if (git_repository_head(&head, repo) == 0)
    {
        if (git_branch_name(&branch_name, head) == 0)
        {
            result = g_strdup(branch_name);
        }
        git_reference_free(head);
    }
    
    return result;
}

gboolean
tgp_git_add_files(git_repository *repo, GList *files, GError **error)
{
    git_index *index;
    const gchar *workdir;
    gboolean success = TRUE;
    
    if (git_repository_index(&index, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to open repository index");
        return FALSE;
    }
    
    workdir = git_repository_workdir(repo);
    
    for (GList *l = files; l != NULL; l = l->next)
    {
        const gchar *file = l->data;
        gchar *relative_path;
        
        if (g_str_has_prefix(file, workdir))
            relative_path = g_strdup(file + strlen(workdir));
        else
            relative_path = g_strdup(file);
        
        if (git_index_add_bypath(index, relative_path) != 0)
        {
            g_set_error(error, 0, 0, "Failed to add file: %s", relative_path);
            success = FALSE;
            g_free(relative_path);
            break;
        }
        
        g_free(relative_path);
    }
    
    if (success)
    {
        git_index_write(index);
    }
    
    git_index_free(index);
    return success;
}

gboolean
tgp_git_commit(git_repository *repo, const gchar *message, GList *files, GError **error)
{
    git_signature *sig = NULL;
    git_index *index = NULL;
    git_oid tree_id, commit_id;
    git_tree *tree = NULL;
    git_reference *head = NULL;
    git_commit *parent = NULL;
    gboolean success = FALSE;
    
    /* Add files first */
    if (files && !tgp_git_add_files(repo, files, error))
        return FALSE;
    
    /* Get default signature */
    if (git_signature_default(&sig, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to create signature");
        goto cleanup;
    }
    
    /* Get the index and create tree */
    if (git_repository_index(&index, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to open index");
        goto cleanup;
    }
    
    if (git_index_write_tree(&tree_id, index) != 0)
    {
        g_set_error(error, 0, 0, "Failed to write tree");
        goto cleanup;
    }
    
    if (git_tree_lookup(&tree, repo, &tree_id) != 0)
    {
        g_set_error(error, 0, 0, "Failed to lookup tree");
        goto cleanup;
    }
    
    /* Get parent commit */
    if (git_repository_head(&head, repo) == 0)
    {
        git_commit_lookup(&parent, repo, git_reference_target(head));
    }
    
    /* Create commit */
    if (git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,
                            NULL, message, tree, parent ? 1 : 0, parent) == 0)
    {
        success = TRUE;
    }
    else
    {
        g_set_error(error, 0, 0, "Failed to create commit");
    }
    
cleanup:
    if (sig) git_signature_free(sig);
    if (index) git_index_free(index);
    if (tree) git_tree_free(tree);
    if (head) git_reference_free(head);
    if (parent) git_commit_free(parent);
    
    return success;
}

gboolean
tgp_git_clone(const gchar *url, const gchar *path, GError **error)
{
    git_repository *repo = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    
    if (git_clone(&repo, url, path, &clone_opts) != 0)
    {
        const git_error *e = git_error_last();
        g_set_error(error, 0, 0, "Clone failed: %s", e ? e->message : "unknown error");
        return FALSE;
    }
    
    git_repository_free(repo);
    return TRUE;
}

gboolean
tgp_git_push(git_repository *repo, const gchar *remote_name, const gchar *branch, GError **error)
{
    git_remote *remote = NULL;
    git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
    gchar *refspec;
    const git_strarray refspecs = { &refspec, 1 };
    gboolean success = FALSE;
    
    if (git_remote_lookup(&remote, repo, remote_name ? remote_name : "origin") != 0)
    {
        g_set_error(error, 0, 0, "Failed to lookup remote");
        return FALSE;
    }
    
    refspec = g_strdup_printf("refs/heads/%s:refs/heads/%s", branch, branch);
    
    if (git_remote_push(remote, &refspecs, &push_opts) == 0)
    {
        success = TRUE;
    }
    else
    {
        const git_error *e = git_error_last();
        g_set_error(error, 0, 0, "Push failed: %s", e ? e->message : "unknown error");
    }
    
    g_free(refspec);
    git_remote_free(remote);
    
    return success;
}

gboolean
tgp_git_pull(git_repository *repo, const gchar *remote_name, const gchar *branch, GError **error)
{
    git_remote *remote = NULL;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    gboolean success = FALSE;
    
    if (git_remote_lookup(&remote, repo, remote_name ? remote_name : "origin") != 0)
    {
        g_set_error(error, 0, 0, "Failed to lookup remote");
        return FALSE;
    }
    
    if (git_remote_fetch(remote, NULL, &fetch_opts, NULL) == 0)
    {
        /* TODO: Implement merge after fetch */
        success = TRUE;
    }
    else
    {
        const git_error *e = git_error_last();
        g_set_error(error, 0, 0, "Pull failed: %s", e ? e->message : "unknown error");
    }
    
    git_remote_free(remote);
    return success;
}

gboolean
tgp_git_has_conflicts(git_repository *repo)
{
    git_index *index;
    gboolean has_conflicts = FALSE;
    
    if (git_repository_index(&index, repo) == 0)
    {
        has_conflicts = git_index_has_conflicts(index);
        git_index_free(index);
    }
    
    return has_conflicts;
}

gboolean
tgp_git_stash(git_repository *repo, const gchar *message, GError **error)
{
    git_signature *sig = NULL;
    git_oid stash_id;
    gboolean success = FALSE;
    
    if (git_signature_default(&sig, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to create signature");
        return FALSE;
    }
    
    if (git_stash_save(&stash_id, repo, sig, message, GIT_STASH_DEFAULT) == 0)
    {
        success = TRUE;
    }
    else
    {
        g_set_error(error, 0, 0, "Failed to create stash");
    }
    
    git_signature_free(sig);
    return success;
}

gboolean
tgp_git_stash_pop(git_repository *repo, GError **error)
{
    git_stash_apply_options opts = GIT_STASH_APPLY_OPTIONS_INIT;
    
    if (git_stash_pop(repo, 0, &opts) != 0)
    {
        const git_error *e = git_error_last();
        g_set_error(error, 0, 0, "Stash pop failed: %s", e ? e->message : "unknown error");
        return FALSE;
    }
    
    return TRUE;
}


gchar*
tgp_git_get_diff(git_repository *repo, const gchar *path)
{
    git_diff *diff = NULL;
    git_diff_options diff_opts = GIT_DIFF_OPTIONS_INIT;
    GString *diff_text = g_string_new("");
    
    /* Get diff between HEAD and working directory */
    if (git_diff_index_to_workdir(&diff, repo, NULL, &diff_opts) == 0)
    {
        size_t num_deltas = git_diff_num_deltas(diff);
        
        for (size_t i = 0; i < num_deltas; i++)
        {
            const git_diff_delta *delta = git_diff_get_delta(diff, i);
            git_patch *patch = NULL;
            
            if (path && strcmp(delta->new_file.path, path) != 0)
                continue;
            
            if (git_patch_from_diff(&patch, diff, i) == 0)
            {
                git_buf patch_buf = {0};
                if (git_patch_to_buf(&patch_buf, patch) == 0)
                {
                    g_string_append(diff_text, patch_buf.ptr);
                    git_buf_dispose(&patch_buf);
                }
                git_patch_free(patch);
            }
        }
        
        git_diff_free(diff);
    }
    
    if (diff_text->len == 0)
    {
        g_string_free(diff_text, TRUE);
        return NULL;
    }
    
    return g_string_free(diff_text, FALSE);
}

GList*
tgp_git_get_branches(git_repository *repo)
{
    GList *branches = NULL;
    git_branch_iterator *iter;
    git_reference *ref;
    git_branch_t branch_type;
    
    if (git_branch_iterator_new(&iter, repo, GIT_BRANCH_ALL) == 0)
    {
        while (git_branch_next(&ref, &branch_type, iter) == 0)
        {
            const gchar *name;
            if (git_branch_name(&name, ref) == 0)
            {
                branches = g_list_append(branches, g_strdup(name));
            }
            git_reference_free(ref);
        }
        git_branch_iterator_free(iter);
    }
    
    return branches;
}

gboolean
tgp_git_checkout_branch(git_repository *repo, const gchar *branch_name, GError **error)
{
    git_reference *branch_ref = NULL;
    git_object *treeish = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    gboolean success = FALSE;
    
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    
    if (git_branch_lookup(&branch_ref, repo, branch_name, GIT_BRANCH_LOCAL) != 0)
    {
        g_set_error(error, 0, 0, "Branch not found: %s", branch_name);
        goto cleanup;
    }
    
    if (git_reference_peel(&treeish, branch_ref, GIT_OBJ_TREE) != 0)
    {
        g_set_error(error, 0, 0, "Failed to peel reference");
        goto cleanup;
    }
    
    if (git_checkout_tree(repo, treeish, &checkout_opts) != 0)
    {
        g_set_error(error, 0, 0, "Checkout failed");
        goto cleanup;
    }
    
    if (git_repository_set_head(repo, git_reference_name(branch_ref)) != 0)
    {
        g_set_error(error, 0, 0, "Failed to set HEAD");
        goto cleanup;
    }
    
    success = TRUE;
    
cleanup:
    if (branch_ref) git_reference_free(branch_ref);
    if (treeish) git_object_free(treeish);
    
    return success;
}

gboolean
tgp_git_create_branch(git_repository *repo, const gchar *branch_name, GError **error)
{
    git_reference *head = NULL;
    git_commit *target_commit = NULL;
    git_reference *new_branch = NULL;
    gboolean success = FALSE;
    
    if (git_repository_head(&head, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to get HEAD");
        return FALSE;
    }
    
    if (git_commit_lookup(&target_commit, repo, git_reference_target(head)) != 0)
    {
        g_set_error(error, 0, 0, "Failed to lookup commit");
        git_reference_free(head);
        return FALSE;
    }
    
    if (git_branch_create(&new_branch, repo, branch_name, target_commit, 0) == 0)
    {
        success = TRUE;
        git_reference_free(new_branch);
    }
    else
    {
        g_set_error(error, 0, 0, "Failed to create branch");
    }
    
    git_commit_free(target_commit);
    git_reference_free(head);
    
    return success;
}

gboolean
tgp_git_remove_files(git_repository *repo, GList *files, GError **error)
{
    git_index *index;
    const gchar *workdir;
    gboolean success = TRUE;
    
    if (git_repository_index(&index, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to open repository index");
        return FALSE;
    }
    
    workdir = git_repository_workdir(repo);
    
    for (GList *l = files; l != NULL; l = l->next)
    {
        const gchar *file = l->data;
        gchar *relative_path;
        
        if (g_str_has_prefix(file, workdir))
            relative_path = g_strdup(file + strlen(workdir));
        else
            relative_path = g_strdup(file);
        
        if (git_index_remove_bypath(index, relative_path) != 0)
        {
            g_set_error(error, 0, 0, "Failed to remove file: %s", relative_path);
            success = FALSE;
            g_free(relative_path);
            break;
        }
        
        g_free(relative_path);
    }
    
    if (success)
    {
        git_index_write(index);
    }
    
    git_index_free(index);
    return success;
}

gboolean
tgp_git_fetch(git_repository *repo, const gchar *remote_name, GError **error)
{
    git_remote *remote = NULL;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    gboolean success = FALSE;
    
    if (git_remote_lookup(&remote, repo, remote_name ? remote_name : "origin") != 0)
    {
        g_set_error(error, 0, 0, "Failed to lookup remote");
        return FALSE;
    }
    
    if (git_remote_fetch(remote, NULL, &fetch_opts, "fetch") == 0)
    {
        success = TRUE;
    }
    else
    {
        const git_error *e = git_error_last();
        g_set_error(error, 0, 0, "Fetch failed: %s", e ? e->message : "unknown error");
    }
    
    git_remote_free(remote);
    return success;
}

GList*
tgp_git_get_remotes(git_repository *repo)
{
    GList *remotes = NULL;
    git_strarray remote_names;
    
    if (git_remote_list(&remote_names, repo) == 0)
    {
        for (size_t i = 0; i < remote_names.count; i++)
        {
            remotes = g_list_append(remotes, g_strdup(remote_names.strings[i]));
        }
        git_strarray_free(&remote_names);
    }
    
    return remotes;
}

GList*
tgp_git_get_log(git_repository *repo, gint limit)
{
    /* This would return a list of commit structures */
    /* For simplicity, we're using the dialog implementation directly */
    return NULL;
}

GList*
tgp_git_get_conflicted_files(git_repository *repo)
{
    GList *files = NULL;
    git_index *index;
    
    if (git_repository_index(&index, repo) == 0)
    {
        git_index_conflict_iterator *iter;
        const git_index_entry *ancestor, *our, *their;
        
        if (git_index_conflict_iterator_new(&iter, index) == 0)
        {
            while (git_index_conflict_next(&ancestor, &our, &their, iter) == 0)
            {
                const gchar *path = our ? our->path : (their ? their->path : NULL);
                if (path)
                    files = g_list_append(files, g_strdup(path));
            }
            git_index_conflict_iterator_free(iter);
        }
        git_index_free(index);
    }
    
    return files;
}

gboolean
tgp_git_resolve_conflict(git_repository *repo, const gchar *path, GError **error)
{
    git_index *index;
    gboolean success = FALSE;
    
    if (git_repository_index(&index, repo) != 0)
    {
        g_set_error(error, 0, 0, "Failed to open index");
        return FALSE;
    }
    
    if (git_index_conflict_remove(index, path) == 0)
    {
        git_index_write(index);
        success = TRUE;
    }
    else
    {
        g_set_error(error, 0, 0, "Failed to resolve conflict");
    }
    
    git_index_free(index);
    return success;
}

GList*
tgp_git_get_stashes(git_repository *repo)
{
    /* Simplified - would need to implement stash list traversal */
    return NULL;
}

/*
 * Push with authentication support
 * Uses provided credentials or stored credentials
 */
gboolean
tgp_git_push_with_auth(git_repository *repo, const gchar *remote, const gchar *branch,
                       const gchar *username, const gchar *password, GError **error)
{
    git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
    git_remote *remote_obj = NULL;
    gchar *refspec = NULL;
    int ret = 0;

    if (!repo || !remote || !branch)
        return FALSE;

    /* Open remote */
    if (git_remote_lookup(&remote_obj, repo, remote) != 0)
    {
        g_set_error(error, 0, 0, "Remote '%s' not found", remote);
        return FALSE;
    }

    /* Set up credentials callback */
    git_credential_acquire_cb cred_callback = tgp_git_credentials_callback;
    push_opts.callbacks.credentials = cred_callback;

    /* If credentials provided, try to store them first */
    if (username && password)
    {
        git_remote_url(remote_obj);
        /* Extract host from URL */
        const gchar *url = git_remote_url(remote_obj);
        if (url)
        {
            gchar *host = NULL;
            if (g_str_has_prefix(url, "https://"))
                host = g_strdup(url + 8);
            else if (g_str_has_prefix(url, "http://"))
                host = g_strdup(url + 7);

            if (host)
            {
                gchar *slash = strchr(host, '/');
                if (slash)
                    *slash = '\0';

                tgp_credentials_store(host, username, password, 3600);
                g_free(host);
            }
        }
    }

    /* Build refspec: local branch -> remote branch */
    refspec = g_strdup_printf("refs/heads/%s:refs/heads/%s", branch, branch);

    /* Push */
    ret = git_remote_push(remote_obj, (const char * const*)&refspec, &push_opts);

    g_free(refspec);
    git_remote_free(remote_obj);

    if (ret != 0)
    {
        g_set_error(error, 0, 0, "Push failed: %s", git_error_last()->message);
        return FALSE;
    }

    return TRUE;
}

/*
 * Pull with authentication support
 * Uses provided credentials or stored credentials
 */
gboolean
tgp_git_pull_with_auth(git_repository *repo, const gchar *remote, const gchar *branch,
                       const gchar *username, const gchar *password, GError **error)
{
    git_remote *remote_obj = NULL;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_reference *ref = NULL;
    git_annotated_commit *annotated = NULL;
    gint ret = 0;

    if (!repo || !remote || !branch)
        return FALSE;

    /* Open remote */
    if (git_remote_lookup(&remote_obj, repo, remote) != 0)
    {
        g_set_error(error, 0, 0, "Remote '%s' not found", remote);
        return FALSE;
    }

    /* Set up credentials callback */
    fetch_opts.callbacks.credentials = tgp_git_credentials_callback;

    /* If credentials provided, store them first */
    if (username && password)
    {
        const gchar *url = git_remote_url(remote_obj);
        if (url)
        {
            gchar *host = NULL;
            if (g_str_has_prefix(url, "https://"))
                host = g_strdup(url + 8);
            else if (g_str_has_prefix(url, "http://"))
                host = g_strdup(url + 7);

            if (host)
            {
                gchar *slash = strchr(host, '/');
                if (slash)
                    *slash = '\0';

                tgp_credentials_store(host, username, password, 3600);
                g_free(host);
            }
        }
    }

    /* Fetch from remote */
    ret = git_remote_fetch(remote_obj, NULL, &fetch_opts, NULL);
    if (ret != 0)
    {
        g_set_error(error, 0, 0, "Fetch failed: %s", git_error_last()->message);
        git_remote_free(remote_obj);
        return FALSE;
    }

    /* Merge remote tracking branch */
    gchar *remote_ref = g_strdup_printf("refs/remotes/%s/%s", remote, branch);

    if (git_reference_lookup(&ref, repo, remote_ref) != 0)
    {
        g_set_error(error, 0, 0, "Remote reference '%s' not found", remote_ref);
        g_free(remote_ref);
        git_remote_free(remote_obj);
        return FALSE;
    }

    if (git_annotated_commit_from_ref(&annotated, repo, ref) != 0)
    {
        g_set_error(error, 0, 0, "Failed to resolve remote reference '%s'", remote_ref);
        git_reference_free(ref);
        g_free(remote_ref);
        git_remote_free(remote_obj);
        return FALSE;
    }

    git_reference_free(ref);

    ret = git_merge(repo,
                    (const git_annotated_commit **)&annotated,
                    1,
                    &merge_opts,
                    &checkout_opts);

    git_annotated_commit_free(annotated);

    g_free(remote_ref);
    git_remote_free(remote_obj);

    if (ret != 0 && ret != GIT_EMERGECONFLICT)
    {
        g_set_error(error, 0, 0, "Merge failed: %s", git_error_last()->message);
        return FALSE;
    }

    return TRUE;
}
