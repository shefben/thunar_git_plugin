# Installation Guide for Gentoo Linux

This document provides detailed instructions for installing Thunar Git Plugin on Gentoo Linux with XFCE4.

## Prerequisites

### System Requirements
- Gentoo Linux (stable or testing)
- XFCE4 desktop environment
- Thunar file manager >= 1.8.0
- Git (optional, for development)

### Install Dependencies

```bash
# Install base dependencies
sudo emerge --ask \
    dev-libs/glib \
    x11-libs/gtk+:3 \
    dev-libs/libgit2 \
    xfce-base/thunar \
    xfce-base/exo

# Install build tools
sudo emerge --ask \
    dev-util/meson \
    dev-util/pkgconfig \
    sys-devel/gcc
```

## Installation Methods

### Method 1: Using Portage (Recommended)

1. **Set up local overlay (if not already done):**
```bash
# Create directories
sudo mkdir -p /var/db/repos/local/{metadata,profiles}

# Create layout.conf
sudo tee /var/db/repos/local/metadata/layout.conf << 'EOF'
masters = gentoo
auto-sync = false
EOF

# Create repo_name
echo "local" | sudo tee /var/db/repos/local/profiles/repo_name

# Add to repos.conf
sudo mkdir -p /etc/portage/repos.conf
sudo tee /etc/portage/repos.conf/local.conf << 'EOF'
[local]
location = /var/db/repos/local
auto-sync = false
EOF
```

2. **Add package to overlay:**
```bash
# Create package directory
sudo mkdir -p /var/db/repos/local/xfce-extra/thunar-git-plugin

# Copy source to temporary location
sudo cp -r thunar-git-plugin /tmp/thunar-git-plugin-1.0.0

# Create source tarball
cd /tmp
tar czf thunar-git-plugin-1.0.0.tar.gz thunar-git-plugin-1.0.0/

# Move to distfiles
sudo mkdir -p /var/cache/distfiles
sudo mv thunar-git-plugin-1.0.0.tar.gz /var/cache/distfiles/

# Copy ebuild
cd -
sudo cp thunar-git-plugin-1.0.0.ebuild /var/db/repos/local/xfce-extra/thunar-git-plugin/

# Generate manifest
cd /var/db/repos/local/xfce-extra/thunar-git-plugin
sudo ebuild thunar-git-plugin-1.0.0.ebuild manifest
```

3. **Install via emerge:**
```bash
sudo emerge --ask xfce-extra/thunar-git-plugin
```

### Method 2: Direct Meson Build

1. **Navigate to source directory:**
```bash
cd thunar-git-plugin
```

2. **Configure with meson:**
```bash
meson setup build \
    --prefix=/usr \
    --libdir=/usr/lib64 \
    --buildtype=release
```

3. **Build:**
```bash
meson compile -C build
```

4. **Install:**
```bash
sudo meson install -C build
```

5. **Update icon cache:**
```bash
sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
```

### Method 3: Local User Installation

For testing without system-wide installation:

```bash
# Configure for local install
meson setup build \
    --prefix=$HOME/.local \
    --libdir=$HOME/.local/lib

# Build and install
meson compile -C build
meson install -C build

# Set Thunarx extension path
echo 'export THUNARX_EXTENSION_PATH=$HOME/.local/lib/thunarx-3:$THUNARX_EXTENSION_PATH' >> ~/.bashrc
source ~/.bashrc

# Update local icon cache
mkdir -p ~/.local/share/icons/hicolor
gtk-update-icon-cache -f -t ~/.local/share/icons/hicolor
```

## Post-Installation

### Verify Installation

1. **Check plugin library:**
```bash
# For system install
ls -la /usr/lib64/thunarx-3/libthunar-git-plugin.so

# For local install
ls -la ~/.local/lib/thunarx-3/libthunar-git-plugin.so
```

2. **Check emblems:**
```bash
ls -la /usr/share/icons/hicolor/48x48/emblems/emblem-git-*.svg
```

### Activate Plugin

1. **Restart Thunar:**
```bash
thunar -q
thunar &
```

Or log out and log back in to your XFCE session.

2. **Test the plugin:**
   - Open Thunar
   - Navigate to a Git repository
   - Right-click on a file
   - You should see a "Git" submenu

