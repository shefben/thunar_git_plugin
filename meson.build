# Thunar Git Plugin - Meson Build Configuration
# Copyright (C) 2025 MiniMax Agent

project('thunar-git-plugin', 'c',
    version: '1.0.0',
    license: 'GPL-2.0+',
    meson_version: '>= 0.50.0',
    default_options: [
        'c_std=gnu11',
        'warning_level=2',
        'buildtype=release'
    ]
)

# Dependencies
glib_dep = dependency('glib-2.0', version: '>= 2.50.0')
gtk_dep = dependency('gtk+-3.0', version: '>= 3.22.0')
libgit2_dep = dependency('libgit2', version: '>= 1.0.0')
thunarx_dep = dependency('thunarx-3', version: '>= 1.8.0')

# Compiler flags
add_project_arguments(
    '-DPACKAGE_NAME="thunar-git-plugin"',
    '-DPACKAGE_VERSION="@0@"'.format(meson.project_version()),
    '-DGETTEXT_PACKAGE="thunar-git-plugin"',
    '-DG_LOG_DOMAIN="thunar-git-plugin"',
    language: 'c'
)

# Source files
plugin_sources = [
    'src/tgp-plugin.c',
    'src/tgp-git-utils.c',
    'src/tgp-menu-provider.c',
    'src/tgp-emblem-provider.c',
    'src/tgp-dialogs.c'
]

# Plugin library
thunar_git_plugin = shared_module('thunar-git-plugin',
    sources: plugin_sources,
    dependencies: [
        glib_dep,
        gtk_dep,
        libgit2_dep,
        thunarx_dep
    ],
    install: true,
    install_dir: join_paths(get_option('libdir'), 'thunarx-3')
)

# Install emblems
emblem_dir = join_paths(get_option('datadir'), 'icons', 'hicolor', '48x48', 'emblems')
install_data(
    [
        'icons/emblem-git-clean.svg',
        'icons/emblem-git-modified.svg',
        'icons/emblem-git-added.svg',
        'icons/emblem-git-deleted.svg',
        'icons/emblem-git-conflict.svg',
        'icons/emblem-git-untracked.svg',
        'icons/emblem-git-ignored.svg',
        'icons/emblem-git-ahead.svg',
        'icons/emblem-git-behind.svg'
    ],
    install_dir: emblem_dir
)

# Install desktop file for configuration (optional)
desktop_file_dir = join_paths(get_option('datadir'), 'applications')

# Summary
summary({
    'prefix': get_option('prefix'),
    'libdir': get_option('libdir'),
    'datadir': get_option('datadir'),
}, section: 'Directories')

summary({
    'glib-2.0': glib_dep.version(),
    'gtk+-3.0': gtk_dep.version(),
    'libgit2': libgit2_dep.version(),
    'thunarx-3': thunarx_dep.version(),
}, section: 'Dependencies')
