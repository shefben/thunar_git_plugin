# Thunar Git Plugin

A native Git integration plugin for the Thunar file manager (XFCE4), providing TortoiseGit-like features for Linux.

## Features

### Visual Emblems
Display Git status indicators directly on files and folders:
- **Green checkmark** - Clean/synced files
- **Yellow exclamation** - Modified files
- **Blue plus** - Added files
- **Red minus** - Deleted files
- **Red X** - Conflicted files
- **Purple question mark** - Untracked files
- **Gray circle** - Ignored files
- **Green up arrow** - Ahead of remote
- **Orange down arrow** - Behind remote

### Context Menu Operations

#### File Operations
- **Add** - Stage files for commit
- **Commit** - Commit changes with message
- **Revert** - Discard local changes
- **Show Diff** - View file differences
- **Show Log** - View commit history

#### Synchronization
- **Push** - Push commits to remote
- **Pull** - Pull changes from remote
- **Fetch** - Fetch from remote without merging

#### Branch Management
- **Branch Manager** - View, create, delete, and checkout branches
- **Current branch display** - See which branch you're on

#### Advanced Features
- **Stash Changes** - Temporarily save uncommitted changes
- **Resolve Conflicts** - View and manage merge conflicts
- **Repository Status** - Complete repository status overview
- **Clone Repository** - Clone existing repositories
- **Create Repository** - Initialize new Git repositories

## Requirements

### Dependencies
- **GLib** >= 2.50.0
- **GTK+** >= 3.22.0
- **libgit2** >= 1.0.0
- **Thunarx** >= 3.0 (Thunar >= 1.8.0)
- **Meson** >= 0.50.0 (for building)

### Gentoo-specific packages
```bash
emerge --ask dev-libs/glib x11-libs/gtk+:3 dev-libs/libgit2 xfce-base/thunar dev-util/meson
```

## Installation

### Quick Install (Meson)

```bash
# Clone or download the source
cd thunar-git-plugin

# Configure with meson
meson setup build --prefix=/usr

# Build
meson compile -C build

# Install (as root)
sudo meson install -C build

# Update icon cache
sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
```

### Gentoo Installation

1. **Copy ebuild to local overlay:**
```bash
# Create local overlay if it doesn't exist
sudo mkdir -p /var/db/repos/local/xfce-extra/thunar-git-plugin

# Copy ebuild
sudo cp thunar-git-plugin-1.0.0.ebuild /var/db/repos/local/xfce-extra/thunar-git-plugin/

# Create manifest
cd /var/db/repos/local/xfce-extra/thunar-git-plugin
sudo ebuild thunar-git-plugin-1.0.0.ebuild manifest
```

2. **Install via portage:**
```bash
sudo emerge --ask xfce-extra/thunar-git-plugin
```

### Manual Installation

1. **Build the plugin:**
```bash
meson setup build
meson compile -C build
```

2. **Copy plugin library:**
```bash
sudo cp build/libthunar-git-plugin.so /usr/lib64/thunarx-3/
```

3. **Install emblems:**
```bash
sudo mkdir -p /usr/share/icons/hicolor/48x48/emblems/
sudo cp icons/*.svg /usr/share/icons/hicolor/48x48/emblems/
sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
```

4. **Restart Thunar:**
```bash
thunar -q && thunar
```

## Usage

### Basic Workflow

1. **In a Git repository:**
   - Right-click on any file or folder
   - Select "Git" from the context menu
   - Choose your desired action

2. **Outside a Git repository:**
   - Right-click in any folder
   - Select "Git" → "Clone Repository" or "Create Repository Here"

### Committing Changes

1. Right-click on modified files
2. Select "Git" → "Commit..."
3. Enter your commit message
4. Select files to include
5. Click "Commit"

### Pushing Changes

1. Right-click anywhere in the repository
2. Select "Git" → "Push"
3. Select remote and branch
4. Click "Push"

### Managing Branches

1. Right-click anywhere in the repository
2. Select "Git" → "Branch Manager..."
3. View current branches
4. Create, checkout, or delete branches

## Troubleshooting

### Plugin not loading

1. Check if the plugin is in the correct location:
```bash
ls -la /usr/lib64/thunarx-3/ | grep git
```

2. Check Thunar plugin settings:
   - Edit → Configure Toolbars → Plugins tab

3. Check for errors in logs:
```bash
thunar --debug 2>&1 | grep -i git
```

### Emblems not showing

1. Verify emblems are installed:
```bash
ls -la /usr/share/icons/hicolor/48x48/emblems/ | grep git
```

2. Update icon cache:
```bash
sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
```

3. Check your icon theme supports emblems

### Authentication issues for push/pull

The plugin uses libgit2's default credential handling. You may need to:
- Set up SSH keys properly
- Configure Git credential helper
- Use HTTPS with credential caching

```bash
# Set up credential caching
git config --global credential.helper cache
```

## Development

### Building for Development

```bash
meson setup build --prefix=$HOME/.local -Dbuildtype=debug
meson compile -C build
meson install -C build
```

### Testing

1. Install to local directory:
```bash
mkdir -p ~/.local/lib/thunarx-3
cp build/libthunar-git-plugin.so ~/.local/lib/thunarx-3/
```

2. Set environment variable:
```bash
export THUNARX_EXTENSION_PATH=$HOME/.local/lib/thunarx-3:$THUNARX_EXTENSION_PATH
thunar
```

### Debug Mode

Enable debug output:
```bash
G_MESSAGES_DEBUG=thunar-git-plugin thunar
```

## Architecture

```
thunar-git-plugin/
├── src/
│   ├── tgp-plugin.c/.h       # Main plugin entry point
│   ├── tgp-git-utils.c/.h    # Git operations via libgit2
│   ├── tgp-menu-provider.c/.h # Context menu provider
│   ├── tgp-emblem-provider.c/.h # Status emblems
│   └── tgp-dialogs.c/.h      # GTK3 dialogs
├── icons/                     # SVG emblem icons
├── data/                      # Desktop/metadata files
├── meson.build                # Build configuration
└── *.ebuild                   # Gentoo package
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the GNU General Public License v2.0 or later.

## Credits

- **Author:** MiniMax Agent
- **Inspired by:** TortoiseGit, RabbitVCS
- **Built with:** libgit2, GTK3, Thunarx API

## Changelog

### Version 1.0.0 (2025-11-17)
- Initial release
- Full Git integration via libgit2
- Context menu with comprehensive Git operations
- GTK3 dialogs for all major operations
- Emblem system for visual status indication
- Branch management
- Stash support
- Conflict resolution interface
- Complete repository status overview