### Configure Icon Theme (Optional)

If emblems don't show up:

1. **Install additional icon themes:**
```bash
sudo emerge --ask \
    x11-themes/gnome-icon-theme \
    x11-themes/hicolor-icon-theme
```

2. **Set icon theme in XFCE:**
   - Settings Manager → Appearance → Icons
   - Select a theme that supports emblems (e.g., Adwaita, Tango)

## Troubleshooting

### Plugin not loaded

1. **Check Thunar version:**
```bash
thunar --version
```
Must be >= 1.8.0

2. **Check for conflicting plugins:**
```bash
ls -la /usr/lib64/thunarx-3/
```

3. **Enable debug mode:**
```bash
G_MESSAGES_DEBUG=all thunar 2>&1 | tee thunar-debug.log
```

Look for git plugin messages in the log.

### Build failures

1. **Missing dependencies:**
```bash
# Check installed versions
pkg-config --modversion glib-2.0
pkg-config --modversion gtk+-3.0
pkg-config --modversion libgit2
pkg-config --modversion thunarx-3
```

2. **Update dependencies:**
```bash
sudo emerge --update --deep --newuse @world
```

3. **Clean build directory:**
```bash
rm -rf build
meson setup build --wipe
```

### Libgit2 version issues

If libgit2 is too old:

```bash
# Unmask newer version if needed
sudo echo ">=dev-libs/libgit2-1.0.0" >> /etc/portage/package.accept_keywords
sudo emerge --ask ">=dev-libs/libgit2-1.0.0"
```

## Uninstallation

### Method 1: Via Portage
```bash
sudo emerge --unmerge xfce-extra/thunar-git-plugin
```

### Method 2: Manual
```bash
# Remove plugin library
sudo rm /usr/lib64/thunarx-3/libthunar-git-plugin.so

# Remove emblems
sudo rm /usr/share/icons/hicolor/48x48/emblems/emblem-git-*.svg

# Update icon cache
sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor

# Restart Thunar
thunar -q
```

### Method 3: Meson uninstall
```bash
cd thunar-git-plugin/build
sudo ninja uninstall
```

## Development Setup

For plugin development on Gentoo:

1. **Install development tools:**
```bash
sudo emerge --ask \
    dev-util/gdb \
    dev-util/valgrind \
    dev-util/strace \
    app-editors/vim  # or your preferred editor
```

2. **Set up debug build:**
```bash
meson setup build-debug \
    --prefix=$HOME/.local \
    --buildtype=debug \
    -Db_sanitize=address
    
meson compile -C build-debug
meson install -C build-debug
```

3. **Run with debugger:**
```bash
G_MESSAGES_DEBUG=all gdb thunar
```

4. **Memory leak detection:**
```bash
G_SLICE=always-malloc \
G_DEBUG=gc-friendly \
valgrind --leak-check=full \
    --show-leak-kinds=all \
    thunar
```

## Performance Optimization

### For faster builds:
```bash
# Use ninja with multiple jobs
meson setup build --backend=ninja
ninja -C build -j$(nproc)
```

### For smaller binary size:
```bash
meson setup build \
    --buildtype=release \
    --strip \
    -Db_lto=true
```

## Integration with Gentoo System

### Update after system upgrade:
```bash
# After major system updates
sudo emerge @preserved-rebuild

# Rebuild plugin if thunar was updated
sudo emerge --oneshot xfce-extra/thunar-git-plugin
```

### Clean old builds:
```bash
# Clean portage cache
sudo eclean-dist --deep

# Remove old versions
sudo emerge --depclean
```

## Support

If you encounter issues:

1. Check `/var/log/Xorg.0.log` for X11 errors
2. Check `~/.xsession-errors` for session errors
3. Enable verbose logging: `G_MESSAGES_DEBUG=all`
4. Check Gentoo forums: https://forums.gentoo.org/
5. File bug reports with full logs

## Additional Resources

- Gentoo Wiki: https://wiki.gentoo.org/
- Thunar Documentation: https://docs.xfce.org/xfce/thunar/
- libgit2 Documentation: https://libgit2.org/docs/
- Meson Build System: https://mesonbuild.com/
